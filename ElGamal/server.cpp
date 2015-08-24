#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE.h"
#include "random.h"
#include <iostream>
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{

//-----------------INITIALISATION DES SOCKETS ET CONNECTION-------------
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
    
     
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
//**********************************************************************    
    

//----------GESTION DU TROUSSEAU DE ALICE (s,P)-------------------------    
     //Sélection de la courbe
	CE ce("w256-001.gp");
    char buffer[256];
    
    //selection de la clé secrète
    cout<<"-----clé secrète de Alice----------"<<endl;
    mpz_t s;
    mpz_init (s);
    do
    {
		cout<<"Entrer un entier entre 1 et "<<ce.getN()<<":"<<endl;
		bzero(buffer,256);
		fgets(buffer,255,stdin);
		mpz_init_set_str(s, buffer, 10);
	}
	while ((mpz_cmpabs_ui (s,1)<0) || (mpz_cmpabs(s,ce.getN().get_mpz_t())>0));
	
	cout<<"-----calcul et envoie de la clé publique de Alice-----"<<endl;
	Point P = ce.mult(s,ce.getGen());
	
	//envoie de Px
     bzero(buffer,256);
     mpz_get_str (buffer, 10, P.getX().get_mpz_t());
      n = write(newsockfd,buffer,255);
     if (n < 0) error("ERROR writing to socket");
     
     //envoie de Py
     bzero(buffer,256);
     mpz_get_str (buffer, 10, P.getY().get_mpz_t());
      n = write(newsockfd,buffer,255);
     if (n < 0) error("ERROR writing to socket");

//**********************************************************************


//---------------RECEPTION DU MESSAGE CHIFFRE (C1, C2) DE BOB-----------
		
	 cout<<"------Reception du chiffré (C1, C2) de Bob------"<<endl;
     mpz_t x, y;
     //reception de C1x
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("C1x= %s\n",buffer);
     mpz_init_set_str(x, buffer, 10);
     
     //reception de C1y
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("C1y= %s\n",buffer);
     mpz_init_set_str(y, buffer, 10);
     
     Point C1(x, y);
     
     cout <<"C1 = ("<<C1.getX()<<", "<<C1.getY()<<")"<<endl;
     
     mpz_t C2;
     mpz_init (C2);
     //reception de C2
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("C2= %s\n",buffer);
     mpz_init_set_str(C2, buffer, 10);
//**********************************************************************

//---------------DECHIFFREMENT DU MESSAGE DE BOB------------------------     
    
    cout<<"-------Dechiffrement--------"<<endl; 
	mpz_t m;
	mpz_init (m);
	Point tmp = ce.mult (s, C1);//sC1
	mpz_sub (m, C2, tmp.getX().get_mpz_t());
	
	cout<<"message de Bob: "<<m<<endl;
//**********************************************************************         
     close(newsockfd);
     close(sockfd);
     
     return 0; 
}
