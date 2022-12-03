# libircbot

This is a library providing a framework for implementing an IRC bot in C.

So far, it is only suitable for pure "fun" bots. It doesn't offer managing
channel or user modes, authenticating with services and any of that. It *does*
offer reacting on typical bot commands, other PRIVMSGs, channel joins etc.

TLS is available by adding `WITH_TLS=1` for building. This requires OpenSSL
(or a compatible library). You can do for example:

    make WITH_TLS=1
    make WITH_TLS=1 install

Options are enabled with `1`, `yes`, `on` or `true` and disabled with `0`,
`no`, `off` or `false`.

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

