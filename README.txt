A C++ implementation of B+ tree.

How to Run:
$ g++ test.cpp -o test && ./test

Command:
i <key>: insert a key
d <key>: delete a key
s:       sequential scan
rs:      reverse sequential scan

For Example:
i 5
After insert 5:
├──ROOT
│   └──0005 s5
---------------------
i 8
After insert 8:
├──ROOT
│   ├──0005 s5
│   └──0008 s8
---------------------
i 9
After insert 9:
├──ROOT
│   ├──0005 s5
│   ├──0008 s8
│   └──0009 s9
---------------------
i 3
After insert 3:
├──ROOT
│   ├──0003 s3
│   ├──0005 s5
│   ├──0008 s8
│   └──0009 s9
---------------------
i 12
After insert 12:
├──ROOT
│   ├──0008
│   │   ├──0003 s3
│   │   └──0005 s5
│   └──NULL
│       ├──0008 s8
│       ├──0009 s9
│       └──0012 s12
---------------------
d 8
After remove 8:
├──ROOT
│   ├──0009
│   │   ├──0003 s3
│   │   └──0005 s5
│   └──NULL
│       ├──0009 s9
│       └──0012 s12
---------------------
d 9
After remove 9:
├──ROOT
│   └──NULL
│       ├──0003 s3
│       ├──0005 s5
│       └──0012 s12
---------------------
