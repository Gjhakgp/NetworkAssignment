/*
Sometime when multiple process are running signal handler can be tidous,
context witch happens and signal handler become async. If only One process is running I will prefer signal handling
because Non-blocking will check again and again while this will check only if message is recieved.
And multiple process are running I will prefer NON blocking call
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>


#define MAXSIZE 100
int sockfd;

void signal_handler(int signum){
	struct sockaddr_in cli_addr;
	int	clilen=sizeof(cli_addr);
	int newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);
	if(newsockfd<0){
		perror("ERROR DURING ACCEPTING CONNECTION ");
		exit(0);
	}
	char msg[MAXSIZE];
	int n=recv(newsockfd,msg,MAXSIZE,0);
	printf("message from client->%s\n",msg);

	msg[n]='\0';
	send(newsockfd,msg,strlen(msg)+1,0);
	printf("%s\n",msg);
	signal(SIGIO,signal_handler);
}
int main(){
	struct sockaddr_in serv_addr;

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("Could not create socket");
		exit(EXIT_FAILURE);
	}

	serv_addr.sin_family=AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(30000);

	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		perror("ERROR DURING BINDING");
		exit(0);
	}
	fcntl(sockfd,F_SETFL,O_NONBLOCK|O_ASYNC);
	fcntl(sockfd,F_SETOWN,getpid());
	signal(SIGIO,signal_handler);
	listen(sockfd,5);
	while(1){
		
	}

}