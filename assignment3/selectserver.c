#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


#define MESSAGE_SIZE 100

int main(){
	struct sockaddr_in serv_addr,cli_addr;
	int sock_tcp,sock_udp,newsockfd;
	int clilen;
	char buf[MESSAGE_SIZE];
	char *msg;
	fd_set rfds;
	int fp;
	int nfds;
	int i,n;
	int flag=0,len;
	int pid_tcp,pid_udp;

	struct hostent *he;
	struct in_addr **addr_list;

	if((sock_tcp=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("Error in tcp socket creation");
		exit(EXIT_FAILURE);
	}
	if((sock_udp=socket(AF_INET,SOCK_DGRAM,0))<0){
		perror("Error in udp socket creation");
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	memset(&cli_addr,0,sizeof(cli_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(20000);

	if(bind(sock_tcp,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		perror("Error in binding of tcp socket");
		exit(EXIT_FAILURE);
	}

	listen(sock_tcp,5);

	if(bind(sock_udp,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
		perror("Error in binding of udp socket");
		exit(EXIT_FAILURE);
	}

	FD_ZERO(&rfds);
	nfds=sock_tcp > sock_udp ? sock_tcp+1 : sock_udp +1 ;

	while(1){
		FD_SET(sock_tcp,&rfds);
		FD_SET(sock_udp,&rfds);

		if(select(nfds,&rfds,NULL,NULL,NULL)){
			if(FD_ISSET(sock_tcp,&rfds)){
				pid_tcp=fork();
				if(pid_tcp<0){
					perror("Could not fork for tcp");
					exit(EXIT_FAILURE);
				}
				if(pid_tcp==0){
					clilen=sizeof(cli_addr);
					newsockfd=accept(sock_tcp,(struct sockaddr*)&cli_addr,&clilen);
					if(newsockfd<0){
						perror("Connection error");
						exit(EXIT_FAILURE);
					}
					n=recv(newsockfd,buf,MESSAGE_SIZE,0);
					fp=open(buf,O_RDONLY);
					if(fp==-1){
						close(fp);
						printf("Requested file not find\n");
						close(newsockfd);
					}
					else{
						do{
							memset(msg,0,sizeof(msg));
							n=read(fp,msg,MESSAGE_SIZE);
							send(newsockfd,msg,n,0);
						}while(n==MESSAGE_SIZE);
					}
					close(fp);
					close(newsockfd);
					exit(0);
				}
			}

			if(FD_ISSET(sock_udp,&rfds)){
				pid_udp=fork();
				if(pid_udp<0){
					perror("Could not fork udp");
					exit(EXIT_FAILURE);
				}
				if(pid_udp==0){
					clilen=sizeof(cli_addr);
					n=recvfrom(sock_udp,(char*)buf,MESSAGE_SIZE,0,(struct sockaddr*)&cli_addr,&clilen);
					buf[n]='\0';
					printf("client asked for : %s\n",buf);
					if((he=gethostbyname(buf))==NULL){
						herror("error in recovering host name");
						exit(EXIT_FAILURE);
					}
					addr_list=(struct in_addr**)he->h_addr_list;
					for(i=0;addr_list[i]!=NULL;i++){
						msg=inet_ntoa(*addr_list[i]);
						sendto(sock_udp,(char*)msg,strlen(msg),0,(const struct sockaddr*)&cli_addr,sizeof(cli_addr));
					}
					msg="END";
					sendto(sock_udp,(char*)msg,strlen(msg),0,(const struct sockaddr*)&cli_addr,sizeof(cli_addr));
					exit(0);
				}
			}
		}

	}

}
