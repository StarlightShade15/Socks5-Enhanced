#include "libs.h"

static const char *const usages[] = {
    "socks5-enhance [options]",
    NULL,
};

/*  USE THIS FUNCTION UNTIL I MAKE A CRYPTOGRAPHIC PRNG !

    ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);

    USE THIS FUNCTION UNTIL I MAKE A CRYPTOGRAPHIC PRNG ! */

int main(int argc, const char **argv) {
    signal(SIGPIPE, SIG_IGN);

    char *host = "127.0.0.1";
    char *username = "user";
    char *password = "4uSmFzdYzggJhj90QcfB3YphqYH2I3qs";
    int port = 6969;
    int timeout = 3;
    int backlog = 128;
    int verbose = 0;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Server options"),
        OPT_STRING('H', "host", &host, "address to bind to", NULL, 0, 0),
        OPT_INTEGER('p', "port", &port, "TCP port to listen on", NULL, 0, 0),
        OPT_INTEGER('t', "timeout", &timeout, "Timeout value in second", NULL, 0, 0),
        OPT_STRING('U', "username", &username, "username used to authenticate", NULL, 0, 0),
        OPT_STRING('P', "password", &password, "password used to connect", NULL, 0, 0),
        OPT_INTEGER('b', "backlog", &backlog, "maximum pending connections", NULL, 0, 0),
        OPT_BOOLEAN('v', "verbose", &verbose, "enable verbose output", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;

    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "SOCKS5 proxy server configuration.", NULL);
    argc = argparse_parse(&argparse, argc, argv);

    if (argc != 0) {
        fprintf(stderr, "error: unexpected positional arguments\n");
        return EXIT_FAILURE;
    }
    if (port < 1 || port > 65535) {
        fprintf(stderr, "error: port must be between 1 and 65535\n");
        return EXIT_FAILURE;
    }
    if (backlog < 1) {
        fprintf(stderr, "error: backlog must be greater than 0\n");
        return EXIT_FAILURE;
    }

    /*
    if (!initialize_seed()) {
        fprintf(stderr, "error: PRNG seed is zero\n");
        return EXIT_FAILURE;
    }
    */

    printf("SOCKS5 server configuration:\n");
    printf("  host: %s\n", host);
    printf("  port: %d\n", port);
    printf("  timeout: %d\n", timeout);
    printf("  username: %s\n", username);
    printf("  password: %s\n", password);
    printf("  backlog: %d\n", backlog);
    printf("  verbose: %s\n", verbose ? "yes" : "no");
    
    int sock_fd = socks_listen(host, port, backlog, verbose);

    //i64 random_number = fprng(0, )

    while (1) {
        handle_client(sock_fd, timeout, username, password);
    }
    
    return EXIT_SUCCESS;
}