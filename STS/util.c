#include "util.h"

//lecture de données depuis un fichier
void lire(char nomFic[], char *data, unsigned int  taille)
{
	FILE *f;
	
	f=fopen(nomFic,"rb");
	assert(f!=NULL);
	
	fread(data,1,taille,f);
	
	fclose(f);
	
	return ;
}

//ecriture de données dans un fichier 
void ecrire(char *data, int taille, char nomFic[])
{
	FILE *f;
	
	f=fopen(nomFic,"wb");
	assert(f!=NULL);
	
	fwrite(data,1,taille,f);
	fclose(f);
}

//traitement d'erreur qui terminent le programme
void erreur(const char *a)
{
	printf("Erreur : %s \n\n",a);
		
	exit(EXIT_FAILURE);
}

//création d'un fichier caratéristique
//return 1 si le fichier a une extension png 0 sinon
int creation_fichier(char fic1[] ,int op, char fic2[])
{
	int l=strlen(fic1);
	int i,ret = 0;
	strcpy(fic2,(op == 1 ? "chiffre" : "clair"));
	for(i=0; i<l; i++)
		if(fic1[i] == '.') break;
	if(strcmp(&fic1[i],".png")==0)
		ret = 1;
	if(i<=l)
	{
		memcpy(&fic2[strlen(fic2)],&fic1[i],l-i+1);
	}
	return ret;
}
