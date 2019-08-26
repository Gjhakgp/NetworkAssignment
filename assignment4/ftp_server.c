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
#include <ctype.h>
#include <stdint.h>


#define port_X 50000
#define cd_id 0
#define get_id 1
#define put_id 2
#define quit_id 3

#define Successfull 1

#define MAX_MSG_SIZE 80
#define Row 10
#define Column 10

#define disconnect 1
#define not_disconnect 0

char** create_2d_array(int num_of_row,int num_of_column);
int set_server(struct sockaddr_in *serv_addr_control);
int accept_socket(int sock_tcp,struct sockaddr_in *cli_addr);
void send_code(int code,int port);
int get_port(int sock);
int check_for_port(char *buf);
int communicate(int port_SC,int port_SD,struct sockaddr_in* cli_addr);
void parse_command(char *buf,char **args);
int get_command_id(char *arg);
int call_function(int sock_SC,int command_id , char **args , int port, struct sockaddr_in* cli_addr);
int command_cd(int sock_SC,char **args,int port, struct sockaddr_in* cli_addr);
int command_get(int sock_SC,char **args,int port, struct sockaddr_in* cli_addr);
int command_put(int sock_SC,char **args,int port, struct sockaddr_in* cli_addr);
int command_quit(int sock_SC,char **args,int port, struct sockaddr_in* cli_addr);




int main(){
	struct sockaddr_in serv_addr_control,cli_addr;
	int server_control_tcp;
	int server_data_tcp;
	int newsock;
	int port_Y;

	memset(&serv_addr_control,0,sizeof(serv_addr_control));
	memset(&cli_addr,0,sizeof(cli_addr));

	server_control_tcp=set_server(&serv_addr_control);

	listen(server_control_tcp,5);
	printf("SERVER running.......\n");
	int k;

	while(1){
		newsock=accept_socket(server_control_tcp,&cli_addr);
		port_Y=get_port(newsock);
		if(port_Y<0){
			close(newsock);
			continue;
		}
		else{
			do{
				k=communicate(newsock,port_Y,&cli_addr);
			}while(k==not_disconnect);
		}
		close(newsock);
	}
	close(server_control_tcp);
	return 0;
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


int set_server(struct sockaddr_in *serv_addr_control){
	int count=1;
	int sock;
	while(count<=3){
		sock=socket(AF_INET,SOCK_STREAM,0);
		if(sock>=0){
			break;
		}
		count++;
	}
	if(sock<0){
		printf("Socket Creation error\n");
		exit(0);
	}
	int opt=1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,  &opt, sizeof(opt))){
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

	(*serv_addr_control).sin_family=AF_INET;
	(*serv_addr_control).sin_addr.s_addr=INADDR_ANY;
	(*serv_addr_control).sin_port=htons(port_X);
	count=1;
	int k;
	while(count<=3){
		k=bind(sock,(struct sockaddr*)serv_addr_control,sizeof(*serv_addr_control));
		if(k>=0){
			break;
		}
		count++;
	}
	if(k<0){
		printf("Binding error...");
		exit(0);
	}
	return sock;
}

int accept_socket(int sock_tcp,struct sockaddr_in *cli_addr){
	int newsock;
	int len=sizeof(*cli_addr);
	newsock=accept(sock_tcp,(struct sockaddr*)cli_addr,&len);
	if(newsock<0){
		printf("Error in accepting client request\n");
		exit(0);
	}
	return newsock;
}

void send_code(int code,int sock_SC){
	send(sock_SC,&code,sizeof(code),0);
}
int get_port(int sock_SC){
	char buf[MAX_MSG_SIZE];
	int size_msg_rcv;
	size_msg_rcv=recv(sock_SC , buf , MAX_MSG_SIZE , 0);
	printf("%s\n",buf);
	int port_Y=check_for_port(buf);
	printf("port_y=%d\n",port_Y);
	if(port_Y<0){
		send_code(503,sock_SC);
		return -1;
	}
	else if(port_Y<=1024 || port_Y >=65535){
		send_code(550,sock_SC);
		return -1;
	}
	else{
		send_code(200,sock_SC);
		return port_Y;
	}
}

int check_for_port(char *buf){
	char check[]="port";
	char port_num[5];
	int i;
	for(i=0;i<4;i++){
		if(check[i]!=*buf){
			return -1;
		}
		*buf++;
	}
	while(*buf==' '){
		*buf++;
	}
	int len=strlen(buf);
	if(len>5 || len<1){
		return -1;
	}
	for(i=0;i<len;i++){
		if(*buf=='\0'){
			break;
		}
		port_num[i]=*buf;
		*buf++;
	}
	printf("%s\n",port_num);	
	return atoi(port_num);
}

int communicate(int sock_SC,int port_Y,struct sockaddr_in* cli_addr){
	char buf[MAX_MSG_SIZE];
	char **args;
	args=create_2d_array(Row,Column);
	int size_msg_rcv;
	memset(buf,0,sizeof(buf));
	size_msg_rcv=recv(sock_SC,buf,MAX_MSG_SIZE,0);
	buf[size_msg_rcv]='\0';
	parse_command(buf,args);
	int command_id;
	command_id=get_command_id(args[0]);
	return call_function(sock_SC,command_id , args , port_Y, cli_addr);
}

void parse_command(char *buf,char **args){
	while(*buf!='\0'){
		*args=buf;
		while(*buf!='\t' && *buf!=' ' && *buf!='\n' && *buf!='\0'){
			*buf++;
		}
		*args++;
		//*args=buf;
		while(*buf==' ' || *buf=='\t' || *buf == '\n'){
			*buf='\0';
			*buf++;
		}
		//*args=NULL;
	}
	*args=NULL;
}

int get_command_id(char *arg){
	if(strcmp(arg,"cd")==0){
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
int  call_function(int sock_SC,int command_id , char **args , int port_Y, struct sockaddr_in* cli_addr){
	if(command_id==cd_id){
	   return command_cd(sock_SC,args , port_Y, cli_addr);
	}
	else if(command_id==get_id){
		return command_get(sock_SC,args , port_Y , cli_addr);
	}
	else if(command_id==put_id){
		return command_put(sock_SC,args , port_Y , cli_addr);
	}
	else if(command_id==quit_id){
		return command_quit(sock_SC,args , port_Y , cli_addr);
	}
	else{
		return not_disconnect;
	}
}

int command_cd(int sock_SC,char **args,int port_Y, struct sockaddr_in* cli_addr){
	int k=chdir(args[1]);
	if(k<0){
		send_code(501,sock_SC);
	}
	else{
		send_code(200,sock_SC);
	}
	return not_disconnect;
}

int command_get(int sock_SC,char **args,int port_Y, struct sockaddr_in* cli_addr){
	int pid;
	int status;
	int fp;
	fp=open(args[1],O_RDONLY);
	if(fp<0){
		send_code(550,port_X);
		close(fp);
		return not_disconnect;
	}
	else{
		pid=fork();
		if(pid<0){
			printf("Could not create Server Data....\n");
			exit(0);
		}
		else if(pid==0){
			//command_get_server_data(fp,port,cli_addr);
			int sock_SD;
			if((sock_SD=socket(AF_INET,SOCK_STREAM,0))<0){
				perror("ERROR DURING SOCKET CREATION");
				exit(0);
			}
			int opt=1;
			if (setsockopt(sock_SD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,  &opt, sizeof(opt))){
		        perror("setsockopt"); 
		        exit(EXIT_FAILURE); 
		    } 
			struct sockaddr_in cli_addr_data;
			memset(&cli_addr_data,0,sizeof(cli_addr_data));
			cli_addr_data.sin_family=(*cli_addr).sin_family;
			cli_addr_data.sin_addr.s_addr=(*cli_addr).sin_addr.s_addr;
			cli_addr_data.sin_port=htons(port_Y);
			sleep(1);
			if(connect(sock_SD,(struct sockaddr*)&cli_addr_data,sizeof(cli_addr_data))<0){
				printf("Could not connect to Client Data..\n");
				exit(0);
			}
			int size_msg_send;
			char buf[MAX_MSG_SIZE];
			do{
				memset(buf,0,sizeof(buf));
				size_msg_send=read(fp,buf,MAX_MSG_SIZE);
				send(sock_SD,buf,size_msg_send,0);
			}while(size_msg_send==MAX_MSG_SIZE);
			close(sock_SD);
			exit(Successfull);
		}
		else{
			wait(&status);
			close(fp);
			if(status==256*Successfull){
				sleep(1);
				send_code(250,sock_SC);
				return disconnect;
			}
			else{
				sleep(1);
				send_code(550,sock_SC);
				return not_disconnect;
			}
		}
	}
	close(fp);
	return not_disconnect;
}

int command_put(int sock_SC,char **args,int port_Y,struct sockaddr_in* cli_addr){
	int pid;
	int status;
	int fp;
	fp=open(args[1],O_WRONLY | O_CREAT);
	if(fp<0){
		send_code(550,port_X);
		return not_disconnect;
	}
	else{
		pid=fork();
		if(pid<0){
			printf("Could not create Server Data....\n");
			exit(0);
		}
		else if(pid==0){
			//command_put_server_data(fp,port,cli_addr);
			int sock_SD;
			if((sock_SD=socket(AF_INET,SOCK_STREAM,0))<0){
				perror("ERROR DURING SOCKET CREATION");
				exit(0);
			}
			int opt=1;
			if (setsockopt(sock_SD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,  &opt, sizeof(opt))){
		        perror("setsockopt"); 
		        exit(EXIT_FAILURE); 
		    } 
			struct sockaddr_in cli_addr_data;
			memset(&cli_addr_data,0,sizeof(cli_addr_data));
			cli_addr_data.sin_family=(*cli_addr).sin_family;
			cli_addr_data.sin_addr.s_addr=(*cli_addr).sin_addr.s_addr;
			cli_addr_data.sin_port=htons(port_Y);
			sleep(1);
			if(connect(sock_SD,(struct sockaddr*)&cli_addr_data,sizeof(cli_addr_data))<0){
				printf("Could not connect to Client Data..\n");
				exit(0);
			}
			int size_msg_recv;
			char buf[MAX_MSG_SIZE];
			do{
				memset(buf,0,sizeof(buf));
				size_msg_recv=recv(sock_SD,buf,MAX_MSG_SIZE,0);
				write(fp,buf,size_msg_recv);
			}while(size_msg_recv==MAX_MSG_SIZE);
			close(sock_SD);
			exit(Successfull);
		}
		else{
			wait(&status);
			close(fp);
			if(status==256*Successfull){
				sleep(1);
				send_code(250,sock_SC);
				return disconnect;
			}
			else{
				sleep(1);
				send_code(550,sock_SC);
				return not_disconnect;
			}
		}
	}
	// close(fp);
	// return not_disconnect;
}

int command_quit(int sock_SC,char **args, int port_Y , struct sockaddr_in * cli_addr){
	send_code(421,sock_SC);
	return disconnect;
}
