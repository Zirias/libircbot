ircbot_MODULES:= util service event log threadpool
ircbot_LDFLAGS:= -pthread
ircbot_HEADERS_INSTALL:= config decl
ircbot_HEADERDIR:= include$(PSEP)ircbot
ircbot_V_MAJ:= 0
ircbot_V_MIN:= 1
ircbot_V_REV:= 0
$(call librules, ircbot)
