#! /bin/bash
rm bob
g++ Point.cpp CE.cpp server.cpp sha1.c -o bob -lgmpxx -lgmp
