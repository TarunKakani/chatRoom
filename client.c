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
#define MAXDATASIZE 100 // max number of bytes we can recv at once for security

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
    if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("Client: socket_fd");
      continue;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), ip,
              sizeof ip);
    printf("Client: Attempting connection to %s\n", ip);

    if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("Client: connect");
      close(socket_fd);
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), ip,
            sizeof ip);
  printf("client: connected to %s\n", ip);

  freeaddrinfo(servinfo);

  if ((numbytes = recv(socket_fd, buffer, MAXDATASIZE - 1, 0)) == -1) {
    perror("Client: recv");
    exit(1);
  }

  buffer[numbytes] = '\0'; // null terminator at string end

  printf("Client: recieved '%s' \n", buffer);

  close(socket_fd);

  return 0;
}
