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
#define EAGAIN  11

void count(char *word,int *Word_count,int m);

int main(){
	int sockfd;
	struct sockaddr_in serv_addr;
	char buf[MAXSIZE];
	char msg[MAXSIZE];
	int fp;
	char prev='-';
	char filename[20];
	char temp[MAXSIZE];

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

	printf("ENTER FILE NAME:");
	scanf("%s",msg);

	send(sockfd,msg,strlen(msg)+1,0);

	int n;
	int word=0,byte=0;

	memset(buf,0,sizeof(buf));
	n=recv(sockfd,buf,100,0);
	if(n==0){
		printf("FILE DOESN'T EXIST\n");
		close(sockfd);
		exit(0);
	}
	else{
		byte+=n;
		memset(temp,0,sizeof(temp));
		strcpy(temp,buf);
		temp[strlen(buf)]='\0';
		prev=count(temp,&word,n,prev);
	}

	printf("Enter new file name to be made:");
	scanf("%s",filename);
	fp=open(filename, O_CREAT | O_RDWR);
	if(fp<0){
		perror("ERROR DURING FILE CREATION : ");
		close(fp);
		close(sockfd);
		exit(0);
	}
	while(1){
		write(fp,buf,n);
		printf("%s\n",buf);
		memset(buf,0,sizeof(buf));
		n=recv(sockfd,buf,100,0);
		//WRITE CODE FOR CONNECTION CHECKING
		if(n==-1 && errno==ENOTCONN){
			perror("CONNECTION CLOSED");
			close(sockfd);
			close(fp);
			//exit(0);
			break;
		}
		else if(n==0){
			close(fp);
			break;
		}
		else{
			memset(temp,0,sizeof(temp));
			strcpy(temp,buf);
			temp[strlen(buf)]='\0';
		   	prev=count(temp,&word,n,prev);
			byte+=n;
		}

	}
	printf("WORD COUNT=%d , Bytes Count=%d\n",word,byte);

}

char count(char *buf,int *word,int n, char prev){
    for(int i=0;i<n;i++){
        char c=buf[i];
        if(isalnum(c)&&!isalnum(prev)){
            *word=*word+1;
        }
        prev=c;
    }
return prev;
}
