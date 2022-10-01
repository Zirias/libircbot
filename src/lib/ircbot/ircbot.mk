ircbot_MODULES:=		client \
				connection \
				event \
				hashtable \
				ircbot \
				ircmessage \
				ircserver \
				list \
				log \
				queue \
				service \
				threadpool \
				util

ircbot_HEADERS_INSTALL:= 	config \
				decl \
				event \
				ircbot \
				ircserver \
				log \
				service

ircbot_LDFLAGS:=		-pthread
ircbot_HEADERDIR:=		include$(PSEP)ircbot
ircbot_V_MAJ:=			0
ircbot_V_MIN:=			1
ircbot_V_REV:=			0

$(call librules, ircbot)
