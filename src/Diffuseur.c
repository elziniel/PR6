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

struct message{
	char id[9];
	char message[140];
	struct message *prev;
	struct message *next;
};

static char *identifiant,*portGest;
static int portTCP, portMD,nbrMessages=0,compteur=0;
static struct message *liste,*courant, *dernier;
pthread_mutex_t verrou= PTHREAD_MUTEX_INITIALIZER;

void *comm(void *ptr);
void *diffanduser(void *ptr);
void *initialise(void *ptr);
void *gestionnaire(void *ptr);


int main(int argc,char* argv[]){
	identifiant=(char*) malloc(sizeof(char)*9);
	liste=(struct message*) malloc(sizeof(struct message));
	dernier=liste;
	courant=liste;
	sprintf(identifiant,"%s",argv[1]);
	int i = strlen(identifiant);
	while(i!=8){
		identifiant[i++]='#';
	}
	identifiant[strlen(identifiant)]='\0';
	portTCP=atoi(argv[2]);
	portMD=atoi(argv[3]);
	portGest=argv[4];
	pthread_t th1;
	pthread_create(&th1,NULL,gestionnaire,NULL);
	pthread_join(th1,NULL);
	return 0;
}

void *gestionnaire(void *ptr){
	int sockclient=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in *addressin;
	struct addrinfo *first_info;
	struct addrinfo hints;
	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	int r=getaddrinfo("lucien",portGest,&hints,&first_info);
	if(r==0){
		if(first_info!=NULL){
			addressin=(struct sockaddr_in *)first_info->ai_addr;
			int ret=connect(sockclient,(struct sockaddr *)addressin,(socklen_t)sizeof(struct sockaddr_in));
			perror("connect");
			if(ret==0){
				pthread_t th1;
				pthread_create(&th1,NULL,initialise,NULL);
				char buf[1024]="";
				sprintf(buf,"REGI %s lucien %d 225.1.2.4 %d\r\n",identifiant,portTCP,portMD);
				send(sockclient,buf,strlen(buf),0);
				int recu=recv(sockclient,buf,1023,0);
				buf[recu]='\0';
				printf("%s",buf);
				while(1){
					char ok[6];
					recu=recv(sockclient,ok,5,0);
					ok[5]='\0';
					if(!strcmp(ok,"RUOK\r\n")){
						wait(240);
						send(sockclient,"IMOK\r\n",6,0);
					}
				}
				pthread_cancel(th1);
			}else{
				printf("connect\n");
			}
		}
	}else{
		printf("getaddrinfo\n");
	}
	return NULL;
}

void *initialise(void *ptr){
	pthread_t th1;
	pthread_create(&th1,NULL,comm,NULL);
	int sock=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in sockaddress;
	sockaddress.sin_family=AF_INET;
	sockaddress.sin_port=htons(portTCP);
	sockaddress.sin_addr.s_addr=htonl(INADDR_ANY);
	int r = bind(sock,(struct sockaddr*) &sockaddress,sizeof(struct sockaddr_in));
	if(r==0){
		r=listen(sock,0);
		if(r==0){
			struct sockaddr_in caller;
			socklen_t si=sizeof(caller);
			while(1){
				int sock2=accept(sock,(struct sockaddr *)&caller,&si);
				if(sock2>=0){
					pthread_t th;
					pthread_create(&th,NULL,diffanduser,&sock2);
					pthread_join(th,NULL);
					close(sock2);
				}
			}
		}else{
			printf("Listen erreur\n");
		}
	}else{
		printf("Bind erreur\n");
	}
	return NULL;
}

void *comm(void *ptr){
	int sock=socket(PF_INET,SOCK_DGRAM,0);
	sock=socket(PF_INET,SOCK_DGRAM,0);
	int ok=1;
	int r=setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&ok,sizeof(ok));
	struct sockaddr_in address_sock;
	address_sock.sin_family=AF_INET;
	address_sock.sin_port=htons(portMD);
	address_sock.sin_addr.s_addr=htonl(INADDR_ANY);
	r=bind(sock,(struct sockaddr *)&address_sock,sizeof(struct sockaddr_in));
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr=inet_addr("225.1.2.4");
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	r=setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
	char tampon[100];
	struct addrinfo *first_info;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
 	hints.ai_family = AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	char pm[8];
	sprintf(pm,"%d\0",portMD);
	r=getaddrinfo("225.1.2.4",pm,NULL,&first_info);
	if(r==0){
    	if(first_info!=NULL){
     		struct sockaddr *saddr=first_info->ai_addr;
			while(1){
				sleep(10);
				if(nbrMessages!=0 && compteur<9999){
					char val[5];
					if(compteur>=1000){
						sprintf(val,"%d\0",compteur);
					}else if(compteur<1000 && compteur>=100){
						sprintf(val,"0%d\0",compteur);
					}else if(compteur<100 && compteur>=10){
						sprintf(val,"00%d\0",compteur);
					}else{

						sprintf(val,"000%d\0",compteur);

					}					
					char message[12+strlen(courant->id)+strlen(courant->message)];
					sprintf(message,"DIFF %s %s %s\0",val,courant->id,courant->message);
					sendto(sock,message,strlen(message),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
					compteur++;
					
				}
				if(compteur==10000 || compteur==nbrMessages){

					compteur=0;
					courant=liste;				
				}else{
					courant=courant->next;
				}	
			}
		}
	}
	return NULL;
}

void *diffanduser(void *ptr){
	pthread_mutex_lock(&verrou);
	int sockcomm = *((int *)ptr);
	char buf[1024],cmd[5];
	int recu = recv(sockcomm,buf,1023,0);
	buf[recu]='\0';
	int i;
	for(i=0;i<4;i++){
		cmd[i]=buf[i];
	}
	cmd[i++]='\0';

	if(!strcmp("MESS",cmd)){
		struct message *new;
		if(nbrMessages==0){
			new=liste;
		}else{
			new=(struct message*) malloc(sizeof(struct message));
		}		
		new->next=(struct message*) malloc(sizeof(struct message));
		char id[9], mess[141];
		while(buf[i]!='#' && i<13){
			id[i-5]=buf[i++];
		}
		id[i-5]='\0';
		sprintf(new->id,"%s",id);
		while(i<recu-1){
			mess[i-14]=buf[i++];
		}
		mess[i-14]='\0';
		sprintf(new->message,"%s",mess);
		nbrMessages++;
		dernier->next=new;
		new->prev=dernier;
		dernier=new;
		send(sockcomm,"ACKM\r\n",6,0);
	} else if(!strcmp("LAST",cmd)){
		struct message *tmp=courant;
		int nbr=atoi(buf+i)-1,cmpt1=compteur,cmpt=0;
		if(nbr>nbrMessages){
			nbr=nbrMessages-1;
		}
		while(nbr>=0 && cmpt<999){
			printf("%d\n",nbr);
			char val[5];
			int n=nbr-cmpt1;
			if(nbr-cmpt1<0){
				n=nbrMessages-1;
			}
			
			if(n>=1000){
				sprintf(val,"%d\0",n);
			}else if(n<1000 && n>=100){
				sprintf(val,"0%d\0",n);
			}else if(n<100 && n>=10){
				sprintf(val,"00%d\0",n);
			}else{
				sprintf(val,"000%d\0",n);

			}	
			char message[12+strlen(tmp->id)+strlen(tmp->message)];
			sprintf(message,"OLDM %s %s %s\0",val,tmp->id,tmp->message);
			send(sockcomm,message,strlen(message),0);
			nbr--;
			cmpt++;
			if(cmpt1-nbr<=0 && tmp->prev==(tmp->prev)->prev){
				tmp=dernier;				
			}else{
				tmp=tmp->prev;
			}
		}
		send(sockcomm,"ENDM\r\n",6,0);
	}
			
		
	
	pthread_mutex_unlock(&verrou);
	return NULL;
}
