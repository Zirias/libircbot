test_MODULES:= main
test_LDFLAGS:= -pthread
test_STATICDEPS:= ircbot
test_STATICLIBS:= ircbot
$(call binrules, test)
