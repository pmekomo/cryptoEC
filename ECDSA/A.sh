#! /bin/bash
rm alice
g++ Point.cpp CE.cpp client.cpp sha1.c random.c -o alice -lgmpxx -lgmp
