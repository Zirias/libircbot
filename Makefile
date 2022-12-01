BOOLCONFVARS=	WITH_TLS
SINGLECONFVARS=	OPENSSLINC OPENSSLLIB
include zimk/zimk.mk

INCLUDES += -I.$(PSEP)include
$(call zinc, src/lib/ircbot/ircbot.mk)
