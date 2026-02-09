/*
 * ping.c - UDP ping/pong client code
 *          author: Aiden Nguyen
 */
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "util.h"

#define PORTNO "1266"


int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  while ((ch = getopt(argc, argv, "h:n:p:s:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr, "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // Create array
  char *array = malloc(arraysize);
  
  if (!array) { 
    perror("malloc"); 
    exit(EXIT_FAILURE); 
  }
  
  memset(array, 200, arraysize);
  
  // Get IP address, port number from getaddrinfo
  struct addrinfo hints, *result;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // ipv4 addresses
  hints.ai_socktype = SOCK_DGRAM; // udp

  if (getaddrinfo(ponghost, pongport, &hints, &result) != 0) {
    perror("getaddrinfo");
    exit(EXIT_FAILURE);
  }

  // Call socket
  int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Start timer
  double total_time = 0.0;
  
  // Ping packet loop
  for (int i = 0; i < nping; i++) {
    double start = get_wctime();
  
  // Send array to server
    if (sendto(sockfd, array, arraysize, 0, result->ai_addr, result->ai_addrlen) < 0) {
        perror("sendto"); 
	exit(EXIT_FAILURE);
    }

  // Receive reply
    char buf[1024];
    int bytes = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (bytes < 0) { perror("recvfrom"); exit(EXIT_FAILURE); }

    double end = get_wctime();

    // Validate result
    int error = 0;
    for (int j = 0; j < arraysize; j++) {
        if ((unsigned char)buf[j] != 201) { // 200 + 1
            error = 1;
            break;
        }
    }
    if (error) errors++;

    // Print
    printf("ping[%d]: round-trip time: %.3f ms\n", i, (end - start) * 1000);
    total_time += (end - start);
    }

  // UDP ping implemenation goes here
  if (errors == 0)
    printf("no errors detected\n");

  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n ",
      nping, arraysize, errors, ponghost, pongport);

  printf("time to send %d packet of %d bytes %.3f ms (%.3f avg par packet)\n",
      nping, arraysize, total_time * 1000, (total_time /nping) *1000);


  free(array);
  free(ponghost);
  free(pongport);
  freeaddrinfo(result);
  close(sockfd);

  return 0;
}
