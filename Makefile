include zimk/zimk.mk

INCLUDES += -I.$(PSEP)include
$(call zinc, src/lib/ircbot/ircbot.mk)
$(call zinc, src/bin/test/test.mk)
