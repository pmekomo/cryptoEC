#ifndef _UTIL_
#define _UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>


void lire(char nomFic[], char *data, unsigned int  taille);
void ecrire(char *data, int taille, char nomFic[]);
void erreur(const char *a);

int creation_fichier(char fic1[] ,int op, char fic2[]);



#endif
