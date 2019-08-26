#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#define MAXSIZE 100
int main(){
	struct sockaddr_in serv_addr;
	int sockfd;
	char buf[MAXSIZE];

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("ERROR DURING SOCKET CREATION");
		exit(0);
	}

	serv_addr.sin_family=AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(30000);

	if((connect(sockfd,(struct sockaddr*)&serv_addr,
		sizeof(serv_addr)))<0){
		perror("ERROR IN CONNECTING");
		exit(0);
	}

	printf("Enter message:");
	scanf("%s",buf);
	buf[strlen(buf)]='\0';
	send(sockfd,buf,strlen(buf)+1,0);

	char msg[MAXSIZE];
	int n=recv(sockfd,msg,MAXSIZE,0);

	printf("Recieved from server->%s\n",msg);
}