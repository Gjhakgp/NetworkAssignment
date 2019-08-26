//A UDP client side implementation
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024


int main(){
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd<0){
		perror("socket connection failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr,0,sizeof(servaddr));

	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=INADDR_ANY;
	servaddr.sin_port=htons(8181);

	int n,i;
	socklen_t len;
	char *msg;
	char buffer[MAXLINE];
	FILE *fp;
	char c;
	char filename[100];

	do{
		msg=(char*)malloc(MAXLINE*sizeof(char));
		printf("Enter file to be retrieved:");
		scanf("%s",msg);
		sendto(sockfd,(const char*)msg,strlen(msg),0,(const struct sockaddr*)&servaddr,sizeof(servaddr));
		len=sizeof(servaddr);
		n=recvfrom(sockfd,(char*)buffer,MAXLINE,0,(struct sockaddr*)&servaddr,&len);
		buffer[n]='\0';
		printf("SERVER:%s\n",buffer);
		free(msg);
	}while(strcmp(buffer,"File Not Found")==0);

	if(strcmp(buffer,"HELLO\n")==0){
		//write code for opening a file
		printf("ENTER NEW FILE NAME:");
		scanf("%s",filename);
		fp=fopen(filename,"a");
		for(i=0;i<n;i++){
				putc(buffer[i],fp);
		}
		i=0;

		do{
			msg=(char*)malloc(MAXLINE*sizeof(char));
			printf("Enter Word no to be retrieved:");
			scanf("%s",msg);
			sendto(sockfd,(char*)msg,strlen(msg),0,(const struct sockaddr*)&servaddr,sizeof(servaddr));
			len=sizeof(servaddr);
			n=recvfrom(sockfd,(char*)buffer,MAXLINE,0,(struct sockaddr*)&servaddr,&len);
			buffer[n]='\0';
			printf("SERVER:%s\n",buffer);
			//write code for appending the word to file
			for(i=0;i<n;i++){
				putc(buffer[i],fp);
			}
			free(msg);
		}while(strcmp(buffer,"END\n")!=0);
		fclose(fp);
	}
}
