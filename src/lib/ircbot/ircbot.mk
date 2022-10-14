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
ircbot_V_MIN:=			0
ircbot_V_REV:=			0

$(call librules, ircbot)
