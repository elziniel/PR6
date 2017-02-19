#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char* argv[]){
	int sockclient=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in *addressin;
	struct addrinfo *first_info;
	struct addrinfo hints;
	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	int r=getaddrinfo("lucien",argv[1],&hints,&first_info);
	if(r==0){
		if(first_info!=NULL){
			addressin=(struct sockaddr_in *)first_info->ai_addr;
			int ret=connect(sockclient,(struct sockaddr *)addressin,(socklen_t)sizeof(struct sockaddr_in));
			perror("connect");
			if(ret==0){
				char buf[1024]="",cmd[5];
				fgets(buf,1023,stdin);
				send(sockclient,buf,1024,0);
				int i;
				for(i=0;i<4;i++){
					cmd[i]=buf[i];
				}
				cmd[i++]='\0';
				int boucle=1;
				if(!strcmp("LAST",cmd)){
					boucle=atoi(buf+5);
					if(boucle>999){
						boucle=999;
					}
				}
				if(!strcmp("LIST",cmd)){
					int tmp = recv(sockclient,buf,1023,0);
					buf[tmp]='\0';
					printf("%s",buf);
					boucle=atoi(buf+i);
					if(boucle>999){
						boucle=999;
					}
				}
				while(boucle>0){
					int recu = recv(sockclient,buf,1023,0);
					buf[recu]='\0';
					printf("%s",buf);
					if(!strcmp("ENDM\r\n",buf)){
						break;
					}
					boucle--;				
				}
			}else{
				printf("connect\n");
			}
		}
	}else{
		printf("getaddrinfo\n");
	}
}