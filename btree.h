#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace btree {

constexpr int M = 5; // branching factor
constexpr int MIN_PTRS = (M + 1) / 2;

constexpr int MAX_KEYS = M - 1;
constexpr int MIN_KEYS = M / 2;

constexpr int NULL_KEY = -1;

using K = int;
using V = std::string;
using Ptr = void *;

struct KeyComparator {
  static int compare(const K &k1, const K &k2) { return k1 - k2; }
};

using C = KeyComparator;

// template <class K>
struct BTreeNode {
  // int level = 0;
  char type;     // 0 for internal node, 1 for leaf node
  int nkeys = 0; // number of inserted keys
  int nptrs = 0; // number of links

  K keys[M - 1];
  Ptr ptrs[M];

  BTreeNode(int type = 0) : type(type) { std::fill_n(ptrs, M, nullptr); }

  inline const V *get_value(int idx) const {
    return static_cast<const V *>(ptrs[idx]);
  }

  inline BTreeNode *get_child(int idx) const {
    return static_cast<BTreeNode *>(ptrs[idx]);
  }

  inline void insert(const K &key, const Ptr &ptr) {
    assert(nkeys < MAX_KEYS);

    if (key != NULL_KEY) {
      keys[nkeys] = key;
      nkeys += 1;
    }

    if (ptr != nullptr) {
      ptrs[nptrs] = ptr;
      nptrs += 1;
    }
  }

  inline bool is_leaf() const { return type == 1; }
};

// template <class K>
struct BTree {
  BTree() {
    root = new BTreeNode(1); // is also a leaf node
  }

  ~BTree() { deallocate(root); }

  void deallocate(BTreeNode *node) {
    if (node->is_leaf()) {
      // it is responsible for user to deallocate value pointers
      delete node;
    } else {
      for (int i = 0; i < node->nptrs; i++) {
        deallocate((BTreeNode *)(node->ptrs[i]));
      }
      delete node;
    }
  }

  // no duplicate keys
  bool search(const K &key, V *value) const {
    return search_impl(root, key, value);
  }

  bool search_impl(const BTreeNode *node, const K &key, V *value) const {
    if (node == nullptr || node->nptrs == 0) {
      return false;
    }

    if (node->is_leaf()) { // leaf
      for (int i = 0; i < node->nkeys; i++) {
        if (C::compare(key, node->keys[i]) == 0) { // k == ki
          *value = *(node->get_value(i));
          return true;
        }
      }

    } else {
      for (int i = 0; i < node->nkeys; i++) {
        if (C::compare(key, node->keys[i]) < 0) { // k < ki
          return search_impl(node->get_child(i), key, value);
        }
      }

      return search_impl(node->get_child(node->nptrs - 1), key, value);
    }

    return false; // could not find key
  }

  bool search_path(BTreeNode *node, const K &key,
                   std::vector<BTreeNode *> *path) {
    assert(node != nullptr);

    path->push_back(node);
    if (node->is_leaf()) { // leaf
      return true;
    } else {
      for (int i = 0; i < node->nkeys; i++) {
        if (C::compare(key, node->keys[i]) < 0) { // k < ki
          return search_path(node->get_child(i), key, path);
        }
      }

      return search_path(node->get_child(node->nptrs - 1), key, path);
    }

    return false; // could not find key
  }

  int find_key_pos(const BTreeNode *node, const K &key) {
    int key_pos;
    for (key_pos = 0; key_pos < node->nkeys; key_pos++) {
      if (C::compare(key, node->keys[key_pos]) < 0) {
        break;
      }
    }
    return key_pos;
  }

  bool sequential_scan() {
    auto node = root;
    while (!node->is_leaf()) {
      node = node->get_child(0);
    }

    int id = 0;
    while (node != nullptr) {
      assert(node->is_leaf());

      std::cout << "Leaf " << id++ << ": ";
      for (int i = 0; i < node->nkeys; i++) {
        std::cout << node->keys[i] << " ";
      }
      std::cout << std::endl;
      node = node->get_child(M - 1);
    }

    return true;
  }

  bool insert(const K &key, V *value) {
    std::vector<BTreeNode *> path;
    search_path(root, key, &path);
    return insert_impl(key, value, &path);
  }

  bool insert_impl(const K &key, const Ptr &ptr,
                   std::vector<BTreeNode *> *path) {
    assert(path->size() > 0);
    auto node = path->back();
    path->pop_back();

    if (node->nkeys < MAX_KEYS) {
      insert_into_node(node, key, ptr);
    } else {
      BTreeNode *new_node;
      K new_key;
      if (node->is_leaf()) {
        split_leaf_node(node, key, ptr, &new_node, &new_key);
      } else {
        split_internal_node(node, key, ptr, &new_node, &new_key);
      }

      if (path->empty()) {
        auto new_root = new BTreeNode(0); // not a leaf node
        new_root->keys[0] = new_key;
        new_root->ptrs[0] = node;
        new_root->ptrs[1] = new_node;
        new_root->nkeys = 1;
        new_root->nptrs = 2;

        root = new_root;
      } else {
        insert_impl(new_key, new_node, path);
      }
    }

    return true;
  }

  bool split_leaf_node(BTreeNode *node, const K &key, const Ptr &ptr,
                       BTreeNode **new_node, K *new_key) {
    assert(node->nkeys == MAX_KEYS);

    int key_pos = find_key_pos(node, key);
    int nptrs_left = (node->nptrs + 1) / 2;
    int nptrs_right = (node->nptrs + 1) - nptrs_left;

    auto &left_node = node;
    auto right_node = new BTreeNode(node->type);

    right_node->ptrs[M - 1] = left_node->ptrs[M - 1]; // copy the next node link
    left_node->ptrs[M - 1] = right_node;              // point to the right node

    int idx = M - 2;
    for (int i = M - 1; i >= 0; i--) { // M ptrs,keys in total
      Ptr p;
      K k;
      if (i == key_pos) {
        p = ptr;
        k = key;
      } else {
        assert(idx >= 0);
        p = node->ptrs[idx];
        k = node->keys[idx];
        idx -= 1;
      }

      if (i < nptrs_left) {
        node->ptrs[i] = p;
        node->keys[i] = k;
      } else {
        right_node->ptrs[i - nptrs_left] = p;
        right_node->keys[i - nptrs_left] = k;
      }
    }
    assert(idx == -1);

    left_node->nkeys = nptrs_left;
    left_node->nptrs = nptrs_left;

    right_node->nkeys = nptrs_right;
    right_node->nptrs = nptrs_right;

    *new_node = right_node;
    *new_key = right_node->keys[0];
    return true;
  }

  bool split_internal_node(BTreeNode *node, const K &key, const Ptr &ptr,
                           BTreeNode **new_node, K *new_key) {
    assert(node->nkeys == MAX_KEYS);
    assert(node->nptrs == M);

    int key_pos = find_key_pos(node, key);
    int ptr_pos = key_pos + 1;

    int nptrs_left = (M + 1) / 2;
    int nptrs_right = (M + 1) - nptrs_left;

    auto &left_node = node;
    auto right_node = new BTreeNode(node->type);

    int nkeys_left = nptrs_left - 1;
    int nkeys_right = nptrs_right - 1;
    assert(nkeys_left > 0 && nkeys_right > 0);

    int idx = M - 1;
    for (int i = M; i >= 0; i--) { // (M + 1) ptrs in total
      Ptr p;
      if (i == ptr_pos) {
        p = ptr;
      } else {
        assert(idx >= 0);
        p = node->ptrs[idx];
        idx -= 1;
      }

      if (i < nptrs_left) {
        node->ptrs[i] = p;
      } else {
        right_node->ptrs[i - nptrs_left] = p;
      }
    }
    assert(idx == -1);

    *new_key = 0;

    idx = MAX_KEYS - 1;
    for (int i = MAX_KEYS; i >= 0; i--) { // M keys in total
      K k;
      if (i == key_pos) {
        k = key;
      } else {
        assert(idx >= 0);
        k = node->keys[idx];
        idx -= 1;
      }

      // skip the middle one
      if (i < nkeys_left) {
        node->keys[i] = k;
      } else if (i > nkeys_left) {
        right_node->keys[i - nkeys_left - 1] = k;
      } else {
        *new_key = k;
      }
    }
    assert(idx == -1);
    assert(*new_key != 0);

    left_node->nkeys = nkeys_left;
    left_node->nptrs = nptrs_left;

    right_node->nkeys = nkeys_right;
    right_node->nptrs = nptrs_right;

    *new_node = right_node;

    return true;
  }

  bool insert_into_node(BTreeNode *node, const K &key, const Ptr &ptr) {
    int key_pos = find_key_pos(node, key);
    int ptr_pos = key_pos + node->nptrs - node->nkeys;

    memmove(node->keys + key_pos + 1, node->keys + key_pos,
            sizeof(K) * (node->nkeys - key_pos));
    memmove(node->ptrs + ptr_pos + 1, node->ptrs + ptr_pos,
            sizeof(Ptr) * (node->nptrs - ptr_pos));

    node->keys[key_pos] = key;
    node->ptrs[ptr_pos] = ptr;

    node->nkeys += 1;
    node->nptrs += 1;

    std::cout << "next ptr: " << node->ptrs[M - 1] << std::endl;

    return true;
  }

  bool remove(const K &key) {
    std::vector<BTreeNode *> path;
    if (!search_path(root, key, &path)) {
      return false;
    }

    const auto &node = path.back();

    int key_pos;
    for (key_pos = 0; key_pos < node->nkeys; key_pos++) {
      if (C::compare(key, node->keys[key_pos]) == 0) {
        break;
      }
    }
    if (key_pos == node->nkeys) {
      return false;
    }

    return remove_impl(key_pos, &path);
  }

  bool remove_impl(const int ptr_pos, std::vector<BTreeNode *> *path) {
    assert(path->size() > 0);

    auto node = path->back();
    path->pop_back();

    bool is_root = path->empty();

    bool remove_first;
    K min_key;
    remove_in_node(node, ptr_pos, &min_key);

    if (ptr_pos == 0) {
      if (!is_root) {
        auto parent = path->back();
        update_key(parent, min_key);
      }
    }

    if (node->nkeys < MIN_KEYS && !is_root) {
      // re-arange or merge
      auto parent = path->back(); // must be an internal node
      int key_pos = find_key_pos(parent, min_key) - 1;
      int left_pos = key_pos;      // left ptr pos
      int right_pos = key_pos + 2; // right ptr pos

      // borrow from left
      if (left_pos >= 0) {
        auto left_node = parent->get_child(left_pos);
        if (left_node->nkeys > MIN_KEYS) {
          return rearange_left(parent, left_node, node, key_pos);
        }
      }

      // borrow from right
      if (right_pos < parent->nptrs) {
        auto right_node = parent->get_child(right_pos);
        if (right_node->nkeys > MIN_KEYS) {
          return rearange_right(parent, right_node, node, key_pos);
        }
      }

      // merge to left
      if (left_pos >= 0) {
        auto left_node = parent->get_child(left_pos);
        assert(left_node->nkeys + node->nkeys <= MAX_KEYS);

        merge_left(parent, left_node, node, key_pos);
        return remove_impl(key_pos + 1, path);
      }

      // merge to right
      if (right_pos < parent->nptrs) {
        auto right_node = parent->get_child(right_pos);
        assert(right_node->nkeys + node->nkeys <= MAX_KEYS);

        merge_right(parent, right_node, node, key_pos);
        return remove_impl(key_pos + 1, path);
      }
    }

    return true;
  }

  bool remove_in_node(BTreeNode *node, const int ptr_pos, K *min_key) {
    if (!node->is_leaf()) {
      *min_key = node->keys[0];
    }

    int key_pos = node->is_leaf() ? ptr_pos : std::max(ptr_pos - 1, 0);
    memmove(node->keys + key_pos, node->keys + key_pos + 1,
            sizeof(K) * (node->nkeys - 1 - key_pos));
    memmove(node->ptrs + ptr_pos, node->ptrs + ptr_pos + 1,
            sizeof(Ptr) * (node->nptrs - 1 - ptr_pos));

    node->nkeys -= 1;
    node->nptrs -= 1;

    if (node->is_leaf()) {
      *min_key = node->keys[0];
    }

    return true;
  }

  bool update_key(BTreeNode *node, const K &key) {
    int key_pos;
    for (key_pos = 0; key_pos < node->nkeys; key_pos++) {
      if (C::compare(key, node->keys[key_pos]) < 0) {
        break;
      }
    }
    key_pos = key_pos - 1;
    if (key_pos < 0) {
      return true;
    }

    assert(key_pos < node->nkeys); // could not find key
    node->keys[key_pos] = key;
    return true;
  }

  bool rearange_left(BTreeNode *parent, BTreeNode *left_node, BTreeNode *node,
                     const int key_pos) {
    /// move the rightmost K,V from left_node to node

    // right shift one to make room for the new coming key and ptr
    memmove(node->keys + 1, node->keys, sizeof(K) * node->nkeys);
    memmove(node->ptrs + 1, node->ptrs, sizeof(Ptr) * node->nptrs);

    if (node->is_leaf()) {
      node->keys[0] = left_node->keys[left_node->nkeys - 1]; // leaf
    } else {
      node->keys[0] = parent->keys[key_pos];
    }
    node->ptrs[0] = left_node->ptrs[left_node->nptrs - 1];
    parent->keys[key_pos] =
        left_node->keys[left_node->nkeys - 1]; // update parent

    node->nkeys += 1;
    node->nptrs += 1;
    left_node->nkeys -= 1;
    left_node->nptrs -= 1;

    return true;
  }

  bool rearange_right(BTreeNode *parent, BTreeNode *right_node, BTreeNode *node,
                      const int key_pos) {
    /// move the leftmost K,V from right_node to node

    if (node->is_leaf()) {
      node->keys[node->nkeys] = right_node->keys[0];
    } else {
      node->keys[node->nkeys] = parent->keys[key_pos + 1];
    }
    node->ptrs[node->nptrs] = right_node->ptrs[0];
    parent->keys[key_pos + 1] =
        right_node->keys[node->is_leaf() ? 1 : 0]; // update parent

    right_node->nkeys -= 1;
    right_node->nptrs -= 1;

    memmove(right_node->keys, right_node->keys + 1,
            sizeof(K) * right_node->nkeys);
    memmove(right_node->ptrs, right_node->ptrs + 1,
            sizeof(Ptr) * right_node->nptrs);

    node->nkeys += 1;
    node->nptrs += 1;

    return true;
  }

  bool merge_left(BTreeNode *parent, BTreeNode *left_node, BTreeNode *node,
                  const int key_pos) {
    memcpy(left_node->ptrs + left_node->nptrs, node->ptrs,
           sizeof(Ptr) * node->nptrs);
    if (node->is_leaf()) {
      memcpy(left_node->keys + left_node->nkeys, node->keys,
             sizeof(K) * node->nkeys);

      left_node->ptrs[M - 1] = node->ptrs[M - 1]; // next ptr
      left_node->nkeys += node->nkeys;
    } else {
      left_node->keys[left_node->nkeys] = parent->keys[key_pos];
      memcpy(left_node->keys + left_node->nkeys + 1, node->keys,
             sizeof(K) * node->nkeys);

      left_node->nkeys += node->nkeys + 1;
    }

    left_node->nptrs += node->nptrs;

    return true;
  }

  bool merge_right(BTreeNode *parent, BTreeNode *right_node, BTreeNode *node,
                   const int key_pos) {
    // right shift to make room for node ptrs
    memmove(right_node->ptrs + node->nptrs, right_node->ptrs,
            sizeof(Ptr) * right_node->nptrs);
    memcpy(right_node->ptrs, node->ptrs, sizeof(Ptr) * node->nptrs);

    if (node->is_leaf()) {
      memmove(right_node->keys + node->nkeys, right_node->keys,
              sizeof(K) * right_node->nkeys);
      memcpy(right_node->keys, node->keys, sizeof(K) * node->nkeys);

      if (key_pos - 1 >= 0) {
        parent->get_child(key_pos - 1)->ptrs[M - 1] = right_node;
      }
      right_node->nkeys += node->nkeys;
    } else {
      memmove(right_node->keys + node->nkeys + 1, right_node->keys,
              sizeof(K) * right_node->nkeys);
      memcpy(right_node->keys, node->keys, sizeof(K) * node->nkeys);
      right_node->keys[node->nkeys] = parent->keys[key_pos + 1];

      right_node->nkeys += node->nkeys + 1;
    }
    parent->keys[key_pos + 1] = parent->keys[key_pos];

    right_node->nptrs += node->nptrs;
    return true;
  }

  BTreeNode *root;
  int nlevels;
  // int size;  // number of inserted keys
};

} // namespace btree