#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <sys/sendfile.h>
#include <dirent.h>

void dostuff(int); 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void dostuff (int sock)
{
   printf("\n\n Access Granted");
   printf("\nConnected\n");
   int count = 0;
   int buflen;
   while(count == 0){

	int n;
	char buffer[256];

	bzero(buffer,256);
	n = read(sock, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("Cannot read from socket");
	buflen = htonl(buflen);
	n = read(sock,buffer,buflen);
	if (n < 0) error("Cannot read from socket");

	printf("\nSelected choice: %s\n",buffer);
	
	if(buffer != NULL){
	if((strcmp(buffer, "1\n")) == 0){	
		count = 0;
	}
	else if((strcmp(buffer, "2\n")) == 0){	
	
		printf("Sending...");
		char buff[256];
		int n;
		
		char dir[256] = "/home/";
		char hostname[256];
		gethostname(hostname, 255);
		char file[256] = "/Server/";
		strcat(dir, hostname);
		strcat(dir, file);
		printf("\nPath: %s", dir);
			
		char tempo[256];
		printf("\nFiles: ");
		DIR *directory;
		struct dirent *ent;
		if((directory = opendir(dir)) != NULL){
		  while((ent = readdir(directory)) != NULL)
		  {
			strcat(tempo, ent->d_name);
			strcat(tempo, "\n");
		  }
		  closedir(directory);
		}
		else{
		  perror("ERROR");
		  exit(0);
		}

		int datalen = strlen(tempo);
		int tmp = htonl(datalen);
		n = write(sock, (char*)&tmp, sizeof(tmp));
		if(n < 0) error("Cannot write to socket");
		n = write(sock,tempo,datalen);
		if (n < 0) error("Cannot write to socket");

		char fileRev[256];
		bzero(fileRev,256);
		n = read(sock, (char*)&buflen, sizeof(buflen));
		if (n < 0) error("Cannot read from socket");
		buflen = htonl(buflen);
		n = read(sock,fileRev,buflen);
		if (n < 0) error("Cannot read from socket");

		char split[2] = "\n";
		strtok(fileRev, split);
		printf("\nSending file %s to Client... \n", fileRev);
		
		
		if(fileRev != NULL){
			strcat(dir, fileRev);
			FILE *fs = fopen(dir, "rb");
			if(fs == NULL){
			  printf("File not found.\n");
			  perror("fopen");
			  break;
			}
			else{
			  
			  bzero(buff, 256);
			  int fs_block_sz;
			  while((fs_block_sz = fread(buff, sizeof(char), 256, fs)) > 0){
			    if(send(sock, buff, fs_block_sz, 0) < 0){
				fprintf(stderr, "Fail to send file. %d", errno);
				break;
			    }
			    bzero(buff, 256);
			  }
			  printf("\nSent successfully\n");
		 	  fclose(fs);
			}
		}
		else{
			printf("\nDon't leave the filename empty");		
			printf("\nPlease try again");
			exit(0);
		}
		count = 0;
	}
	else if((strcmp(buffer, "3\n")) == 0){ 

		printf("Receiving... ");
		char revBuff[256];
		
		
		char dir[256] = "/home/";
		char hostname[256];
		gethostname(hostname, 255);
		char file[256] = "/Server/";
		strcat(dir, hostname);
		strcat(dir, file);
		printf("\nPath: %s", dir);
		
		
		struct stat st = {0};
		if(stat(dir, &st) == -1){
		  mkdir(dir, 0700);
		}
			
		
		char tempo[256];
		bzero(tempo,256);
		n = read(sock, (char*)&buflen, sizeof(buflen));
		if (n < 0) error("Cannot read from socket");
		buflen = htonl(buflen);
		n = read(sock,tempo,buflen);
		if (n < 0) error("Cannot read from socket");

		char split[2] = "\n";
		strtok(tempo, split);

		printf("\nFile saved: %s\n", tempo);
		
		char filename[256];
		strcpy(filename, tempo);

		
		if(filename != NULL){
			strcat(dir, filename);
			printf("\nFile directory: %s", dir);
			FILE *fr = fopen(dir, "w+");
			if(fr == NULL){
			  printf("Cannot open file");
			  perror("fopen");
			  break;
			}
			else{
			  
			  bzero(revBuff, 256);
			  int fr_block_sz = 0;
			  while((fr_block_sz = recv(sock, revBuff, 256, 0)) > 0){
			  	int write_sz = fwrite(revBuff, sizeof(char), fr_block_sz, fr);
				if(write_sz < fr_block_sz){
				  error("Fail to write on server.\n");
				}
	 			bzero(revBuff, 256);
				if(fr_block_sz == 0 || fr_block_sz != 256){
				  break;			
				}
			  }
			  printf("\nSuccessfully received\n");
			  fclose(fr);
			}
		}		
		else{
			printf("\nDon't leave the filename empty");		
			printf("\nPlease try again");
			break;
		}
		count = 0;
	}
	else if((strcmp(buffer, "4\n")) == 0){	
		count = 0;
	}
	else if((strcmp(buffer, "5\n")) == 0){	
		printf("Client is disconnecting");
		count = 1;
	}
	else{	
		printf("\nWrong input.\n\n");
		count = 0;
		break;
	}
	}

   }
}

int main(int argc, char *argv[]) 
{

     int num;
     int sin_size; 

     int sockfd, newsockfd, portno, pid;

     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"No port available\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("Cannot open socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("Binding Error");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while (1) {
         newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("Access Denied");
         pid = fork();
         if (pid < 0)
             error("Fork Error");
         if (pid == 0)  {
             close(sockfd);
             dostuff(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } 
     close(sockfd);
     return 0; 
}

