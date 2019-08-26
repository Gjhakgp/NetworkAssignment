#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>

#define TIMEOUT 2
#define P 0.4
#define SOCK_MRP 573
#define MAX_TABLE_SIZE 100
#define MAX_MSG_SIZE 100

struct TABLE1{
 	int msg_id;
 	char* msg;
 	int port;
 	ssize_t msg_len;
 	struct tm time;
};
typedef struct TABLE1 UNACK_MSG_TABLE;

struct TABLE2{
 	char* msg;
 	ssize_t msg_len;
 	int port;
};
typedef struct TABLE2 RECV_BUF_TABLE;

UNACK_MSG_TABLE *unack_msg_table;
RECV_BUF_TABLE *recv_buf_table;
int* recv_msg_id;
int id=0;
int close_flag=0;
pthread_mutex_t lock1; 
pthread_mutex_t lock2; 
pthread_t tid;


int dropMessage(float p){
	srand(time(NULL));
	float temp=(float)1.0*rand()/RAND_MAX;
	if(temp<p){
		return 1;
	}
	return 0;
}


int difference_in_time(struct tm t1,struct tm t2){
	int sec,hour,min;

	hour=(t2.tm_hour-t1.tm_hour)*3600;
	min=(t2.tm_min-t1.tm_min)*60;
	sec=(t2.tm_sec-t1.tm_sec);
	sec=hour+min+sec;
	return sec;
}

void HandleRetransmit(int sockfd){
	int i;
	struct sockaddr_in	d_addr;

	d_addr.sin_family = AF_INET;  
    d_addr.sin_addr.s_addr = INADDR_ANY; 

	for(i=0;i<MAX_TABLE_SIZE;i++){
		if(unack_msg_table[i].msg_id!=-1){
			time_t t=time(NULL);
 			struct tm tm=*localtime(&t);
 			int diff=difference_in_time(unack_msg_table[i].time,tm);
 			if(diff>TIMEOUT){
 				d_addr.sin_port = htons(unack_msg_table[i].port);

 				int len=unack_msg_table[i].msg_len;
 				char* msg;
 				msg=malloc((len+4)*sizeof(char));
				
				int temp=unack_msg_table[i].msg_id;
				int j;
				msg[0]='0';
				for(j=3;j>=1;j--){
					msg[j]='0'+temp%10;
					temp/=10;
				}
				int x=strlen(unack_msg_table[i].msg);
				for(j=4;j<4+x;j++){
					msg[j]=unack_msg_table[i].msg[j-4];
				}
				if(j==4){
					msg[j]='\0';
				}
				ssize_t send_len;
				send_len=sendto(sockfd,(const char *)msg,len+4,0, (const struct sockaddr *)&d_addr,sizeof(d_addr));

				unack_msg_table[i].time=*localtime(&t);
			}
		}
	}
}

void HandleAppMsgRecv(int sockfd,char* buf,struct sockaddr_in cli_addr,int len){
	int i,temp_index;
	char temp[3];
	temp[0]=buf[1];
	temp[1]=buf[2];
	temp[2]=buf[3];
	sscanf(temp,"%d",&temp_index);
	if(recv_msg_id[temp_index]==0){
		pthread_mutex_lock(&lock1); //////////////////////////////////
		
		recv_msg_id[temp_index]=1;
		recv_buf_table[temp_index].msg_len=len;
		int j;
		for(j=4;j<len;j++){
			recv_buf_table[temp_index].msg[j-4]=buf[j];
		}
		recv_buf_table[temp_index].port=ntohs(cli_addr.sin_port);
	
		pthread_mutex_unlock(&lock1); 
	}
	


	char new_buf[4];
		
	new_buf[0]='1';
		
	for(i=1;i<4;i++){
		new_buf[i]=buf[i];
	}


	int l=sendto(sockfd,(const char *)new_buf,4,0, 
			(const struct sockaddr *)&cli_addr, sizeof(cli_addr));


}

void HandleACKMsgRecv(char* buf){
	int i,n;
	char temp[3];
	temp[0]=buf[1];
	temp[1]=buf[2];
	temp[2]=buf[3];
	int temp_index;
	sscanf(temp,"%d",&temp_index);
	if(unack_msg_table[temp_index].msg_id!=-1){
		pthread_mutex_lock(&lock2); 
		unack_msg_table[temp_index].msg_id=-1;
		pthread_mutex_unlock(&lock2); 
	}
}

void HandleReceive(int sockfd){

	struct sockaddr_in	cli_addr;
	int len = sizeof(cli_addr);
	char buf[100];
	int recv_len=recvfrom(sockfd,buf,100,0,(struct sockaddr *)&cli_addr,&len);
	if(dropMessage(P)==1) {//////////////////////////0.5
		return;
	}
	
	if(buf[0]=='0'){
		HandleAppMsgRecv(sockfd,buf,cli_addr,recv_len);
	}	
	else{
		HandleACKMsgRecv(buf);
	}
}


void* thread_X(void* vargp){

	int* temp=(int*)vargp;
	int sockfd=*temp;
	fd_set rfds;
	struct timeval timeout;

	timeout.tv_sec = TIMEOUT; 
	timeout.tv_usec = 0; 
	
	while(1){

		
		FD_ZERO(&rfds);
		FD_SET(sockfd,&rfds);
		
		int n=select(sockfd+1,&rfds,NULL,NULL,&timeout);

		if(n==0){
			HandleRetransmit(sockfd);
			timeout.tv_sec = TIMEOUT; 
			timeout.tv_usec = 0; 
		}
		else if(FD_ISSET(sockfd,&rfds)){
			HandleReceive(sockfd);
		}
		else{
			printf("ERROR IN X\n");
			exit(0);		
		}
	}
}

int r_socket(int domain,int type,int protocol){
	
	if(type!=SOCK_MRP){
		return -1;
	}


	if((pthread_mutex_init(&lock1,NULL)!=0)&&(pthread_mutex_init(&lock2,NULL)!=0)){
		return -1;
	}

	int sockfd,i;
	sockfd=socket(domain,SOCK_DGRAM,protocol);
	if(sockfd>=0){
		unack_msg_table=(UNACK_MSG_TABLE*)malloc(MAX_TABLE_SIZE*sizeof(UNACK_MSG_TABLE));
		recv_msg_id=(int*)malloc(MAX_TABLE_SIZE*sizeof(int));
		for (i=0;i<MAX_TABLE_SIZE;i++){
			unack_msg_table[i].msg=(char*)malloc(MAX_MSG_SIZE*sizeof(char));
			unack_msg_table[i].msg_id=-1;
			recv_msg_id[i]=0;
		}
		recv_buf_table=(RECV_BUF_TABLE*)malloc(MAX_TABLE_SIZE*sizeof(RECV_BUF_TABLE));
		for(i=0;i<MAX_TABLE_SIZE;i++){
			recv_buf_table[i].msg=(char*)malloc(MAX_MSG_SIZE*sizeof(char));
			recv_buf_table[i].port=-1;
		}
		pthread_create(&tid,NULL,thread_X,(void*)&sockfd);
	}
	close_flag=1;
	return sockfd;
}

int r_bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen){
	int bind_status;
	bind_status=bind(sockfd,addr,addrlen);
	return bind_status;
}

ssize_t r_sendto(int sockfd,char *buf,size_t len,int flags,const struct sockaddr *dest_addr,socklen_t addrlen){
	char* msg;
	char* msg_temp;
	msg=(char*)malloc((len+4)*sizeof(char));
	msg_temp=(char*)malloc(len*sizeof(char));
	int temp=id;
	int i;
	ssize_t msg_length;
	msg[0]='0';
	for(i=3;i>=1;i--){
		msg[i]='0'+temp%10;
		temp/=10;
	}
	i=4;
	while(*buf!=NULL){
		msg[i]=*buf;
		msg_temp[i-4]=*buf;
		*buf++;
		i++;
	}
	if(i==4){
		msg[i]='\0';
	}
	msg_length=sendto(sockfd,(const char *)msg,len+4,flags,(const struct sockaddr *)dest_addr,addrlen);
	time_t t=time(NULL);
	struct sockaddr_in *d_addr=(struct sockaddr_in *) dest_addr;
	pthread_mutex_lock(&lock1);
	unack_msg_table[id].msg_id=id;
	strcpy(unack_msg_table[id].msg,msg_temp);
	unack_msg_table[id].port=ntohs(d_addr->sin_port);
	unack_msg_table[id].msg_len=len;
	unack_msg_table[id].time=*localtime(&t);
	pthread_mutex_unlock(&lock1);
	id++; 
	return msg_length;
}
ssize_t r_recvfrom(int sockfd,void *buf,size_t len,int flags,struct sockaddr* src_addr,socklen_t *addrlen){
	char *msg=(char*)buf;
	int i,j;
	ssize_t msg_length;
	while(1){
		for (i=0;i<100;i++){
			if(recv_buf_table[i].port>=0){
				for(j=0;j<len;j++){
					msg[j]=recv_buf_table[i].msg[j];
				}
				pthread_mutex_lock(&lock2);
				recv_buf_table[i].port=-1;
				pthread_mutex_unlock(&lock2);
				msg_length=recv_buf_table[i].msg_len;
				return msg_length;
			}
		}
		sleep(1);
	}
}
void r_close(int file_descriptor){
	int i;
	int flag=0;
	if(close_flag==1){
		for(i=0;i<100;i++){
			if(flag==1){
				i=i-1;
				flag=0;
			}
			if(unack_msg_table[i].msg_id!=-1){
				sleep(1);
				i--;
				flag=1;
			}
		}
		for(i=0;i<100;i++){
			free(unack_msg_table[i].msg);
			free(recv_buf_table[i].msg);
		}
		pthread_mutex_destroy(&lock1);
		pthread_mutex_destroy(&lock2); 
		pthread_cancel(tid);
		close(file_descriptor);
		close_flag=0;
	}
}

