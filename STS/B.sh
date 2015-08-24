#! /bin/bash
rm bob
g++ Point.cpp CE.cpp server.cpp sha1.c aes.c random.cpp -o bob -lgmpxx -lgmp -lssl
