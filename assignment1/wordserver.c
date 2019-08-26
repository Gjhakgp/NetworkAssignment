// A UDP Server side implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 1024


int main(){
	int sockfd;
	struct sockaddr_in servaddr,cliaddr;

	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd<0){
		perror("socket conection failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr,0,sizeof(servaddr));
	memset(&cliaddr,0,sizeof(cliaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(8181);
	servaddr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sockfd,(const struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		perror("binding failed");
		exit(EXIT_FAILURE);
	}

	printf("\nServer Running....\n");

	int n,i=0;
	socklen_t len;
	char buffer[MAXLINE];
	char *msg;
	FILE *fp;
	char c;

	while(1){
		len=sizeof(cliaddr);
		n=recvfrom(sockfd,(char *)buffer,MAXLINE,0,(struct sockaddr*)&cliaddr,&len);
		buffer[n]='\0';
		printf("CLIENT:Requested file is %s\n",buffer);
		//WRITE CODE FOR OPENING THE FILE
		fp=fopen(buffer,"r");
		if(fp!=NULL){
			msg=(char*)malloc(MAXLINE*sizeof(char));
			do{
				c=getc(fp);
				msg[i++]=c;
			}while(c!=' ' && c!='\n');
			i=0;
			printf("SERVER:%s\n",msg);
			sendto(sockfd,(char*)msg,strlen(msg),0,(const struct sockaddr*)&cliaddr,sizeof(cliaddr));
			do{
				memset(msg,0,sizeof(msg));
				len=sizeof(cliaddr);
				n=recvfrom(sockfd,(char *)buffer,MAXLINE,0,(struct sockaddr*)&cliaddr,&len);
				buffer[n]='\0';
				printf("CLIENT:%s\n",buffer);
				//write code to retrieve word
				do{
					c=getc(fp);
					msg[i++]=c;
				}while(c!=' ' && c!='\n');

				printf("SERVER:%s\n",msg);
				sendto(sockfd,(char*)msg,strlen(msg),0,(const struct sockaddr*)&cliaddr,sizeof(cliaddr));
				i=0;
			}while(strcmp(msg,"END\n")!=0);
			fclose(fp);
			free(msg);
		}
		else{
			msg="File Not Found";
			sendto(sockfd,(char*)msg,strlen(msg),0,(const struct sockaddr*)&cliaddr,sizeof(cliaddr));
		}

	}
	close(sockfd);
	return 0;
}
