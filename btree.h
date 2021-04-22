#pragma once

#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace btree {

constexpr int M = 5; // branching factor
constexpr int MAX_KEYS = M - 1;
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

  // no duplicate keys
  bool search(const K &key, V *value) const {
    return search_internel(root, key, value);
  }

  bool search_internel(const BTreeNode *node, const K &key, V *value) const {
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
          return search_internel(node->get_child(i), key, value);
        }
      }

      return search_internel(node->get_child(node->nptrs - 1), key, value);
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
    for (int i = M - 1; i >= 0; i--) {  // M ptrs,keys in total
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
    printf("split_internal_node: key=%d key_pos=%d\n", key, key_pos);

    int nptrs_left = (M + 1) / 2;
    int nptrs_right = (M + 1) - nptrs_left;

    auto &left_node = node;
    auto right_node = new BTreeNode(node->type);

    int nkeys_left = nptrs_left - 1;
    int nkeys_right = nptrs_right - 1;
    assert(nkeys_left > 0 && nkeys_right > 0);

    int idx = M - 1;
    for (int i = M; i >= 0; i--) {  // (M + 1) ptrs in total
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
    for (int i = MAX_KEYS; i >= 0; i--) {  // M keys in total
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

    node->keys[node->nkeys] = key;
    node->ptrs[node->nptrs] = ptr;
    std::swap(node->keys[node->nkeys], node->keys[key_pos]);
    std::swap(node->ptrs[node->nptrs], node->ptrs[ptr_pos]);
    node->nkeys += 1;
    node->nptrs += 1;
  }

  BTreeNode *root;
  int nlevels;
  // int size;  // number of inserted keys
};

} // namespace btree