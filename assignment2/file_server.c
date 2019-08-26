#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define MAXSIZE 100



int main(){
	int sockfd,newsockfd;
	int clilen;
	struct sockaddr_in serv_addr,cli_addr;
	int n;
	char buf[MAXSIZE];
	int fp;
	char msg[MAXSIZE+1];

	if((sockfd= socket(AF_INET,SOCK_STREAM,0))<0){
		perror("ERROR DURING SOCKET CREATION :(");
		exit(0);
	}

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(30000);
	serv_addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		perror("ERROR DURING BINDING");
		exit(0);
	}

	listen(sockfd,5);

	while(1){
		clilen=sizeof(cli_addr);
		newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
		if(newsockfd<0){
			perror("ERROR DURING ACCEPTING CONNECTION ");
			exit(0);
		}
		n=recv(newsockfd,buf,MAXSIZE,0);
		fp=open(buf,O_RDONLY);
		if(fp==-1){
			close(newsockfd);
			printf("Requested file not found ");
			continue;
		}
		else{
			do{
				memset(msg,0,sizeof(msg));
				n=read(fp,msg,MAXSIZE);
				printf("%d\n",n);
				msg[n]='\0';
				send(newsockfd,msg,n, 0);
			}while(n==MAXSIZE);
		}
		close(fp);
		close(newsockfd);
	}
	return 0;
}

