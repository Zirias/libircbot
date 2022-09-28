#ifndef IRCBOT_CONFIG_H
#define IRCBOT_CONFIG_H

#include <ircbot/decl.h>

typedef struct Config
{
    long uid;
    long gid;
    int daemonize;
    const char *pidfile;

    int nthreads;
    int maxthreads;
    int threads_per_cpu;
    int def_nthreads;
    int queuesize;
    int maxqueuesize;
    int minqueuesize;
    int queuesize_per_thread;

} Config;

#endif
