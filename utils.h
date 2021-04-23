#pragma once

#include <iostream>
#include <string>
#include "btree.h"

namespace btree {
namespace utils {

constexpr int END_MARK = 8888;
constexpr int ROOT_MARK = 6666;

void print_btree(std::string prefix, const BTreeNode *node, K key, bool is_last,
                 bool is_value) {
  if (node != nullptr) {
    std::cout << prefix;

    std::cout << (is_last ? "└──" : "├──");

    // print the value of the node
    switch (key) {
      case ROOT_MARK:
        printf("ROOT");
        break;
      case END_MARK:
        printf("NULL");
        break;
      default:
        printf("%04d", key);
    }

    if (is_value) {
      std::cout << " " << *(const V *)node << std::endl;
      return;
    }
    std::cout << std::endl;

    // enter the next tree level - m branch
    prefix = prefix + (is_last ? "    " : "│   ");
    for (int i = 0; i < node->nkeys; i++) {
      is_last = (i == node->nptrs - 1);
      print_btree(prefix, node->get_child(i), node->keys[i], is_last,
                  node->is_leaf());
    }

    if (node->nptrs > node->nkeys) {
      int idx = node->nptrs - 1;
      print_btree(prefix, node->get_child(idx), END_MARK, true,
                  node->is_leaf());
    }
  }
}

void print_btree(const BTree &tree) {
  print_btree("", tree.root, ROOT_MARK, false, false);
}

}  // namespace utils
}  // namespace btree