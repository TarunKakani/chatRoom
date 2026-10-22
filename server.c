#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10
#define PORT "4444"

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sigchld_handler(int s) {
  (void)s;

  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0) {
    errno = saved_errno;
  }
}

int main() {

  int socket_fd, accept_fd;
  int status;
  int yes = 1;
  socklen_t sin_size;
  struct sigaction sa;
  struct addrinfo hints, *servinfo, *p;

  // ? still not understood
  // this will store connector info after accept?
  struct sockaddr_storage their_addr;

  // inet uses this buffer for filling in the printable ip string
  char ip[INET6_ADDRSTRLEN];

  // gethostname(host, sizeof host);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // meaning use my ip

  if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(servinfo->ai_family, servinfo->ai_socktype,
                            servinfo->ai_protocol)) == -1) {
      perror("Server: socket_fd");
      continue;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("Server: setsockopt");
      exit(1);
    }

    if (bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
      perror("Server: bind");
      continue;
    }

    break;
  }

  // we free the servinfo structure as we dont need it anymore
  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "Server: failed to bind\n");
    exit(1);
  }

  if (listen(socket_fd, BACKLOG) == -1) {
    perror("Server: listen");
    exit(1);
  }

  // sigaction struct?
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("Server: Sigaction");
    exit(1);
  }

  // server waiting for connections
  printf("Server: Waiting for connections on port %s\n", PORT);

  // accept has to be put under a loop so that it can wait for clients and
  // listen for them
  while (1) {
    // ?
    sin_size = sizeof their_addr;

    accept_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &sin_size);

    if (accept_fd == -1) {
      perror("Server: accept");
      exit(1);
    }

    // conversion from network to presentaion
    inet_ntop(their_addr.ss_family, ((struct sockaddr *)&their_addr), ip,
              sizeof ip);

    // this is the child process
    if (!fork()) {
      close(socket_fd); // child doesn't need the listener

      // sending the message
      if (send(accept_fd, "Hello Client", 13, 0) == -1) {
        perror("Server: send");
      }
      close(accept_fd);
      exit(0);
    }
    close(accept_fd);
  }
  return 0;
}
