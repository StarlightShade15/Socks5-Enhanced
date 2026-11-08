#ifndef NETWORKING_H
#define NETWORKING_H

int socks_listen(char* host, int port, int backlog, int verbose);
void handle_client(int sock_fd, int timeout, char* _user, char* _passwd);

#endif // NETWORKING_H