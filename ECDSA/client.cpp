#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include "CE.h"
#include <time.h>
#include "sha1.h"
#include "random.h"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{

//--------------INITIALISATION DU SOCKET ET CONNECTION------------------
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
//**********************************************************************


	
    //GENERATION DE LA CLE
    CE ce ("w256-001.gp");

//------------------------(d, Q) de Alice-------------------------------

	char buffer[256];
    cout<<"-------selection de d--------------"<<endl;
    mpz_t d;
    mpz_init (d);
    do
    {
		cout<<"Entrer un entier entre 1 et "<<ce.getN()<<":"<<endl;
		bzero(buffer,256);
		fgets(buffer,255,stdin);
		mpz_init_set_str(d, buffer, 10);
	}
	while ((mpz_cmpabs_ui (d,1)<0) || (mpz_cmpabs(d,ce.getN().get_mpz_t()-1)>0));
	
	cout<<"------calcul de Q-----------"<<endl;
	
	Point Q = ce.mult(d,ce.getGen()); // Q = d.G
	
	//clé publique Q clé secrète d
	

	char *H, M[256];
	mpz_t r, s;
	cout<<"Entrer un message:"<<endl;
	bzero(M,256);
	fgets(M,255,stdin);

//-------------------------SIGNATURE------------------------------------

	cout<<"-----Signature du message--------"<<endl;
	do
	{
		//GENERATION ALEATOIRE DE t ET CALCULE DE t⁻¹
		mpz_t t,t_; // t et t⁻¹
		mpz_init(t);
		mpz_init(t_);
		gmp_randstate_t r_state;
		gmp_randinit_mt (r_state);
		random_seeding(r_state);
		mpz_urandomm(t , r_state ,ce.getN().get_mpz_t());
		gmp_randclear(r_state);
		
		mpz_invert (t_, t, ce.getN().get_mpz_t()); //calcul de t⁻¹
		Point tP = ce.mult(t,ce.getGen());
		
		mpz_init(r);
		mpz_set (r, tP.getX().get_mpz_t());
		mpz_mod (r, r, ce.getN().get_mpz_t());
		
		
		//Hachage du Message
		H = sha1 (M);
		
		mpz_t e;
		mpz_init (s);
		mpz_init_set_str(e, H, 16);
		cout <<"\ne:"<<e<<endl;
		mpz_mul (s, d, r); 
		mpz_add (s, e, s);
		mpz_mul (s, s, t_);
		mpz_mod (s, s, ce.getN().get_mpz_t());
		
	}while ((mpz_cmpabs_ui (r,0)==0) || (mpz_cmpabs_ui (s,0)==0));

//**********************************************************************

//-------------ENVOIE DU MESSAGE ET DE LA SIGNATURE---------------------

	cout<<"-----Envoie du Message M et de la signature (r, s)-----"<<endl;
	//envoie de M
    n = write(sockfd,M,strlen(M));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
    //envoie de r     
    mpz_get_str (buffer, 10, r);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
   //envoie de s
   mpz_get_str (buffer, 10, s);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
//**********************************************************************

//-----------------ENVOIE DE LA CLE PUBLIQUE DE ALICE Q-----------------

	cout<<"-------Envoie de la clé publique Q---------"<<endl;	 
    //envoie de Qx     
    mpz_get_str (buffer, 10, Q.getX().get_mpz_t());
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
     sleep(1);
    //envoie de Qy     
    mpz_get_str (buffer, 10, Q.getY().get_mpz_t());
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    sleep(1);
//**********************************************************************
         
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
