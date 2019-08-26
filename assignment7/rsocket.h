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

int dropMessage(float p);
int difference_in_time(struct tm t1,struct tm t2);
void HandleRetransmit(int sockfd);
void HandleAppMsgRecv(int sockfd,char* buf,struct sockaddr_in cli_addr,int len);
void HandleACKMsgRecv(char* buf);
void HandleReceive(int sockfd);
void* thread_X(void* vargp);
int r_socket(int domain,int type,int protocol);
int r_bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen);
ssize_t r_sendto(int sockfd,char *buf,size_t len,int flags,const struct sockaddr *dest_addr,socklen_t addrlen);
ssize_t r_recvfrom(int sockfd,void *buf,size_t len,int flags,struct sockaddr* src_addr,socklen_t *addrlen);
void r_close(int file_descriptor);
