#ifndef IRCBOT_INT_CONNOPTS_H
#define IRCBOT_INT_CONNOPTS_H

typedef enum ConnectionCreateMode
{
    CCM_NORMAL,
    CCM_WAIT,
    CCM_CONNECTING
} ConnectionCreateMode;

typedef struct ConnOpts
{
    const char *tls_client_certfile;
    const char *tls_client_keyfile;
    ConnectionCreateMode createmode;
    int tls_client;
} ConnOpts;

#endif
