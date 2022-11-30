ircbot_MODULES:=		client \
				connection \
				daemon \
				event \
				hashtable \
				ircbot \
				ircchannel \
				irccommand \
				ircmessage \
				ircserver \
				list \
				log \
				queue \
				service \
				stringbuilder \
				threadpool \
				util

ircbot_HEADERS_INSTALL:= 	decl \
				hashtable \
				ircbot \
				ircchannel \
				irccommand \
				ircmessage \
				ircserver \
				list \
				log \
				queue \
				stringbuilder \
				util

ircbot_LDFLAGS:=		-pthread
ircbot_HEADERDIR:=		include$(PSEP)ircbot
ircbot_V_MAJ:=			1
ircbot_V_MIN:=			1
ircbot_V_REV:=			0

ifeq ($(WITH_TLS),1)
ircbot_PKGDEPS:=		libssl
ircbot_DEFINES:=		-DWITH_TLS
endif

$(call librules, ircbot)
