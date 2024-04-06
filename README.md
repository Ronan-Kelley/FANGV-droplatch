# FANGV-droplatch
server/client code for FANGV's in-progress droplatch game.

# building
code can be built with `make build`; requires standard C build tools. No `make install` target is currently provided due to the early state of the software.

# TODO
 - event queue to push commands from TCP server to main thread in server
 - client needs to check for its connection closing and handle it
 - general testing needs to be done (this is my first time ever doing a TCP server from scratch)
