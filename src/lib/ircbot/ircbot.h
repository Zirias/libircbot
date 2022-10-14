#ifndef IRCBOT_INT_IRCBOT_H
#define IRCBOT_INT_IRCBOT_H

#include <ircbot/ircbot.h>

typedef struct DaemonOpts
{
    void (*started)(void);
    const char *pidfile;
    long uid;
    long gid;
    int daemonize;
} DaemonOpts;

struct IBThreadOpts
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
