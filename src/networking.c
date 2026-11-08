#include "libs.h"

int socks_listen(char* host, int port, int backlog, int verbose) {
    struct addrinfo hints = {0};
    struct addrinfo *results = NULL;
    char service[6];

    snprintf(service, sizeof service, "%d", port);
    hints.ai_family = AF_UNSPEC;       // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags = AI_PASSIVE;       // useful when host is NULL (all interfaces)

    // need to test all candidates.
    int error = getaddrinfo(host, service, &hints, &results);
    if (error != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return EXIT_FAILURE;
    }

    if (results->ai_family != AF_INET6 && results->ai_family != AF_INET) {
        fprintf(stderr, "Use either IPv4 or IPv6\n");
        return EXIT_FAILURE;
    } else if (results->ai_family == AF_INET) {
        printf("Warning: IPv4 detected, it is recommanded to use IPv6\n");
    }
    
    int sock_fd = socket(results->ai_family, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        printf(sprintf(NULL, "Error: failed to initialize socket with code %d\n", sock_fd));
        exit(EXIT_FAILURE);
    }

    if (verbose) printf("Success: socket initialized\n");

    if (bind(sock_fd, results->ai_addr, results->ai_addrlen) == -1) {
        printf("Error: failed to bind socket with code \n");
        perror("bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(results);

    if (verbose) printf("Success: socket binded\n");

    if (listen(sock_fd, backlog) == -1) {
        printf("Error: failed to listen socket with code \n");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening...\n");

    return sock_fd;
}

void handle_client(int sock_fd, int timeout, char* _user, char* _passwd) {
    int client_sock_fd = accept(sock_fd, NULL, NULL);
    if (client_sock_fd == -1) {
        fprintf(stderr, "error: couldn't accept client\n");
        perror("accept");
        return;
    }
    printf("Connection accepted\n");

    int logging = 0;
    int prelogging = 1;
    struct timeval tv;
    tv.tv_sec  = timeout;
    tv.tv_usec = 0;
    setsockopt(client_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(client_sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    ssize_t count;
    unsigned char unsupported[2]  = {0x05, 0xFF};
    unsigned char accepted[3]     = {0x05, 0x01, 0x02};
    unsigned char accepted_two[4] = {0x05, 0x02, 0x00, 0x02};
    unsigned char _continue[2]    = {0x05, 0x02};
    unsigned char _userfail[2]    = {0x01, 0x01};
    unsigned char _success[2]     = {0x01, 0x00};

    for (;;) {
        unsigned char buffer[1024];
        count = read(client_sock_fd, buffer, sizeof buffer);
        
        if (count == 0) {
            close(client_sock_fd);
            break;
        } else if (count < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("timeout\n");
                close(client_sock_fd);
                break;
            } else {
                perror("read");
                close(client_sock_fd);
                break;
            }
        } else {
            // handle data
            if (prelogging) {
                if (count == 3) {
                    if (memcmp(buffer, accepted, 3) != 0) {
                        printf("invalid handshake.\n");
                        close(client_sock_fd);
                        break;
                    }
                } else if (count == 4) {
                    if (memcmp(buffer, accepted_two, 4) != 0) {
                        printf("invalid handshake.\n");
                        close(client_sock_fd);
                        break;
                    }
                }
                send(client_sock_fd, _continue, 2, 0);
                printf("Accepted !\n");
                prelogging = 0;
                logging = 1;
            } else if (logging) {
                if (buffer[0] != 0x01) {
                    printf("invalid handshake.\n");
                    close(client_sock_fd);
                    break;
                }
                int userlen = buffer[1];
                if (userlen != strlen(_user)) {
                    send(client_sock_fd, _userfail, 2, 0);
                    close(client_sock_fd);
                    break;
                }
                char username[userlen];
                for (int i = 0; i < userlen; i++) {
                    username[i] = (char) (buffer[2 + i]);
                }
                if (strcmp(username, _user) != 0) {
                    send(client_sock_fd, _userfail, 2, 0);
                    close(client_sock_fd);
                    break;
                }
                int passlen = buffer[2 + userlen];
                if (passlen != strlen(_passwd)) {
                    send(client_sock_fd, _userfail, 2, 0);
                    close(client_sock_fd);
                    break;
                }
                char password[passlen];

                for (int i = 0; i < passlen; i++) {
                    password[i] = (char) (buffer[3 + userlen + i]);
                }
                if (strcmp(password, _passwd) != 0) {
                    send(client_sock_fd, _userfail, 2, 0);
                    close(client_sock_fd);
                    break;
                }

                send(client_sock_fd, _success, 2, 0);

            } else {
                printf(sprintf(NULL, "prelogging: %d\n", prelogging));
                for (int i=0; i<count; i++) {
                    printf(sprintf(NULL, "inputCstr[%d] = %02X\n", i, (unsigned char)buffer[i]));
                }
                close(client_sock_fd);
                break;
            }
            
            continue;
        }
    }
}