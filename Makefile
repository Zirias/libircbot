BOOLCONFVARS= WITH_TLS
include zimk/zimk.mk

INCLUDES += -I.$(PSEP)include
$(call zinc, src/lib/ircbot/ircbot.mk)
