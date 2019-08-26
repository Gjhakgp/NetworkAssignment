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
	servaddr.sin_port=htons(20000);

	int n,i;
	socklen_t len;
	char *msg;
	char buffer[MAXLINE];
	FILE *fp;
	char c;
	char filename[100];
	msg=(char*)malloc(MAXLINE*sizeof(char));
	printf("Enter website:");
	scanf("%s",msg);
	sendto(sockfd,(char*)msg,strlen(msg),0,(const struct sockaddr*)&servaddr,sizeof(servaddr));
	while(1){
		len=sizeof(servaddr);
		memset(buffer,0,sizeof(buffer));
		n=recv(sockfd,(char*)buffer,MAXLINE,0);
		if(n==0 || n==-1 || strcmp(buffer,"END")==0){
			close(sockfd);
			exit(0);
			break;
		}
		buffer[n]='\0';
		printf("%s\n",buffer);
	}

}
