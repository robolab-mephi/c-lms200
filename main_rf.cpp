#include "cpp_rf.h" 
#include<iostream>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <pair>
#include <string>
WORD buff[361];


/* application data */
const char my_name[]="RF1";
#define SERVERPORT "5678"

enum answer_type {IAM,
	DEVICEOF,
	GIVEME,
	PLEASE_START};
	
enum question_type {WHOIS,
	IAM_NODE,
	TAKE_DEVICES,
	TAKE_DATA,
	TAKE_RESULT,
	PLEASE_START_YOUR};

char* make_message(answer_type tp, const char* extras)
{
	switch (tp)
	{
		case IAM: return strcat("IAM PROGRAM ",my_name);
		case DEVICEOF: return strcat(strcat("DEVICESOF ",my_name),"RANGEFINDER2D");
		default: return "(foo)";
	}
}


DWORD WINAPI Listener(LPVOID param)
{
	int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(5678);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
   len = sizeof(cliaddr);
   for (;;)
   {
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      //sendto(sockfd,mesg,n,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
      mesg[n] = 0;
      strtok(mesg," ");
	  char ms[1000];
	  
	  /*Парсинг сообщения*/
	  //WHOIS
	  if (!strcmp(mesg,"WHOIS") && !strcmp(mesg+strlen("WHOIS")+1,"PROGRAM"))
	  {
		strcpy(ms,make_message(DEVICEOF,0));
		sendto(sockfd,ms,strlen(ms),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
		continue;
	  }
	  //IAM_NODE
	  if (!strcmp(mesg,"IAM") && !strcmp(mesg+strlen("IAM")+1,"NODE"))
	  {/*no action */
		continue;
	  }
	  //TAKE_DEVICES,
	  if (!strcmp(mesg,"TAKE") && !strcmp(mesg+strlen("TAKE")+1,"DEVICES"))
	  {/*no action*/
		continue;
	  }
	  //TAKE_DATA
	  if (!strcmp(mesg,"TAKE") && !strcmp(mesg+strlen("TAKE")+1,"DATA"))
	  {
		continue;
	  }
	  //TAKE_RESULT,
	  if (!strcmp(mesg,"TAKE") && !strcmp(mesg+strlen("TAKE")+1,"RESULT"))
	  {
		continue;
	  }
	  //PLEASE_START_YOUR
	  if (!strcmp(mesg,"PLEASE-START") && !strcmp(mesg+strlen("PLEASE-START")+1,"YOUR"))
	  {
		continue;
	  }
   }
    
}

DWORD WINAPI Timer(LPVOID param)
{
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo("192.168.208.255", SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
	char ms[200];
	strcpy(ms,make_message(IAM,0));
    for (;;)
	{
		if ((numbytes = sendto(sockfd, ms, strlen(ms), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		Sleep(1000);
	}
}

DWORD WINAPI Timer2(LPVOID param)
{
	LMS200 *plms=(LMS200*)param;
	int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    if ((rv = getaddrinfo("192.168.208.113", "1456", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }
	strcpy(ms,make_message(IAM,0));
    for (;;)
	{
		plms->full_scan(buff);
		if ((numbytes = sendto(sockfd, buff, 361*sizeof(WORD), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
		}
		Sleep(500);
	}
}

int main()
{
	LMS200 lms;
	/*создаем соединение*/
//	lms.create_connection();
	/*сетевой поток-слушатель*/
	HANDLE m_listener = CreateThread(NULL,0,&Listener,(void*)0,0,NULL);
	HANDLE m_timer = CreateThread(NULL,0,&Timer,(void*)0,0,NULL);
	HANDLE m_timer2 = CreateThread(NULL,0,&Timer2,(void*)lms,0,NULL);
/*	lms.full_scan(buff);
	for(int i=0;i<361;i++)
		std::cout<<buff[i]<<" ";
	lms.close_connection();
*/	for (;;);
	return 0;
}
