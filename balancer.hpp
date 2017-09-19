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
  iterator begin() { return list.begin(); }
  iterator end() { return list.end(); }

  void clear() {
    list.clear();
    weight = 0;
  }
  void assign(typename Object::iterator &item) {
    list.push_back(item);
    weight += item->second.status().get().bytes;
  }
  bool operator!=(const ObjectList &other) const {
    if (list.size() != other.list.size()) {
      return true;
    } else {
      for (size_t i = 0; i < list.size(); ++i) {
        if (list[i] != other.list[i]) {
          return true;
        }
      }
    }
    return false;
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
typename ObjectList<Object>::iterator begin(ObjectList<Object> &list) {
  return list.begin();
}
template <class Object>
typename ObjectList<Object>::iterator end(ObjectList<Object> &list) {
  return list.end();
}

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

template <class Object>
const std::vector<ObjectList<Object>> &
copy_partition(const std::vector<ObjectList<Object>> &source,
               std::vector<ObjectList<Object>> &dest) {
  if (source.size() != dest.size) {
    std::cerr << "Partitions size differs: skip replacement\n";
  } else {
    for (int i = 0; i < source.size(); ++i) {
      dest[i] = source[i];
    }
  }
  return dest;
}
