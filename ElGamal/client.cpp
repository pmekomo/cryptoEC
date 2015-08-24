#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <gmpxx.h>
#include "CE.h"
#include "random.h"
#include <iostream>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
//-------------------INITIALISATION DE LA SOCKET ET CONNECTION----------	
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Connexion réussie\n");
//**********************************************************************
    
    //Sélection de la courbe
	CE ce("w256-001.gp");
	
	//Sélection de k aléatoire
	mpz_t k;
	mpz_init(k);
	gmp_randstate_t r_state;
	gmp_randinit_mt (r_state);
	random_seeding(r_state);
	mpz_urandomm(k , r_state ,ce.getN().get_mpz_t());
	gmp_randclear(r_state);
	cout <<"k="<<k<<endl;

//------------------RECEPTION DE LA CLE PUBLIQUE DE ALICE---------------
	cout<<"------Reception de la clé publique P de Alice-------"<<endl;
	char buffer[256];
	 mpz_t x, y;
    //reception de Px
     bzero(buffer,256);
     n = read(sockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Px= %s\n",buffer);
     mpz_init_set_str(x, buffer, 10);
     
     //reception de Py
     bzero(buffer,256);
     n = read(sockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Py= %s\n",buffer);
     mpz_init_set_str(y, buffer, 10);
     
     Point P(x, y);
     cout <<"P = ("<<P.getX()<<", "<<P.getY()<<")"<<endl;
//**********************************************************************

//------------------CALCUL DU CHIFFRE (C1, C2)--------------------------

	 cout <<"--------Calcul et envoie du chiffré (C1, C2)------------"<<endl;
     Point C1;
	
	 C1 = ce.mult(k, ce.getGen());
	 
	 char M[256];
	 mpz_t m;
	 mpz_init(m);
	 cout<<"Entrer un message:";
	 bzero(M,256);
	 fgets(M,255,stdin);
	 mpz_init_set_str(m, M, 10);
	 
	 mpz_t C2;
	 mpz_init(C2);
	 Point tmp = ce.mult(k, P);
	 mpz_add (C2, tmp.getX().get_mpz_t(), m);
	
	//envoie de C1x
    bzero(buffer,256);
    mpz_get_str (buffer, 10, C1.getX().get_mpz_t());
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
    
    //envoie de C1y
    bzero(buffer,256);
    mpz_get_str (buffer, 10, C1.getY().get_mpz_t());
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
    
    //envoie de C2
    bzero(buffer,256);
    mpz_get_str (buffer, 10, C2);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
            
    cout<<endl;
    close(sockfd);
    
    return 0;
}
