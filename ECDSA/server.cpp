#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "CE.h"
#include "sha1.h"
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
     
     
     //Courbe elliptique
     CE ce ("w256-001.gp"); 
//--------------RECEPTION DU MESSAGE ET DE LA SIGNATURE-----------------

	 cout<<"------reception du message et de la signature-------"<<endl;
     char buffer[256];    
     //reception de M1
     char M1[256];
     bzero(M1,256);
     int n;
     n = read(newsockfd,M1,255);
     if (n < 0) error("ERROR reading from socket");
     printf("message: %s\n",M1);
     
     mpz_t r1, s1;
     //reception de r1
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("r1= %s\n",buffer);
     mpz_init_set_str(r1, buffer, 10);
     
     //reception de s1
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("s1= %s\n",buffer);
     mpz_init_set_str(s1, buffer, 10);
//**********************************************************************

//------------Reception de la clé publique de Alice---------------------

	 mpz_t Qx, Qy;

	 cout<<"------reception de la clé publique de Alice--------"<<endl;
     //reception de Qx
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Qx= %s\n",buffer);
     mpz_init_set_str(Qx, buffer, 10);
     //reception de Qy
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Qy= %s\n",buffer);
     mpz_init_set_str(Qy, buffer, 10);
     
     Point Q (Qx, Qy);
//**********************************************************************


//-----------VERIFICATION DE LA SIGNATURE-------------------------------
	
     cout<<"-----Vérification de la signature-------"<<endl;
     //calcul de H1 le haché de M1
     char *H1;
     //Hachage du Message
	 H1 = sha1 (M1);
	 //entier e1 de H1
	 mpz_t e1;
	 mpz_init_set_str(e1, H1, 16);
     
     
     
     //calcul de s⁻¹
     mpz_t w;
     mpz_init (w);
     mpz_invert (w, s1, ce.getN().get_mpz_t()); 
     
     //calcul de u1 et u2
     mpz_t u1, u2;
     mpz_init (u1);
     mpz_init (u2);
     mpz_mul (u1, e1, w);
     mpz_mod (u1, u1, ce.getN().get_mpz_t()); 
     mpz_mul (u2, r1, w); 
     mpz_mod (u2, u2, ce.getN().get_mpz_t());
     
     Point R = (ce.add(ce.mult (u1, ce.getGen()), ce.mult (u2, Q)));
     
     mpz_t v;
     mpz_init (v);
     mpz_mod (v, R.getX().get_mpz_t(), ce.getN().get_mpz_t());
     
     //VERIFICATION
     cout<<"v="<<v<<endl;
	 cout<<"r1="<<r1<<endl;
     if ((mpz_cmpabs_ui (r1,1)>0) && (mpz_cmpabs(r1,ce.getN().get_mpz_t())<0))
		if((mpz_cmpabs_ui (s1,1)>0) && (mpz_cmpabs(s1,ce.getN().get_mpz_t())<0))
		{
			if (mpz_cmpabs(v, r1)==0)
			{
				
				n = write(newsockfd,"I got your real message",23);
				if (n < 0) error("ERROR writing to socket");
				sleep(1);
				close(newsockfd);
				close(sockfd);
				return 0;
			}
		}
     
     n = write(newsockfd,"I didn't got your real message", 30);
     if (n < 0) error("ERROR writing to socket");
//**********************************************************************

     close(newsockfd);
     close(sockfd);
     return 0; 
}
