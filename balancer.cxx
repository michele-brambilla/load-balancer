#include "balancer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>

template <typename T> // declaration only for TD;
class TD;             // TD == "Type Displayer"

struct Bytes {
public:
  int bytes{0};
};

class Status_stub {
public:
  void assign_weight(const int &i) { weight.bytes = i; }
  const Bytes &get() { return weight; }

private:
  Bytes weight;
};

class ObjectType {
public:
  void assign_weight(const int &i) { status_.assign_weight(i); }
  Status_stub &status() { return status_; }

private:
  Status_stub status_;
};

using ListType = std::map<std::string, ObjectType>;
const size_t num_partitions{5};

void generate_object_parabola(ListType &obj, const int n_elem) {
  for (int i = 0; i < n_elem; ++i) {
    obj[std::to_string(i)].assign_weight(i * i - 8. * i + 113.);
  }
}

void generate_object_harmonic(ListType &obj, const int n_elem) {
  for (int i = 0; i < n_elem; ++i) {
    obj[std::to_string(i)].assign_weight(std::abs(10000 / (i * i - 10.5)));
  }
}

void generate_object_random(ListType &obj, const int n_elem) {
  std::default_random_engine generator;
  std::poisson_distribution<int> distribution(8);
  for (int i = 0; i < n_elem; ++i) {
    obj[std::to_string(i)].assign_weight(distribution(generator));
  }
}

TEST(Partitioner, create_initial_array_one_element_per_partition) {
  ListType object;
  for (size_t i = 0; i < num_partitions; ++i) {
    object[std::to_string(i + 1)] = ObjectType();
  }

  auto object_partition = create_flat_partition(object, num_partitions);

  EXPECT_EQ(num_partitions, object_partition.size());
  for (auto &o : object_partition) {
    EXPECT_EQ(o.list.size(), size_t{1});
  }
}

TEST(Partitioner, create_initial_array_multiple_elements_per_partition) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }

  auto object_partition = create_flat_partition(object_list, num_partitions);

  EXPECT_EQ(num_partitions, object_partition.size());
  size_t nelem{0};
  for (auto &o : object_partition) {
    nelem += o.list.size();
  }
  EXPECT_EQ(nelem, n_objects);
}

TEST(Partitioner, sort_list_by_bytes) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }
  generate_object_parabola(object_list, n_objects);

  ObjectList<ListType> sorted_list;
  assign_to_ordered_list(object_list, sorted_list);

  for (size_t i = 1; i < n_objects; ++i) {
    EXPECT_TRUE(sorted_list.list[i]->second.status().get().bytes <=
                sorted_list.list[i - 1]->second.status().get().bytes);
  }
}

TEST(Partitioner, all_elements_assigned) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }
  generate_object_parabola(object_list, n_objects);

  auto sorted_list = create_balanced_partition(object_list);

  for (auto &s : sorted_list) {
    for (auto &e : s.list) {
      object_list.erase(e->first);
    }
  }
  EXPECT_EQ(object_list.size(), size_t{0});
}

TEST(Partitioner, regenerate_after_new_bytes) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }
  generate_object_parabola(object_list, n_objects);

  auto sorted_list = create_balanced_partition(object_list, num_partitions);

  for (size_t i = 0; i < n_objects; ++i) {
    auto w = object_list[std::to_string(i + 1)].status().get().bytes;
    object_list[std::to_string(i + 1)].assign_weight(w + 100 * (n_objects - i));
  }

  auto new_sorted_list = create_balanced_partition(object_list, num_partitions);

  // either the partitions length or the elements must be different
  bool is_same = true;
  for (size_t i = 0; i < num_partitions; ++i) {
    is_same &= (sorted_list[i].list.size() == new_sorted_list[i].list.size());
  }
  if (!is_same) {
    EXPECT_TRUE(is_same);
  } else {
    for (size_t i = 0; i < num_partitions; ++i) {
      is_same &= (sorted_list[i].list == new_sorted_list[i].list);
    }
  }
  EXPECT_FALSE(is_same);
}

std::mutex mutex;
std::atomic<int> has_changed{0};
void test_consume(ObjectList<ListType> &o) {
  auto old = o;
  for (int i = 0; i < 2; ++i) {
    {
      std::lock_guard<std::mutex> lk(mutex);
      if (o != old) {
        has_changed++;
      }
      // std::cout << std::this_thread::get_id() << "\t"
      //           << o.list.size() << "\n";
      // for(auto& x : o) {
      //   std::cout << x->first
      //             << "(" << x->second.status().get().bytes << ")"
      //             << "\t";
      // }
      // std::cout << "\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{1000});
  }
}

TEST(Partitioner, run_threads_no_update_partition) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }
  generate_object_parabola(object_list, n_objects);
  auto sorted_list = create_balanced_partition(object_list, num_partitions);

  // start the execution
  has_changed = 0;
  std::vector<std::thread> executers;
  for (auto &s : sorted_list) {
    executers.push_back(std::thread([&] { test_consume(s); }));
  }
  // when all thread started update one element but don't do anything else
  std::this_thread::sleep_for(std::chrono::milliseconds{500});
  object_list["3"].assign_weight(0);
  // wait for all threads completed
  for (auto &e : executers) {
    e.join();
  }
  EXPECT_EQ(has_changed, 0);
}

TEST(Partitioner, run_threads_and_update_partition) {
  size_t n_objects = 2 * num_partitions + 3;
  ListType object_list;
  for (size_t i = 0; i < n_objects; ++i) {
    object_list[std::to_string(i + 1)] = ObjectType();
  }
  generate_object_parabola(object_list, n_objects);
  auto sorted_list = create_balanced_partition(object_list, num_partitions);

  has_changed = 0;
  std::vector<std::thread> executers;
  for (auto &s : sorted_list) {
    executers.push_back(std::thread([&] { test_consume(s); }));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds{500});
  object_list["3"].assign_weight(0);
  // create new partition and assign
  //  auto new_list = create_balanced_partition(object_list, num_partitions);
  sorted_list = create_balanced_partition(object_list, num_partitions);

  for (auto &e : executers) {
    e.join();
  }
  EXPECT_NE(has_changed, 0);
}

