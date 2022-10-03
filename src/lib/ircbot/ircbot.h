#ifndef IRCBOT_INT_IRCBOT_H
#define IRCBOT_INT_IRCBOT_H

#include <ircbot/ircbot.h>

typedef struct DaemonOpts
{
    void (*started)(void);
    char *pidfile;
    long uid;
    long gid;
    int daemonize;
} DaemonOpts;

struct ThreadOpts
{
    int nThreads;
    int maxThreads;
    int nPerCpu;
    int defNThreads;
    int queueLen;
    int maxQueueLen;
    int minQueueLen;
    int qLenPerThread;
};

#endif
