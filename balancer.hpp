#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

template <class Object> class ObjectList {
public:
  using list_type = std::vector<typename Object::iterator>;
  using iterator = typename list_type::iterator;

  void clear() {
    list.clear();
    weight = 0;
  }
  void assign(typename Object::iterator &item) {
    list.push_back(item);
    weight += item->second.status().get().bytes;
  }
  bool operator<(const ObjectList &other) const {
    return weight < other.weight;
  }
  bool operator>(const ObjectList &other) const {
    return weight > other.weight;
  }
  list_type list;
  int weight{0};
};

template <class Object>
void assign_to_ordered_list(Object &obj, ObjectList<Object> &sorted) {
  for (auto o = obj.begin(); o != obj.end(); ++o) {
    sorted.list.push_back(o);
  }
  std::sort(
      sorted.list.begin(), sorted.list.end(),
      [](typename Object::iterator &first, typename Object::iterator &second) {
        return (first->second.status().get().bytes >
                second->second.status().get().bytes);
      });
}

template <class Object>
void balanced_partition(typename ObjectList<Object>::list_type &sorted,
                        std::vector<ObjectList<Object>> &list) {

  for (auto o : sorted) {
    auto min = std::min_element(
        list.begin(), list.end(),
        [](ObjectList<Object> &first, ObjectList<Object> &second) {
          return first < second;
        });
    min->assign(o);
  }
}

template <class Object>
void print(const std::vector<ObjectList<Object>> &list) {
  for (auto &l : list) {
    std::cout << l.weight << "  ->  ";
    std::cout << "( ";
    for (auto &o : l.list) {
      std::cout << o->first << ",";
    }
    std::cout << ")\n";
  }
}

template <class Object>
std::vector<ObjectList<Object>>
create_balanced_partition(Object &obj, const int &n_partitions = 2) {
  ObjectList<Object> sorted;
  assign_to_ordered_list(obj, sorted);

  std::vector<ObjectList<Object>> list;
  list.resize(n_partitions);
  balanced_partition(sorted.list, list);
  return std::move(list);
}

template <class Object>
std::vector<ObjectList<Object>>
create_flat_partition(Object &obj, const int &n_partitions = 2) {
  int assignment{0};
  std::vector<ObjectList<Object>> list;
  list.resize(n_partitions);
  for (auto o = obj.begin(); o != obj.end(); ++o) {
    list[assignment++].list.push_back(o);
    assignment %= list.size();
  }
  return std::move(list);
}
