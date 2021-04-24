A C++ implementation of B+ tree.

**How to Run:**

$ g++ test.cpp -o test && ./test

**Command:**

- i <key1, key2, ...>: insert keys
- d <key1, key2, ...>: delete keys
- s:                   sequential scan
- rs:                  reverse sequential scan

**For Example:**

```
i 7 8 10 3 5 
After insert 7 8 10 3 5:
├──ROOT
│   ├──0007
│   │   ├──0003 s3
│   │   └──0005 s5
│   └──NULL
│       ├──0007 s7
│       ├──0008 s8
│       └──0010 s10
---------------------
d 7
After remove 7:
├──ROOT
│   ├──0008
│   │   ├──0003 s3
│   │   └──0005 s5
│   └──NULL
│       ├──0008 s8
│       └──0010 s10
---------------------
i 12 15 18 2
After insert 12 15 18 2:
├──ROOT
│   ├──0008
│   │   ├──0002 s2
│   │   ├──0003 s3
│   │   └──0005 s5
│   ├──0012
│   │   ├──0008 s8
│   │   └──0010 s10
│   └──NULL
│       ├──0012 s12
│       ├──0015 s15
│       └──0018 s18
---------------------
d 8 12
After remove 8 12:
├──ROOT
│   ├──0005
│   │   ├──0002 s2
│   │   └──0003 s3
│   ├──0015
│   │   ├──0005 s5
│   │   └──0010 s10
│   └──NULL
│       ├──0015 s15
│       └──0018 s18
---------------------
```