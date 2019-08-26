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


#define port_X 50000
//#define port_Y 40000
#define cd_id 0
#define get_id 1
#define put_id 2
#define quit_id 3
#define port_id 4

#define Successfull 1

#define MAX_MSG_SIZE 80
#define Row 10
#define Column 10

int port_Y=55000;

void communicate(int sock_CC);
int set_connection(struct sockaddr_in *serv_addr);
void print_message(int code);
void parse_command(char *buf,char **args);
int get_command_id(char *arg);
void call_function(int newsock_CD,int command_id , char **args);
void command_cd(int newsock_CD,char **args);
void command_get(int newsock_CD,char **args);
void command_put(int newsock_CD,char **args);
void command_quit(int newsock_CD,char **args);
void command_port(int newsock_CD,char **args);
void check_for_exit(int code,int pid,int flag);
char** create_2d_array(int num_of_row,int num_of_column);
int get_port(char *args);

int main(){
	struct sockaddr_in serv_addr;
	memset(&serv_addr,0,sizeof(serv_addr));

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(port_X);

	int sock_CC;
	sock_CC=set_connection(&serv_addr);

	communicate(sock_CC);

}
char** create_2d_array(int num_of_row,int num_of_column){
	char **args;
	args=(char**)malloc(num_of_row*sizeof(char*));
	int i;
	for(i=0;i<num_of_row;i++){
		args[i]=(char*)malloc(num_of_column*sizeof(char));
	}
	return args;
}

int set_connection(struct sockaddr_in *serv_addr){
	int count;
	count=1;
	int sock_CC=0;
	while(count<=3){
		sock_CC=socket(AF_INET,SOCK_STREAM,0);
		if(sock_CC>0){
			break;
		}
		count++;
	}
	if(sock_CC<=0){
		printf("Could not create socket...\n");
		exit(0);
	}
	int opt=1;
	if (setsockopt(sock_CC, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,  &opt, sizeof(opt))){
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
	count=1;
	int k;
	while(count<=3){
		k=connect(sock_CC,(struct sockaddr*)serv_addr,sizeof(*serv_addr));
		if(k>=0){
			break;
		}
		count++;
	}
	if(k<0){
		printf("Could not connect to server..\n");
		exit(0);
	}
	return sock_CC;

}

void print_message(int code){
	if(code==200){
		printf("REPLY CODE:%d->Successful\n",code);
	}
	else if(code==503){
		printf("REPLY CODE:%d->Invalid command\n",code);
	}
	else if(code==550){
		printf("REPLY CODE:%d->\n",code);
	}
	else if(code==250){
		printf("REPLY CODE:%d->File Transfer Successfull\n",code);
	}
	else if(code==421){
		//Code for quitting...
	}
	else if(code==501){
		printf("REPLY CODE:%d->Could Not Change directory\n",code);
	}
	else{
		printf("Invalid Reply Code:%d\n",code);
	}
}

void communicate(int sock_CC){
	char buf[MAX_MSG_SIZE];
	char **args;
	args=create_2d_array(Row,Column);
	while(1){
		printf("ftp->");
		scanf("%[^\n]%*c", buf);
		printf("%s\n",buf);
		send(sock_CC,buf,sizeof(buf),0);
		parse_command(buf,args);
		int flag=1;
		if(strcmp(args[0],"port")==0){
			//port_Y==atoi(args[1]);
			port_Y=get_port(args[1]);
			flag=0;
		}

		int pid=fork();
		if(pid<0){
			printf("Could not create Client Data..\n");
			exit(0);
		}
		else if(pid==0){
			struct sockaddr_in cli_addr_data;
			memset(&cli_addr_data,0,sizeof(cli_addr_data));
			cli_addr_data.sin_family=AF_INET;
			cli_addr_data.sin_addr.s_addr=INADDR_ANY;
			cli_addr_data.sin_port=htons(port_Y);
			int sock_CD=socket(AF_INET,SOCK_STREAM,0);
			if(sock_CD<0){
				exit(0);
			}
			int opt=1;
			if (setsockopt(sock_CD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,  &opt, sizeof(opt))){
		        perror("setsockopt"); 
		        exit(EXIT_FAILURE); 
		    } 
			int command_id;
			int newsock_CD;
			command_id=get_command_id(args[0]);
			int len=sizeof(cli_addr_data);
			int k=bind(sock_CD,(struct sockaddr*)&cli_addr_data,len);
			if(k<0){
				exit(0);
			}
			listen(sock_CD,5);
			newsock_CD=accept(sock_CD,(struct sockaddr*)&cli_addr_data,&len);
			if(newsock_CD<0){
				exit(0);
			}
			call_function(newsock_CD,command_id,args);
			close(sock_CD);

		}
		else{
			int code;
			int k=recv(sock_CC,&code,sizeof(code),0);
			printf("%d",code);
			print_message(code);
			check_for_exit(code,pid,flag);
		}
	}
}
int get_port(char *args){
	char buf[5];
	int i=0;
	while(isdigit(*args)!=0){
		buf[i]=*args;
		i++;
		*args++;
	}
	return atoi(buf);
}
void parse_command(char *buf,char **args){
	while(*buf!='\0'){
		*args=buf;
		while(*buf!='\t' && *buf!=' ' && *buf!='\n' && *buf!='\0'){
			*buf++;
		}
		*args++;
		*args=buf;
		while(*buf==' ' || *buf=='\t' || *buf == '\n'){
			*buf='\0';
			*buf++;
		}
		*args=NULL;
	}
}

int get_command_id(char *arg){
	if(strcmp(arg,"port")==0){
		return port_id;
	}
	else if(strcmp(arg,"cd")==0){
		return cd_id;
	}
	else if(strcmp(arg,"get")==0){
		return get_id;
	}
	else if(strcmp(arg,"put")==0){
		return put_id;
	}
	else if(strcmp(arg,"quit")==0){
		return quit_id;
	}
	else{
		return -1;
	}
}

void call_function(int newsock_CD,int command_id , char **args){
	if(command_id==port_id){
		//command_port(newsock_CD,args);
	}
	else if(command_id==cd_id){
		command_cd(newsock_CD,args);
	}
	else if(command_id==get_id){
		command_get(newsock_CD,args);
	}
	else if(command_id==put_id){
		command_put(newsock_CD,args);
	}
	else if(command_id==quit_id){
		command_quit(newsock_CD,args);
	}
	else{
		return;
	}
}

void command_cd(int newsock_CD,char **args){

}

void command_get(int newsock_CD,char **args){
	int fp;
	fp=open(args[1],O_WRONLY | O_CREAT);
	if(fp<0){
		printf("Could not create file..\n");
		close(newsock_CD);
		exit(0);
	}
	char buf[MAX_MSG_SIZE];
	int size_msg_recv;
	do{
		memset(buf,0,sizeof(buf));
		size_msg_recv=recv(newsock_CD,buf,MAX_MSG_SIZE,0);
		write(fp,buf,size_msg_recv);
	}while(size_msg_recv==MAX_MSG_SIZE);
	close(fp);
	close(newsock_CD);
	exit(0);
}

void command_put(int newsock_CD, char **args){
	int fp;
	fp=open(args[1],O_RDONLY);
	if(fp<0){
		printf("Could not read file..\n");
		close(newsock_CD);
		exit(0);
	}
	char buf[MAX_MSG_SIZE];
	int size_msg_send;
	do{
		memset(buf,0,sizeof(buf));
		size_msg_send=read(fp,buf,MAX_MSG_SIZE);
		send(newsock_CD,buf,size_msg_send,0);
	}while(size_msg_send==MAX_MSG_SIZE);
	close(fp);
	close(newsock_CD);
	exit(0);
}

void command_quit(int newsock_CD,char **args){
	close(newsock_CD);
	exit(0);
}
void command_port(int newsock_CD,char **args){
	//port_Y=atoi(args[1]);
}

void check_for_exit(int code,int pid,int flag){
	if(code==200){
		kill(pid,SIGKILL);
	}
	if(code==421){
		kill(pid,SIGKILL);
		exit(0);
	}
	if(code==250){
		kill(pid,SIGKILL);
		exit(0);
	}
	if(code==550 && flag==0){
		kill(pid,SIGKILL);
		exit(0);
	}
}