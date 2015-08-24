#! /bin/bash
rm alice
g++ Point.cpp CE.cpp client.cpp sha1.c aes.c random.cpp -o alice -lgmpxx -lgmp -lssl
