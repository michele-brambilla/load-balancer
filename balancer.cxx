#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

using Object = std::map<std::string, int>;

class ObjectList {
public:
  using iterator = std::vector<Object::iterator>::iterator;
  using list_type = std::vector<Object::iterator>;

  void clear() {
    list.clear();
    weight = 0;
  }
  void assign(Object::iterator &item) {
    list.push_back(item);
    weight += item->second;
  }
  bool operator<(const ObjectList &other) const {
    return weight < other.weight;
  }
  bool operator>(const ObjectList &other) const {
    return weight > other.weight;
  }

  std::vector<Object::iterator> list;
  int weight{0};
};

void generate_object(Object &obj, const int n_elem) {
  for (int i = 0; i < n_elem; ++i) {
    //    obj[std::to_string(i)] = i*i - 5*i + 7;
    obj[std::to_string(i)] = std::abs(10000 / (i * i - 10.5));
  }
}

void generate_object_random(Object &obj, const int n_elem) {
  std::default_random_engine generator;
  std::poisson_distribution<int> distribution(8);
  for (int i = 0; i < n_elem; ++i) {
    obj[std::to_string(i)] = distribution(generator);
  }
}

void assign_to_ordered_list(Object &obj, ObjectList &sorted) {
  for (auto o = obj.begin(); o != obj.end(); ++o) {
    sorted.list.push_back(o);
  }
  std::sort(sorted.list.begin(), sorted.list.end(),
            [](Object::iterator &first, Object::iterator &second) {
              return first->second > second->second;
            });
}

void partition(ObjectList::list_type &sorted, std::vector<ObjectList> &list) {

  for (auto o : sorted) {
    auto min = std::min_element(
        list.begin(), list.end(),
        [](ObjectList &first, ObjectList &second) { return first < second; });
    min->assign(o);
  }
}

void print(const std::vector<ObjectList> &list) {
  for (auto &l : list) {
    std::cout << l.weight << "  ->  ";
    //<< " (" << l.list.size() << ")" << std::endl;
    std::cout << "( ";
    for (auto &o : l.list) {
      std::cout << o->first << ",";
    }
    std::cout << ")\n";
  }
}

void sort_objects(Object &obj, std::vector<ObjectList> &list,
                  const int &n_partitions = 2) {

  ObjectList sorted;
  assign_to_ordered_list(obj, sorted);

  if (!list.empty()) { // make reusable
    list.clear();
  }
  list.resize(n_partitions);
  partition(sorted.list, list);
}

int main(int argc, char **argv) {
  Object obj;
  int n_partitions = 2;
  int n_entries = 20;
  if (argc > 1) {
    n_partitions = std::atoi(argv[1]);
  }
  if (argc > 2) {
    n_entries = std::atoi(argv[2]);
  }

  generate_object(obj, n_entries);
  for (auto s : obj) {
    std::cout << s.second << "  ";
  }
  std::cout << "\n";

  std::vector<ObjectList> list;
  sort_objects(obj, list, n_partitions);

  print(list);

  return 0;
}
