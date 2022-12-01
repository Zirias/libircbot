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
ircbot_V_REV:=			1

ifeq ($(WITH_TLS),1)
  ifneq ($(OPENSSLINC)$(OPENSSLLIB),)
    ifeq ($(OPENSSLINC),)
$(error OPENSSLLIB specified without OPENSSLINC)
    endif
    ifeq ($(OPENSSLLIB),)
$(error OPENSSLINC specified without OPENSSLLIB)
    endif
ircbot_INCLUDES+=		-I$(OPENSSLINC)
ircbot_LDFLAGS+=		-L$(OPENSSLLIB)
ircbot_LIBS+=			ssl
  else
ircbot_PKGDEPS+=		libssl
  endif
ircbot_DEFINES+=		-DWITH_TLS
endif

$(call librules, ircbot)
