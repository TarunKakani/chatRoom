#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "4444"
#define MAXDATASIZE 100; // max number of bytes we can recv at once for security

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {

  int socket_fd, numbytes;
  int status;
  char buffer[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  char ip[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr, "Usage: Client Hostname\n");
    exit(1);
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
  }

  // loop through all the results and connect to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype,
                            servinfo->ai_protocol)) != 0) {
      perror("Client: socket_fd");
      continue;
    }

    inet_ntop()
  }

  return 0;
}
