#ifndef IRCBOT_INT_CLIENTOPTS_H
#define IRCBOT_INT_CLIENTOPTS_H

typedef struct ClientOpts
{
    const char *remotehost;
    const char *tls_certfile;
    const char *tls_keyfile;
    int port;
    int numerichosts;
    int tls;
} ClientOpts;

#endif
