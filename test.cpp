#include "btree.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>

class TestCase {
public:
  TestCase(int size = 100) {
    strs_.resize(size);
    for (int i = 0; i < strs_.size(); i++) {
      strs_[i] = "s" + std::to_string(i);
    }
  }

  void insert(int key) {
    tree_.insert(key, &strs_[key]);
    printf("After insert %d:\n", key);
    btree::utils::print_btree(tree_);
    printf("---------------------\n");
  }

  void remove(int key) {
    tree_.remove(key);
    printf("After remove %d:\n", key);
    btree::utils::print_btree(tree_);
    printf("---------------------\n");
  }

  void test_print() {
    insert(3);
    insert(1);
    insert(2);
    insert(4);
    insert(5);
    insert(7);

    remove(5);

    return;
  }

  void test_in_cmd() {
    std::string action;
    int key;
    while (action != "q") {
      std::cin >> action;
      if (action == "q") {
        break;
      } else if (action == "s") {
        tree_.sequential_scan();
      } else {
        std::cin >> key;
        if (action == "i") {
          insert(key);
        } else if (action == "d") {
          remove(key);
        }
      }
    }
  }

private:
  std::vector<std::string> strs_;
  btree::BTree tree_;
};

int main(void) {
  TestCase test;
  test.test_in_cmd();

  return 0;
}
