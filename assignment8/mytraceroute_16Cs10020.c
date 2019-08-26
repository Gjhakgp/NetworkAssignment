#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <sys/select.h>
#include <sys/time.h>

#include <errno.h>


#define MAX 1024
#define LISTEN_PORT 8081
#define SEND_PORT 32164
#define LISTEN_IP "127.0.0.1"
//#define LISTEN_IP "0.0.0.0"

void check_command(char* buf){
	int i,l;
	//printf("%s\n",buf);
	l=strlen(buf);
	for(i=0;i<l;i++){
		if(buf[i]==' ') {
			buf[i]='\0';
			break;
		}		
	}
	if(strcmp(buf,"mytraceroute")==0){
		int j,k=0;
		for(j=i+1;j<l;j++){
			buf[k++]=buf[j];
		}buf[k]='\0';
		//printf("%s\n",buf);
	}
	else{
		perror("invalid command\n");
		exit(__LINE__);
	}
}


char* find_ip(){
	char buf[MAX];
	char* ip=(char*)malloc(100*sizeof(char));
	//printf("Enter Domain Name: ");
	printf("Enter Command here: ");
	scanf("%[^\n]s",buf);
	check_command(buf);
	struct hostent *lh=gethostbyname(buf);
	if(lh){
		struct in_addr addr;
		memcpy(&addr,lh->h_addr_list[0],sizeof(struct in_addr));
		strcpy(ip,inet_ntoa(addr));
		//printf("%s\n",ip);	
	}
	else {
		perror("dns failed");
		exit(__LINE__);
	}
	return ip;
}

int main(){
	int sockfd1,sockfd2;

	struct sockaddr_in saddr1,saddr2;
	struct sockaddr_in daddr1,daddr2;
	socklen_t addrlen=sizeof(struct sockaddr);

	char *domain_ip=find_ip();

	if((sockfd1=socket(AF_INET,SOCK_RAW,IPPROTO_UDP))<0){///fill the appropriate fields later
		perror("error in socket1\n");
		exit(__LINE__);
	}


	daddr1.sin_family = AF_INET;
	daddr1.sin_port = htons(SEND_PORT);
	daddr1.sin_addr.s_addr = inet_addr(domain_ip);
	//daddr1.sin_addr.s_addr = inet_addr("127.0.0.1");

	// saddr1.sin_family = AF_INET;
	// saddr1.sin_port = htons(LISTEN_PORT);
	// saddr1.sin_addr.s_addr = inet_addr(LISTEN_IP);//INADDR_ANY;

	if((sockfd2=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP))<0){///fill the appropriate fields later
		perror("error in scoket2\n");
		exit(__LINE__);
	}


	// if(bind(sockfd2,(const struct sockaddr*)&saddr1,sizeof(struct sockaddr_in))<0){
	// 	perror("error in bind\n");
	// 	exit(__LINE__);
	// }

	int on=1;
	if(setsockopt(sockfd1,IPPROTO_IP,IP_HDRINCL,(char*)&on,sizeof(on))<0){
		perror("error in setsockopt\n");
		exit(__LINE__);
	}

	struct timeval start,stop;
	int end=0,count,ttl_val=1;
	int max_hop=0;
	while(max_hop<30){
		count=0;
		struct iphdr *ip_hdr=(struct iphdr*)malloc(sizeof(struct iphdr));
		struct udphdr *udp_hdr=(struct udphdr*)malloc(sizeof(struct udphdr));

		ip_hdr->ihl=5;
		ip_hdr->version=4;
		ip_hdr->check=0;
		ip_hdr->daddr=daddr1.sin_addr.s_addr;//inet_addr(domain_ip);//inet_addr(LISTEN_IP);
		ip_hdr->frag_off=0;
		ip_hdr->protocol=17;
		//ip_hdr.saddr=0;//inet_addr(LISTEN_IP);
		ip_hdr->tos=0;
		ip_hdr->tot_len=htons(80);
		ip_hdr->ttl=ttl_val;
		

		udp_hdr->check=0;
		udp_hdr->dest=htons(SEND_PORT);
		udp_hdr->len=htons(60);
		udp_hdr->source=htons(LISTEN_PORT);
		

		int i,j=sizeof(struct udphdr)+sizeof(struct iphdr);
		void *buf=(void*)malloc(j+52);

		buf=(struct iphdr*)ip_hdr;
		memcpy((struct iphdr*)(buf+sizeof(struct iphdr)),udp_hdr,sizeof(struct udphdr));
		
		char c[52]; //////////////////////////
		for(i=0;i<52;i++){
			c[i]='a';
		}
		memcpy((char*)(buf+j),c,52);

		// struct udphdr tr=*((struct udphdr*)(buf+sizeof(struct iphdr)));
		// printf("%d\n",tr.len);

		fd_set rfds;
		struct timeval tv;
		
		tv.tv_sec=1;
		tv.tv_usec=0;



		int n=sendto(sockfd1,(const void*)buf,80,0,(struct sockaddr*)&daddr1,sizeof(daddr1));/////len will be 52 or j+52
		//printf("%s\n",strerror(errno));
		gettimeofday(&start,NULL);


		while(count<3){

			FD_ZERO(&rfds);
			FD_SET(sockfd2,&rfds);

			n=select(sockfd2+1,&rfds,NULL,NULL,&tv);

			if(n==0){
				count++;
				if(count==3){
					printf("Hop_Count(%d)\t\t*\t\t*\n",ttl_val);
					ttl_val+=1;
					tv.tv_sec=1;
					tv.tv_usec=0;
				}
			}
			else if(FD_ISSET(sockfd2,&rfds)){
				void* msg=(void*)malloc(1024);
				n=recvfrom(sockfd2,msg,1024,0,(struct sockaddr*)&daddr2,&addrlen);
				gettimeofday(&stop,NULL);

				struct iphdr ip_hdrR=*((struct iphdr*)msg);
				struct icmphdr icmp_hdrR=*((struct icmphdr*)(msg+sizeof(struct iphdr)));
				if(ip_hdrR.protocol==1){
					if(icmp_hdrR.type==3){ // destination unreachable message
						float diff=(float)(stop.tv_sec-start.tv_sec)*1000.0+(float)(stop.tv_usec-start.tv_usec)/1000.0;
						printf("Hop_Count(%d)\t%s\t%f(ms)\n",ttl_val,inet_ntoa(daddr2.sin_addr),diff);	
						end=1;
						break;
					}
					else if(icmp_hdrR.type==11){ // time exceeded message
						float diff=(float)(stop.tv_sec-start.tv_sec)*1000.0+(float)(stop.tv_usec-start.tv_usec)/1000.0;
						printf("Hop_Count(%d)\t%s\t%f(ms)\n",ttl_val,inet_ntoa(daddr2.sin_addr),diff);	
						ttl_val+=1;
						break;
					}
					else{
						continue;
					}
				}
			}
			else{
				perror("error in select\n");
				exit(__LINE__);
			}	
		}

		if(end==1){
			break;
		}
		max_hop++;
	
	}

	
	close(sockfd1);
	close(sockfd2);
}