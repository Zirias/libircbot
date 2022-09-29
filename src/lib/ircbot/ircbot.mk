ircbot_MODULES:= util service event log threadpool connection client queue \
		 ircserver ircmessage ircbot list
ircbot_LDFLAGS:= -pthread
ircbot_HEADERS_INSTALL:= config decl event ircbot ircserver log service
ircbot_HEADERDIR:= include$(PSEP)ircbot
ircbot_V_MAJ:= 0
ircbot_V_MIN:= 1
ircbot_V_REV:= 0
$(call librules, ircbot)
