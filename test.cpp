#include "btree.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> strs(100);

void tree_insert(btree::BTree &tree, int key) {
  tree.insert(key, &strs[key]);
  printf("After insert %d:\n", key);
  btree::utils::print_btree(tree);
  printf("---------------------\n");
}

void tree_remove(btree::BTree &tree, int key) {
  tree.remove(key);
  printf("After remove %d:\n", key);
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


  for (int i = 0; i < strs.size(); i++) {
    strs[i] = "s" + std::to_string(i);
  }

  tree_insert(tree, 3);
  tree_insert(tree, 1);
  tree_insert(tree, 2);
  tree_insert(tree, 4);
  tree_insert(tree, 5);
  tree_insert(tree, 7);

  // tree_insert(tree, 10);
  // tree_insert(tree, 8);
  // tree_insert(tree, 13);
  // tree_insert(tree, 14);
  // tree_insert(tree, 15);

  for (int i = 15; i < 30; i++) {
    tree_insert(tree, i * 2);
  }

  tree_remove(tree, 5);
  // tree_remove(tree, 1);

  // tree_insert(tree, 1);
  // tree_remove(tree, 1);
  // tree_remove(tree, 15);
  // tree_insert(tree, 4);

  // tree_remove(tree, 2);
  // tree_remove(tree, 4);
  // tree_remove(tree, 4);

  return;
}

void test_by_cmd() {
  btree::BTree tree;

  std::vector<std::string> strs(100);
  for (int i = 0; i < strs.size(); i++) {
    strs[i] = "s" + std::to_string(i);
  }

  std::string action;
  int key;
  while (action != "quit") {
    std::cin >> action >> key;
    if (action == "insert") {
      tree_insert(tree, key);
    } else if (action == "remove") {
      tree_remove(tree, key);
    }
  }
}

int main(void) {
  // test_print();
  test_by_cmd();

  return 0;
}
