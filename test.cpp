#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "btree.h"
#include "utils.h"

class CmdParser {
 public:
  CmdParser() = default;

  void parse() {
    keys_.clear();
    action_.clear();
    std::getline(std::cin, line_);
    iss_ = std::istringstream(line_);

    iss_ >> action_;
    if (action_ == "i" or action_ == "d") {
      int key;
      do {
        iss_ >> key;
        keys_.push_back(key);
      } while (!iss_.eof());
    }
  }

  const std::string &action() const {
    return action_;
  }

  const std::vector<int> &keys() const {
    return keys_;
  }

 private:
  std::string action_;
  std::vector<int> keys_;
  std::string line_;
  std::istringstream iss_;
};

class TestCase {
 public:
  TestCase(int size = 100) {
    strs_.resize(size);
    for (int i = 0; i < strs_.size(); i++) {
      strs_[i] = "s" + std::to_string(i);
    }
  }

  void insert(std::vector<int> keys) {
    printf("After insert");
    for (const auto &key : keys) {
      tree_.insert(key, &strs_[key]);
      printf(" %d", key);
    }
    printf(":\n");

    btree::utils::print_btree(tree_);
    printf("---------------------\n");
  }

  void remove(std::vector<int> keys) {
    printf("After remove");
    for (const auto &key : keys) {
      tree_.remove(key);
      printf(" %d", key);
    }
    printf(":\n");

    btree::utils::print_btree(tree_);
    printf("---------------------\n");
  }

  void test_in_cmd() {
    CmdParser parser;
    int key;
    std::string action;

    while (action != "q") {
      parser.parse();
      action = parser.action();

      if (action == "q") {
        break;
      } else if (action == "ss") {
        tree_.seq_scan();
      } else if (action == "rs") {
        tree_.reverse_seq_scan();
      } else {
        if (action == "i") {
          insert(parser.keys());
        } else if (action == "d") {
          remove(parser.keys());
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
