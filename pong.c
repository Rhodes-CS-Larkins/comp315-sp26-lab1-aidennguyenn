/*
 * pong.c - UDP ping/pong server code
 *          author: Aiden Nguyen
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"


int main(int argc, char **argv) {
  int ch;
  int nping = 1;                    // default packet count
  char *pongport = strdup(PORTNO);  // default port

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  // Resolve local address
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, pongport, &hints, &result) != 0) {
      perror("getaddrinfo");
      exit(EXIT_FAILURE);
  }

  // Get socket
  int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd < 0) { perror("socket"); exit(EXIT_FAILURE); }

  // Bind
  if (bind(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
    perror("bind"); exit(EXIT_FAILURE);
  }

  freeaddrinfo(result);

  // Receive and respond
  for (int i = 0; i < nping; i++) {
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buf[1024];

    int bytes = recvfrom(sockfd, buf, sizeof(buf), 0,
               (struct sockaddr*)&client_addr, &addr_len);
    if (bytes < 0) { perror("recvfrom"); exit(EXIT_FAILURE); }

    // Convert client IP to string
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
    inet_ntop(AF_INET, &s->sin_addr, client_ip, sizeof(client_ip));

    // Print
    printf("pong[%d]: received packet from %s\n", i, client_ip);

    // Increment bytes
    for (int j = 0; j < bytes; j++) {
        buf[j] += 1;
    }

    if (sendto(sockfd, buf, bytes, 0,
        (struct sockaddr*)&client_addr, addr_len) < 0) {
       perror("sendto"); exit(EXIT_FAILURE);
       }
    }

  printf("nping: %d pongport: %s\n", nping, pongport);
  
  close(sockfd);
  free(pongport);

  return 0;
}

