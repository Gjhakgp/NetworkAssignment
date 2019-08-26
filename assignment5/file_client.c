#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h> 
#include <time.h>
#include <ctype.h>
void signal_callback_handler(int signum){

        printf("Connection closed-> %d\n",signum);
        exit(0);
}


char count(char *buf, int *word, int n, char prev){
	for(int i=0;i<n;i++){
		char c=buf[i];
		if(isalnum(c)&&!isalnum(prev)){
			*word=*word+1;
		}
		prev=c;
	}
	return prev;
}
void parsing(char *buf, char *ret, int i1, int i2){
	for (int i=i1;i<i2;i++){
		ret[i-i1]=buf[i];
	}
	ret[i2-i1]='\0';
}
main()
{
	int			sockfd ;
	struct sockaddr_in	serv_addr;

	int i;
	char buf[100];
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Unable to create socket\n");
		exit(0);
	}

	serv_addr.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serv_addr.sin_addr);
	serv_addr.sin_port	= htons(20000);
	signal(SIGPIPE,signal_callback_handler);
	if ((connect(sockfd, (struct sockaddr *) &serv_addr,
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
	printf("Enter file name-> ");
	scanf("%s",buf);
	printf("%s\n",buf);
	printf("hit\n");
	int n1=send(sockfd, buf, strlen(buf)+1, 0);
	int byte=0;
	int word=0;
	int x=0;
	int i1=0;
	int fd;
	char prev;
	int size=0;
	int leftsize=0;
	while(1){
		memset(buf,0,sizeof(buf));
		i1=i1+1;
		int n6=0;
		int n5;
		if(i1==1){
			n5=recv(sockfd, buf+n6, 30, MSG_WAITALL);
		}
		if(buf[1]=='0' && i1==1 && buf[0]=='L'){
			printf("File found but empty\n");
			close(sockfd);
			exit(0);
		}
		if(buf[1]!=0 && i1==1 && buf[0]=='L'){
			char *bufnew=malloc(29 * sizeof(char));;
			parsing(buf,bufnew,1,30);
			size=atoi(bufnew);
			printf("%d\n",size);
			leftsize=size;
			prev='-';
		}
		if(buf[0]=='E' && i1==1){
			printf("File not found\n");
			close(sockfd);
			exit(0);
		}
		if(leftsize>100){
			n1=recv(sockfd, buf, 100, MSG_WAITALL);
		}
		else{
			n1=recv(sockfd, buf, leftsize, MSG_WAITALL);	
		}
		int n2=(int)n1;
		prev=count(buf,&word,n2,prev);
		if(n1>0 && i1==1){
			fd = open("foo11", O_WRONLY | O_CREAT | O_TRUNC, 0644);
			x=write(fd, buf, n2);
		}
		else if(n1==0 && i1==1){
			printf("File not found\n");
			exit(0);
		}
	    else if(n1>0 && i1>1){
			x=write(fd, buf, n2);
		}
		else {
			break;
		}
		byte=byte+n2;
		leftsize=size-byte;
	}
	close(fd);
	printf("\nno of byte->%d,%d\n",byte,word);
	strcpy(buf,"Message from client");
	n1=send(sockfd, buf, strlen(buf) + 1, 0);
	n1=send(sockfd, buf, strlen(buf) + 1, 0);
	n1=send(sockfd, buf, strlen(buf) + 1, 0);
	if (n1==-1){
		perror("connection closed->");
	}
	close(sockfd);
}