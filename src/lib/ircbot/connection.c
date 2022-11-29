#define _DEFAULT_SOURCE

#include <ircbot/log.h>

#include "connection.h"
#include "event.h"
#include "service.h"
#include "threadpool.h"
#include "util.h"

#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef WITH_TLS
#include <openssl/ssl.h>
#endif

#define CONNBUFSZ 4096
#define NWRITERECS 16
#define CONNTICKS 6
#define RESOLVTICKS 6

static char hostbuf[NI_MAXHOST];
static char servbuf[NI_MAXSERV];

typedef struct WriteRecord
{
    const uint8_t *wrbuf;
    void *id;
    uint16_t wrbuflen;
    uint16_t wrbufpos;
} WriteRecord;

typedef struct RemoteAddrResolveArgs
{
    union {
	struct sockaddr sa;
	struct sockaddr_storage ss;
    };
    socklen_t addrlen;
    int rc;
    char name[NI_MAXHOST];
} RemoteAddrResolveArgs;

typedef struct Connection
{
    Event *connected;
    Event *closed;
    Event *dataReceived;
    Event *dataSent;
    ThreadJob *resolveJob;
#ifdef WITH_TLS
    SSL_CTX *tls_ctx;
    SSL *tls;
#endif
    char *addr;
    char *name;
    void *data;
    void (*deleter)(void *);
    WriteRecord writerecs[NWRITERECS];
    DataReceivedEventArgs args;
    RemoteAddrResolveArgs resolveArgs;
    int fd;
    int connecting;
#ifdef WITH_TLS
    int tls_connect_st;
    int tls_read_st;
    int tls_write_st;
#endif
    uint8_t deleteScheduled;
    uint8_t nrecs;
    uint8_t baserecidx;
    uint8_t rdbuf[CONNBUFSZ];
} Connection;

static void checkPendingConnection(void *receiver, void *sender, void *args);
static void wantreadwrite(Connection *self) CMETHOD;
#ifdef WITH_TLS
static void dohandshake(Connection *self) CMETHOD;
#endif
static void dowrite(Connection *self) CMETHOD;
static void deleteConnection(void *receiver, void *sender, void *args);
static void deleteLater(Connection *self);
static void doread(Connection *self) CMETHOD;
static void readConnection(void *receiver, void *sender, void *args);
static void resolveRemoteAddrFinished(
	void *receiver, void *sender, void *args);
static void resolveRemoteAddrProc(void *arg);
static void writeConnection(void *receiver, void *sender, void *args);

static void checkPendingConnection(void *receiver, void *sender, void *args)
{
    (void)sender;
    (void)args;

    Connection *self = receiver;
    if (self->connecting && !--self->connecting)
    {
	IBLog_fmt(L_INFO, "connection: timeout connecting to %s",
		Connection_remoteAddr(self));
	Service_unregisterWrite(self->fd);
	Connection_close(self);
    }
}

static void wantreadwrite(Connection *self)
{
    if (self->connecting ||
#ifdef WITH_TLS
	    self->tls_connect_st == SSL_ERROR_WANT_WRITE ||
	    self->tls_read_st == SSL_ERROR_WANT_WRITE ||
	    self->tls_write_st == SSL_ERROR_WANT_WRITE ||
#endif
	    self->nrecs)
    {
	Service_registerWrite(self->fd);
    }
    else
    {
	Service_unregisterWrite(self->fd);
    }

    if (
#ifdef WITH_TLS
	    self->tls_connect_st == SSL_ERROR_WANT_READ ||
	    self->tls_read_st == SSL_ERROR_WANT_READ ||
	    self->tls_write_st == SSL_ERROR_WANT_READ ||
#endif
	    !self->args.handling)
    {
	Service_registerRead(self->fd);
    }
    else
    {
	Service_unregisterRead(self->fd);
    }
}

#ifdef WITH_TLS
static void dohandshake(Connection *self)
{
    int rc = SSL_connect(self->tls);
    if (rc > 0)
    {
	self->tls_connect_st = 0;
	IBLog_fmt(L_DEBUG, "connection: connected to %s",
		Connection_remoteAddr(self));
	Event_raise(self->connected, 0, 0);
    }
    else
    {
	rc = SSL_get_error(self->tls, rc);
	if (rc == SSL_ERROR_WANT_READ || rc == SSL_ERROR_WANT_WRITE)
	{
	    self->tls_connect_st = rc;
	}
	else
	{
	    IBLog_fmt(L_ERROR, "connection: TLS handshake failed with %s",
		    Connection_remoteAddr(self));
	    Connection_close(self);
	    return;
	}
    }
    wantreadwrite(self);
}
#endif

static void dowrite(Connection *self)
{
    WriteRecord *rec = self->writerecs + self->baserecidx;
    void *id = 0;
#ifdef WITH_TLS
    if (self->tls)
    {
	size_t writesz = 0;
	int rc = SSL_write_ex(self->tls, rec->wrbuf + rec->wrbufpos,
		rec->wrbuflen - rec->wrbufpos, &writesz);
	if (rc > 0)
	{
	    self->tls_write_st = 0;
	    if (writesz < rec->wrbuflen - rec->wrbufpos)
	    {
		rec->wrbufpos += writesz;
		wantreadwrite(self);
		return;
	    }
	    else id = rec->id;
	    if (++self->baserecidx == NWRITERECS) self->baserecidx = 0;
	    --self->nrecs;
	    if (id)
	    {
		Event_raise(self->dataSent, 0, id);
	    }
	}
	else
	{
	    rc = SSL_get_error(self->tls, rc);
	    if (rc == SSL_ERROR_WANT_READ || rc == SSL_ERROR_WANT_WRITE)
	    {
		self->tls_write_st = rc;
	    }
	    else
	    {
		IBLog_fmt(L_WARNING, "connection: error writing to %s",
			Connection_remoteAddr(self));
		Connection_close(self);
		return;
	    }
	}
	wantreadwrite(self);
    }
    else
    {
#endif
	errno = 0;
	int rc = write(self->fd, rec->wrbuf + rec->wrbufpos,
		rec->wrbuflen - rec->wrbufpos);
	if (rc >= 0)
	{
	    if (rc < rec->wrbuflen - rec->wrbufpos)
	    {
		rec->wrbufpos += rc;
		return;
	    }
	    else id = rec->id;
	    if (++self->baserecidx == NWRITERECS) self->baserecidx = 0;
	    --self->nrecs;
	    wantreadwrite(self);
	    if (id)
	    {
		Event_raise(self->dataSent, 0, id);
	    }
	}
	else if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
	    IBLog_fmt(L_INFO, "connection: not ready for writing to %s",
		    Connection_remoteAddr(self));
	    return;
	}
	else
	{
	    IBLog_fmt(L_WARNING, "connection: error writing to %s",
		    Connection_remoteAddr(self));
	    Connection_close(self);
	}
#ifdef WITH_TLS
    }
#endif
}

static void writeConnection(void *receiver, void *sender, void *args)
{
    (void)sender;
    (void)args;

    Connection *self = receiver;
    if (self->connecting)
    {
	Event_unregister(Service_tick(), self, checkPendingConnection, 0);
	int err = 0;
	socklen_t errlen = sizeof err;
	if (getsockopt(self->fd, SOL_SOCKET, SO_ERROR, &err, &errlen) < 0
		|| err)
	{
	    IBLog_fmt(L_INFO, "connection: failed to connect to %s",
		    Connection_remoteAddr(self));
	    Connection_close(self);
	    return;
	}
	self->connecting = 0;
#ifdef WITH_TLS
	if (self->tls)
	{
	    dohandshake(self);
	    return;
	}
#endif
	wantreadwrite(self);
	IBLog_fmt(L_DEBUG, "connection: connected to %s",
		Connection_remoteAddr(self));
	Event_raise(self->connected, 0, 0);
	return;
    }
    IBLog_fmt(L_DEBUG, "connection: ready to write to %s",
	Connection_remoteAddr(self));
#ifdef WITH_TLS
    if (self->tls_connect_st == SSL_ERROR_WANT_WRITE) dohandshake(self);
    else if (self->tls_read_st == SSL_ERROR_WANT_WRITE) doread(self);
    else if (self->tls_write_st == SSL_ERROR_WANT_WRITE) dowrite(self);
    else
    {
#endif
	if (!self->nrecs)
	{
	    IBLog_fmt(L_ERROR,
		    "connection: ready to send to %s with empty buffer",
		    Connection_remoteAddr(self));
	    wantreadwrite(self);
	    return;
	}
	dowrite(self);
#ifdef WITH_TLS
    }
#endif
}

static void doread(Connection *self)
{
#ifdef WITH_TLS
    if (self->tls)
    {
	size_t readsz = 0;
	int rc = SSL_read_ex(self->tls, self->rdbuf, CONNBUFSZ, &readsz);
	if (rc > 0)
	{
	    self->tls_read_st = 0;
	    self->args.size = readsz;
	    Event_raise(self->dataReceived, 0, &self->args);
	    if (self->args.handling)
	    {
		IBLog_fmt(L_DEBUG, "connection: blocking reads from %s",
			Connection_remoteAddr(self));
	    }
	}
	else
	{
	    rc = SSL_get_error(self->tls, rc);
	    if (rc == SSL_ERROR_WANT_READ || rc == SSL_ERROR_WANT_WRITE)
	    {
		self->tls_read_st = rc;
	    }
	    else
	    {
		IBLog_fmt(L_WARNING, "connection: error reading from %s",
			Connection_remoteAddr(self));
		Connection_close(self);
		return;
	    }
	}
	wantreadwrite(self);
    }
    else
    {
#endif
	errno = 0;
	int rc = read(self->fd, self->rdbuf, CONNBUFSZ);
	if (rc > 0)
	{
	    self->args.size = rc;
	    Event_raise(self->dataReceived, 0, &self->args);
	    if (self->args.handling)
	    {
		IBLog_fmt(L_DEBUG, "connection: blocking reads from %s",
			Connection_remoteAddr(self));
	    }
	    wantreadwrite(self);
	}
	else if (errno == EWOULDBLOCK || errno == EAGAIN)
	{
	    IBLog_fmt(L_INFO, "connection: ignoring spurious read from %s",
		    Connection_remoteAddr(self));
	}
	else
	{
	    if (rc < 0)
	    {
		IBLog_fmt(L_WARNING, "connection: error reading from %s",
			Connection_remoteAddr(self));
	    }
	    Connection_close(self);
	}
#ifdef WITH_TLS
    }
#endif
}

static void readConnection(void *receiver, void *sender, void *args)
{
    (void)sender;
    (void)args;

    Connection *self = receiver;
    IBLog_fmt(L_DEBUG, "connection: ready to read from %s",
	    Connection_remoteAddr(self));

#ifdef WITH_TLS
    if (self->tls_connect_st == SSL_ERROR_WANT_READ) dohandshake(self);
    else if (self->tls_read_st == SSL_ERROR_WANT_READ) doread(self);
    else if (self->tls_write_st == SSL_ERROR_WANT_READ) dowrite(self);
    else
    {
#endif
	if (self->args.handling)
	{
	    IBLog_fmt(L_WARNING,
		    "connection: new data while read buffer from %s "
		    "still handled", Connection_remoteAddr(self));
	    return;
	}
	doread(self);
#ifdef WITH_TLS
    }
#endif
}

static void deleteConnection(void *receiver, void *sender, void *args)
{
    (void)sender;
    (void)args;

    Connection *self = receiver;
    self->deleteScheduled = 2;
    Connection_destroy(self);
}

SOLOCAL Connection *Connection_create(int fd, ConnectionCreateMode mode,
	int tls)
{
#ifndef WITH_TLS
    (void) tls;
#endif

    Connection *self = IB_xmalloc(sizeof *self);
    self->connected = Event_create(self);
    self->closed = Event_create(self);
    self->dataReceived = Event_create(self);
    self->dataSent = Event_create(self);
    self->resolveJob = 0;
    self->fd = fd;
    self->connecting = 0;
    self->addr = 0;
    self->name = 0;
    self->data = 0;
    self->deleter = 0;
#ifdef WITH_TLS
    if (tls)
    {
	self->tls_ctx = SSL_CTX_new(TLS_client_method());
	self->tls = SSL_new(self->tls_ctx);
	SSL_set_fd(self->tls, fd);
    }
    else
    {
	self->tls_ctx = 0;
	self->tls = 0;
    }
    self->tls_connect_st = 0;
    self->tls_read_st = 0;
    self->tls_write_st = 0;
#endif
    self->args.buf = self->rdbuf;
    self->args.handling = 0;
    self->deleteScheduled = 0;
    self->nrecs = 0;
    self->baserecidx = 0;
    Event_register(Service_readyRead(), self, readConnection, fd);
    Event_register(Service_readyWrite(), self, writeConnection, fd);
    if (mode == CCM_CONNECTING)
    {
	self->connecting = CONNTICKS;
	Event_register(Service_tick(), self, checkPendingConnection, 0);
	Service_registerWrite(fd);
    }
    else if (mode == CCM_NORMAL)
    {
	Service_registerRead(fd);
    }
    return self;
}

SOLOCAL Event *Connection_connected(Connection *self)
{
    return self->connected;
}

SOLOCAL Event *Connection_closed(Connection *self)
{
    return self->closed;
}

SOLOCAL Event *Connection_dataReceived(Connection *self)
{
    return self->dataReceived;
}

SOLOCAL Event *Connection_dataSent(Connection *self)
{
    return self->dataSent;
}

SOLOCAL const char *Connection_remoteAddr(const Connection *self)
{
    if (!self->addr) return "<unknown>";
    return self->addr;
}

SOLOCAL const char *Connection_remoteHost(const Connection *self)
{
    return self->name;
}

static void resolveRemoteAddrProc(void *arg)
{
    RemoteAddrResolveArgs *rara = arg;
    char buf[NI_MAXSERV];
    rara->rc = getnameinfo(&rara->sa, rara->addrlen,
	    rara->name, sizeof rara->name, buf, sizeof buf, NI_NUMERICSERV);
}

static void resolveRemoteAddrFinished(void *receiver, void *sender, void *args)
{
    Connection *self = receiver;
    ThreadJob *job = sender;
    RemoteAddrResolveArgs *rara = args;

    if (ThreadJob_hasCompleted(job))
    {
	if (rara->rc >= 0 && strcmp(rara->name, self->addr) != 0)
	{
	    IBLog_fmt(L_DEBUG, "connection: %s is %s", self->addr, rara->name);
	    self->name = IB_copystr(rara->name);
	}
	else
	{
	    IBLog_fmt(L_DEBUG, "connection: error resolving name for %s",
		    self->addr);
	}
    }
    else
    {
	IBLog_fmt(L_DEBUG, "connection: timeout resolving name for %s",
		self->addr);
    }
    self->resolveJob = 0;
}

SOLOCAL void Connection_setRemoteAddr(Connection *self,
	struct sockaddr *addr, socklen_t addrlen, int numericOnly)
{
    free(self->addr);
    free(self->name);
    self->addr = 0;
    self->name = 0;
    if (getnameinfo(addr, addrlen, hostbuf, sizeof hostbuf,
		servbuf, sizeof servbuf, NI_NUMERICHOST|NI_NUMERICSERV) >= 0)
    {
	self->addr = IB_copystr(hostbuf);
	if (!numericOnly && !self->resolveJob && ThreadPool_active())
	{
	    memcpy(&self->resolveArgs.sa, addr, addrlen);
	    self->resolveArgs.addrlen = addrlen;
	    self->resolveJob = ThreadJob_create(resolveRemoteAddrProc,
		    &self->resolveArgs, RESOLVTICKS);
	    Event_register(ThreadJob_finished(self->resolveJob), self,
		    resolveRemoteAddrFinished, 0);
	    ThreadPool_enqueue(self->resolveJob);
	}
    }
}

SOLOCAL void Connection_setRemoteAddrStr(Connection *self, const char *addr)
{
    free(self->addr);
    free(self->name);
    self->addr = IB_copystr(addr);
    self->name = 0;
}

SOLOCAL int Connection_write(Connection *self,
	const uint8_t *buf, uint16_t sz, void *id)
{
    if (self->nrecs == NWRITERECS) return -1;
    WriteRecord *rec = self->writerecs +
	((self->baserecidx + self->nrecs++) % NWRITERECS);
    rec->wrbuflen = sz;
    rec->wrbufpos = 0;
    rec->wrbuf = buf;
    rec->id = id;
    wantreadwrite(self);
    return 0;
}

SOLOCAL void Connection_activate(Connection *self)
{
    if (self->args.handling) return;
    IBLog_fmt(L_DEBUG, "connection: unblocking reads from %s",
	    Connection_remoteAddr(self));
    wantreadwrite(self);
}

SOLOCAL int Connection_confirmDataReceived(Connection *self)
{
    if (!self->args.handling) return -1;
    self->args.handling = 0;
    Connection_activate(self);
    return 0;
}

SOLOCAL void Connection_close(Connection *self)
{
    Event_raise(self->closed, 0, 0);
    deleteLater(self);
}

SOLOCAL void Connection_setData(Connection *self,
	void *data, void (*deleter)(void *))
{
    if (self->deleter) self->deleter(self->data);
    self->data = data;
    self->deleter = deleter;
}

SOLOCAL void *Connection_data(const Connection *self)
{
    return self->data;
}

static void deleteLater(Connection *self)
{
    if (!self) return;
    if (!self->deleteScheduled)
    {
	close(self->fd);
	Event_register(Service_eventsDone(), self, deleteConnection, 0);
	self->deleteScheduled = 1;
    }
}

SOLOCAL void Connection_destroy(Connection *self)
{
    if (!self) return;
    if (self->deleteScheduled == 1) return;

    Service_unregisterRead(self->fd);
    Service_unregisterWrite(self->fd);
    for (; self->nrecs; --self->nrecs)
    {
	WriteRecord *rec = self->writerecs + self->baserecidx;
	if (rec->id)
	{
	    Event_raise(self->dataSent, 0, rec->id);
	}
	if (++self->baserecidx == NWRITERECS) self->baserecidx = 0;
    }
    if (self->deleteScheduled)
    {
	Event_unregister(Service_eventsDone(), self, deleteConnection, 0);
    }
    else
    {
	close(self->fd);
    }
#ifdef WITH_TLS
    SSL_free(self->tls);
    SSL_CTX_free(self->tls_ctx);
#endif
    Event_unregister(Service_tick(), self, checkPendingConnection, 0);
    Event_unregister(Service_readyRead(), self, readConnection, self->fd);
    Event_unregister(Service_readyWrite(), self, writeConnection, self->fd);
    if (self->resolveJob)
    {
	ThreadPool_cancel(self->resolveJob);
	Event_unregister(ThreadJob_finished(self->resolveJob), self,
		resolveRemoteAddrFinished, 0);
    }
    if (self->deleter) self->deleter(self->data);
    free(self->addr);
    free(self->name);
    Event_destroy(self->dataSent);
    Event_destroy(self->dataReceived);
    Event_destroy(self->closed);
    Event_destroy(self->connected);
    free(self);
}

