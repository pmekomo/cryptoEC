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
#include <fstream>
#include "aes.h"
#include "random.h"

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


 int send_file(int socket){

   FILE *file;
   int size, read_size, stat, packet_index;
   char send_buffer[10240], read_buffer[256];
   packet_index = 1;

   file = fopen("encBA.txt", "rb");
   printf("Getting file Size\n");   

   if(file == NULL) {
        printf("Error Opening file File"); } 

   fseek(file, 0, SEEK_END);
   size = ftell(file);
   fseek(file, 0, SEEK_SET);
   printf("Total file size: %i\n",size);

   //Send file Size
   printf("Sending file Size\n");
   write(socket, (void *)&size, sizeof(int));

   //Send file as Byte Array
   printf("Sending file as Byte Array\n");

   do { //Read while we get errors that are due to signals.
      stat=read(socket, &read_buffer , 255);
      printf("Bytes read: %i\n",stat);
   } while (stat < 0);

   printf("Received data in socket\n");
   printf("Socket data: %c\n", read_buffer);

   while(!feof(file)) {
   //while(packet_index = 1){
      //Read from the file into our send buffer
      read_size = fread(send_buffer, 1, sizeof(send_buffer)-1, file);

      //Send data through our socket 
      do{
        stat = write(socket, send_buffer, read_size);  
      }while (stat < 0);


      packet_index++;  

      //Zero out our send buffer
      bzero(send_buffer, sizeof(send_buffer));
     }
 	
 }

int receive_file(int socket)
{ // Start function 

	int buffersize = 0, recv_size = 0,size = 0, read_size, write_size, packet_index =1,stat;

	char imagearray[10241],verify = '1';
	FILE *file;

	//Find the size of the file
	do{
	stat = read(socket, &size, sizeof(int));
	}while(stat<0);

	char buffer[] = "Got it";
	//Send our verification signal
	do{
	stat = write(socket, &buffer, sizeof(int));
	}while(stat<0);

	printf("Reply sent\n");
	printf(" \n");

	file = fopen("encAB_2.txt", "wb");

	if( file == NULL) {
	printf("Error has occurred. file file could not be opened\n");
	return -1; }

	//Loop while we have not received the entire file yet


	int need_exit = 0;
	struct timeval timeout = {10,0};

	fd_set fds;
	int buffer_fd, buffer_out;

	while(recv_size < size) {
	//while(packet_index < 2){

		FD_ZERO(&fds);
		FD_SET(socket,&fds);

		buffer_fd = select(FD_SETSIZE,&fds,NULL,NULL,&timeout);

		if (buffer_fd < 0)
		   printf("error: bad file descriptor set.\n");

		if (buffer_fd == 0)
		   printf("error: buffer read timeout expired.\n");

		if (buffer_fd > 0)
		{
			do{
				   read_size = read(socket,imagearray, 10241);
			}while(read_size <0);

			printf("Packet number received: %i\n",packet_index);
			printf("Packet size: %i\n",read_size);


			//Write the currently read data into our file file
			 write_size = fwrite(imagearray,1,read_size, file);
			 printf("Written file size: %i\n",write_size); 

			 if(read_size !=write_size) {
				 printf("error in read write\n");    }


				 //Increment the total number of bytes read
				 recv_size += read_size;
				 packet_index++;
				 printf("Total received file size: %i\n",recv_size);
				 printf(" \n");
				 printf(" \n");
		}

	}


	  fclose(file);
	  printf("file successfully Received!\n");
	  return 1;
  }

int main(int argc, char *argv[])
{

//---------------INITIALISATION DES SOCKETS ET CONNNECTION-------------
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

//======================DIFFIE-HELMAN - CLE PRIVE/CLE PUBLIQUE =========
	cout<<"------Diffie-Helman-----"<<endl;
	char buffer[256];
     mpz_t x, y;
     cout<<"------Reception de A-------------"<<endl;
     //reception de Ax
     bzero(buffer,256);
     int n;
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     mpz_init_set_str(x, buffer, 10);
     
     //reception de Ay
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     mpz_init_set_str(y, buffer, 10);
     
     Point A(x, y);
     
     cout <<"A = ("<<A.getX()<<", "<<A.getY()<<")"<<endl;
     
     
     //Sélection de la courbe
	CE ce("w256-001.gp");
	
	Point B;
	//Sélection de la clé secrète
	cout<<"------selection aléatoire de la clé privée----"<<endl;
	mpz_t b;
	mpz_init(b);
	gmp_randstate_t b_state;
	gmp_randinit_mt (b_state);
	random_seeding(b_state);
	mpz_urandomm(b , b_state ,ce.getN().get_mpz_t());
	gmp_randclear(b_state);
	
	cout<<"------calcul de B-----------"<<endl;
	 B = ce.mult(b, ce.getGen());
	 cout<<"\nAffichage de B:"<<endl;
     cout <<"B = ("<<B.getX()<<", "<<B.getY()<<")"<<endl;
     
     cout<<"-------Envoie de B----------"<<endl;
	 //envoie de Bx
     bzero(buffer,256);
     mpz_get_str (buffer, 10, B.getX().get_mpz_t());
      n = write(newsockfd,buffer,255);
     if (n < 0) error("ERROR writing to socket");
     sleep(1);
     
     //envoie de By
     bzero(buffer,256);
     mpz_get_str (buffer, 10, B.getY().get_mpz_t());
      n = write(newsockfd,buffer,255);
     if (n < 0) error("ERROR writing to socket");
     sleep(1);
     
     
     //On calcule la Shared Key et on la met dans un fichier
     cout<<"------Calcul de la clé partagée en enregistrement dans le fichier cleB.txt"<<endl;
     Point S;
     S = ce.mult(b, A);
     char cle[256];
     bzero (cle, 256);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, S.getX().get_mpz_t());
     strcat (cle, buffer);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, S.getY().get_mpz_t());
     strcat (cle, buffer);
     FILE * f= NULL;
     f = fopen("cleB.txt","wb");
	 if(f == NULL)
		error("Erreur Création fichier cleB.txt\n");
	fwrite(cle,sizeof(char),strlen(cle),f);
	fclose(f);
     

//**********************************************************************
     
     //(B,A)
     char BA[256];
     bzero (BA, 256);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, B.getX().get_mpz_t());
     strcat (BA, buffer);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, B.getY().get_mpz_t());
     strcat (BA, buffer);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, A.getX().get_mpz_t());
     strcat (BA, buffer);
     bzero(buffer,256);
     mpz_get_str (buffer, 10, A.getY().get_mpz_t());
     strcat (BA, buffer);
         
    char *H;
    mpz_t r, s;
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
	
	Point Pb = ce.mult(d,ce.getGen()); // Pb = d.G
	
	//clé publique Pb clé secrète d
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
		/*SHA256_CTX ctx;

		sha256_init(&ctx);
		sha256_update(&ctx,(unsigned char *)BA,strlen(BA));
		sha256_final(&ctx,(unsigned char *)H);*/
		H = sha1(BA);
		
		mpz_t e;
		mpz_init (s);
		mpz_init_set_str(e, H, 16);
		cout<<"e:"<<e<<endl;
		
		mpz_mul (s, d, r); 
		mpz_add (s, e, s);
		mpz_mul (s, s, t_);
		mpz_mod (s, s, ce.getN().get_mpz_t());
		
	}while ((mpz_cmpabs_ui (r,0)==0) || (mpz_cmpabs_ui (s,0)==0));
	

//========================CHIFFREMENT AES-CTR==============================	
	f= NULL;
	f = fopen("cleB.txt","rb");
	unsigned char k[256];
	if(f == NULL)
		error("Erreur ouverture du fichier de cle\n");
	fread(k,sizeof(unsigned char),256,f);
	fclose(f);
	
	
	f = fopen("BA.txt","wb");
	if(f == NULL)
		error("Erreur Création fichier BA.txt\n");
	fwrite(BA,sizeof(unsigned char),strlen(BA),f);
	fclose(f);
    cout<<"Here is BA:"<<BA<<endl;
     
	 aesEncrypt("BA.txt","encBA.txt", k);
	 
	 //envoie de Ek(Sb(B, A))
     send_file (newsockfd);
	 sleep(1);
     
     //envoie de r
     mpz_get_str (buffer, 10, r);
      n = write(newsockfd,buffer,strlen(buffer));
     if (n < 0) error("ERROR writing to socket");
     sleep(1);
     
     //envoie de s
     mpz_get_str (buffer, 10, s);
      n = write(newsockfd,buffer,strlen(buffer));
     if (n < 0) error("ERROR writing to socket");
     sleep(1);
     
     //envoie de PbX     
     mpz_get_str (buffer, 10, Pb.getX().get_mpz_t());
     n = write(newsockfd,buffer,strlen(buffer));
     if (n < 0) 
         error("ERROR writing to socket");
     sleep(1);
     //envoie de PbY     
     mpz_get_str (buffer, 10, Pb.getY().get_mpz_t());
     n = write(newsockfd,buffer,strlen(buffer));
     if (n < 0) 
         error("ERROR writing to socket");
     sleep(1);
     
     bzero(buffer, 256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("%s\n",buffer);
     
     
//**********************************************************************    
     
     //reception de Ek(Sa(A, B)
     receive_file(newsockfd);
     
     mpz_t r1, s1, Pax, Pay;
     //reception de r1
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is r1: %s\n",buffer);
     mpz_init_set_str(r1, buffer, 10);
     
     //reception de s1
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is s1: %s\n",buffer);
     mpz_init_set_str(s1, buffer, 10);
     
     //reception de Pax
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is Pax: %s\n",buffer);
     mpz_init_set_str(Pax, buffer, 10);
     //reception de Pay
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is Pay: %s\n",buffer);
     mpz_init_set_str(Pay, buffer, 10);
     
     Point Pa (Pax, Pay);
     
     //Déchiffrement de Ek (Sb(A, B))
     f= NULL;
	f = fopen("cleB.txt","rb");
	if(f == NULL)
		error("Erreur ouverture du fichier de cle\n");
	fread(k,sizeof(unsigned char),256,f);
	fclose(f);
    
	 aesDecrypt("encAB_2.txt","AB_2.txt", k);
	 ifstream in("AB_2.txt");
	 string contents((istreambuf_iterator<char>(in)), 
     istreambuf_iterator<char>());
	 cout<<"Here is AB from Bob:"<<contents.c_str()<<endl;
     
     char AB[256];
     strcpy(AB, contents.c_str()); 
     //calcul de H1 le haché de AB
     char *H1;
	 H1 = sha1 (AB);
	 
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
     
     Point R = (ce.add(ce.mult (u1, ce.getGen()), ce.mult (u2, Pa)));
     
     mpz_t v;
     mpz_init (v);
     mpz_mod (v, R.getX().get_mpz_t(), ce.getN().get_mpz_t());
     
     //VERIFICATION
     cout<<"\n v is :"<<v<<endl;
	 cout<<"\n r1 is :"<<r1<<endl;
     if ((mpz_cmpabs_ui (r1,1)>0) && (mpz_cmpabs(r1,ce.getN().get_mpz_t())<0)
		&& (mpz_cmpabs_ui (s1,1)>0) && (mpz_cmpabs(s1,ce.getN().get_mpz_t())<0)
		&& (mpz_cmpabs(v, r1)==0))
		{
				
				n = write(newsockfd,"I got your real message",23);
				if (n < 0) error("ERROR writing to socket");
				sleep(1);
				
		}
		else
		{
			 n = write(newsockfd,"I didn't got your real message", 30);
			 if (n < 0) error("ERROR writing to socket");
			 sleep(1);
		}
		
	 cout<<"\nAffichage de la clé secrète S:"<<endl;
     cout <<"S = ("<<S.getX()<<", "<<S.getY()<<")"<<endl;
	
     close(newsockfd);
     close(sockfd);
     return 0; 
}
