# libircbot

This is a library providing a framework for implementing an IRC bot in C.

So far, it is only suitable for pure "fun" bots. It doesn't offer TLS, managing
channel or user modes, authenticating with services and any of that. It *does*
offer reacting on typical bot commands, other PRIVMSGs, channel joins etc.

## Quick start

For a minimal bot to do something, you need:

* an `IrcServer` object, created with `IrcServer_create()` and added to the
  bot with `IrcBot_addServer()`
* If the bot should be on a channel, also call `IrcServer_join()`
* at least one handler added to the bot with `IrcBot_addHandler()`
* running the configured bot with `IrcBot_run()`

The headers in `$(prefix)/include/ircbot` are documented, best start with
`ircbot.h`. You can also generate html reference documentation using `doxygen`.

Pregenerated html docs can currently be found on
https://home.palmen-it.de/~felix/docs/libircbot/

A real usage example can be found on
https://github.com/Zirias/wumsbot

