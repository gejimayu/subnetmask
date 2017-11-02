#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERVER "hmif.cf"
#define PORT 9999

int getMask(char* address) {
   int i = 0;
   while (address[i] != '/') 
      i++;
   i++;
   char a = address[i];
   i++;
   if (address[i] == '\0')
      return a - '0';
   else {
      int x = a - '0';
      int y = address[i] - '0';
      return x*10 + y;
   }
}

void recv_until(int sock, char *buf, char *data) {
   bzero((char *) buf, 512);
   int i = 0;
   while (strstr(buf, data) == NULL) {
	   read(sock, buf+i, 1);
	   i++;
   }
}

void getValidSubnet(char *subnet, char *host) {
   bzero((char *) subnet, 512);
   char *mask = "255.255.255.0"; //prefix length = 24
   struct sockaddr_in hostaddr, maskaddr, subaddr;

   inet_pton(AF_INET, host, &(hostaddr.sin_addr)); 
   inet_pton(AF_INET, mask, &(maskaddr.sin_addr));
   subaddr.sin_addr.s_addr = (hostaddr.sin_addr.s_addr & maskaddr.sin_addr.s_addr);

   inet_ntop(AF_INET, &(subaddr.sin_addr), subnet, INET_ADDRSTRLEN);
   strcat(subnet, "/24\n");
}

void countHosts(char *subnet, char *host) {
   bzero((char *) host, 512);
   int numMask = 32 - getMask(subnet);
   long long count = 1;
   int k = 0;
   while (k < numMask) {
      count *= 2;
      k++;
   }
   sprintf(host, "%lld\n", count);
}

int isSubnetValid(char *subnet, char *host) {
    int prefix = getMask(subnet);
    unsigned long mask;

    if (prefix == 0)
      mask = 0;
    else
      mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;

    char strMask[512];
    bzero((char *) strMask, 512);

    sprintf(strMask, "%lu.%lu.%lu.%lu", mask >> 24, (mask >> 16) & 0xFF, (mask >> 8) & 0xFF, mask & 0xFF);

    char tempSubnet[512];
    bzero((char *) tempSubnet, 512);

    int i = 0;
    while (subnet[i] != '/') {
      tempSubnet[i] = subnet[i];
      i++;
    }

    struct sockaddr_in hostaddr, maskaddr, subaddr;

    inet_pton(AF_INET, host, &(hostaddr.sin_addr)); 
    inet_pton(AF_INET, strMask, &(maskaddr.sin_addr));
    inet_pton(AF_INET, tempSubnet, &(subaddr.sin_addr)); 

    hostaddr.sin_addr.s_addr = (hostaddr.sin_addr.s_addr & maskaddr.sin_addr.s_addr);
    subaddr.sin_addr.s_addr = (subaddr.sin_addr.s_addr & maskaddr.sin_addr.s_addr);

    char sub2[512], host2[512];
    bzero((char *) sub2, 512);
    bzero((char *) host2, 512);

    inet_ntop(AF_INET, &(subaddr.sin_addr), sub2, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(hostaddr.sin_addr), host2, INET_ADDRSTRLEN);

    if (strcmp(sub2, host2) == 0)
      return 1;
    else
      return 0;
}

int main(int argc, char *argv[]) {
   int sockfd, portno, n, i;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[512];
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(SERVER);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(PORT);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   recv_until(sockfd, buffer, "NIM: ");
   printf("NIM: ");
   scanf("%s", buffer);
   write(sockfd, buffer, strlen(buffer));

   recv_until(sockfd, buffer, "NIM: ");
   printf("Verify NIM: ");
   scanf("%s", buffer);
   write(sockfd, buffer, strlen(buffer));

   recv_until(sockfd, buffer, "\n");
   printf("%s", buffer);

   char host[512];
   char subnet[512];
   
   // Phase 1
   for (i = 0; i < 100; i++) {
      recv_until(sockfd, buffer, "Host: ");
      recv_until(sockfd, host, "\n");
      host[strlen(host)-1] = '\0';
      recv_until(sockfd, buffer, "Subnet: ");
      getValidSubnet(subnet, host);
      write(sockfd, subnet, strlen(subnet));
   }
   recv_until(sockfd, buffer, "\n");
   printf("%s", buffer);
   
   // Phase 2
   for (i = 0; i < 100; i++) {
      recv_until(sockfd, buffer, "Subnet: ");
      recv_until(sockfd, subnet, "\n");
      subnet[strlen(subnet)-1] = '\0';
      recv_until(sockfd, buffer, "Number of Hosts: ");
      countHosts(subnet, host);
      write(sockfd, host, strlen(host));
   }
   recv_until(sockfd, buffer, "\n");
   printf("%s", buffer);
   
   // Phase 3
   for (i = 0; i < 100; i++) {
      recv_until(sockfd, buffer, "Subnet: ");
      recv_until(sockfd, subnet, "\n");
      subnet[strlen(subnet)-1] = '\0';
      recv_until(sockfd, buffer, "Host: ");
      recv_until(sockfd, host, "\n");
      host[strlen(host)-1] = '\0';
      if (isSubnetValid(subnet, host) == 1) {
         write(sockfd, "T\n", 2);
	   } else {
         write(sockfd, "F\n", 2);
	   }
   }
   recv_until(sockfd, buffer, "\n");
   printf("%s", buffer);
   
   close(sockfd);
   return 0;
}
