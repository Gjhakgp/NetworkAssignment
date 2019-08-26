
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
void reverse(char s[])
{
 int i, j;
 char c;
 int n=strlen(s);
 for (i = 0, j = n-1; i<j; i++, j--) {
     c = s[i];
     s[i] = s[j];
     s[j] = c;
 }
}

void itoa(int n, char s[])
{
 int i, sign;

 if ((sign = n) < 0)
     n = -n;
 i = 0;
 do {      
     s[i++] = n % 10 + '0';   
 } while ((n /= 10) > 0);   
 if (sign < 0)
     s[i++] = '-';
 s[i] = '\0';
 reverse(s);
}

main()
{
	int			sockfd, newsockfd ;
	int			clilen;
	struct sockaddr_in	cli_addr, serv_addr;

	int i;
	char buf[100];	
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(20000);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, 5);
	while (1) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;

		if (newsockfd < 0) {
			perror("Accept error\n");
			exit(0);
		}

		int n1=recv(newsockfd, buf, 100, 0);
		buf[n1]='\0';
		printf("%d->%s\n",n1,buf);
		char msg[101];
		memset(msg,0,sizeof(msg));
		int fd = open(buf, O_RDONLY); 
		if(fd>0){
				char s123[1];
				memset(s123,0,sizeof(s123));
				s123[0]='L';
				send(newsockfd,s123,1,0);
				printf("Entered in fd>0\n");
				struct stat st;
				stat(buf, &st);
				long long int size = st.st_size;
				char snum[29];
				memset(snum,0,sizeof(snum));
				itoa(size, snum);
				send(newsockfd, snum, 29, 0);
				printf("%s\n",snum);
				 while (1)
				 {
				   int nx=read(fd, msg, 100);
				   if (nx==0){
				   	break;
				   }
				   msg[nx]='\0';
				   send(newsockfd, msg, nx, 0);
				 }
				 close(newsockfd);
		}
		else {
			char snum[30];
			memset(snum,0,sizeof(snum));
			snum[0]='E';		
			send(newsockfd, snum, 30, 0);
			printf("%d\n", fd);
			close(newsockfd);	
		}
	}
}
			


