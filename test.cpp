#include "btree.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>

void tree_insert(btree::BTree &tree, int key, std::vector<std::string> &strs) {
  tree.insert(key, &strs[key]);
  printf("After insert %d:\n", key);
  btree::utils::print_btree(tree);
  printf("---------------------\n");
}

void test_print() {
  btree::BTree tree;
  std::string s1 = "s1";
  std::string s2 = "s2";
  std::string s3 = "s3";
  std::string s4 = "s4";
  std::string s5 = "s5";

  std::vector<std::string> strs(100);
  for (int i = 0; i < strs.size(); i++) {
    strs[i] = "s" + std::to_string(i);
  }

  tree_insert(tree, 3, strs);
  tree_insert(tree, 1, strs);
  tree_insert(tree, 2, strs);
  tree_insert(tree, 4, strs);
  tree_insert(tree, 5, strs);
  tree_insert(tree, 7, strs);

  tree_insert(tree, 10, strs);
  tree_insert(tree, 8, strs);
  tree_insert(tree, 13, strs);
  tree_insert(tree, 14, strs);
  // tree_insert(tree, 15, strs);

  for (int i = 15; i < 20; i++) {
    tree_insert(tree, i, strs);
  }

  return;
}

int main(void) {
  test_print();

  return 0;
}
