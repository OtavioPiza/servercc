# SERVERCC Server

This is directory contains multiple servers that each implement a different transport
layer protocol. The basic idea behind each server is the same: receive a request, look for the
first whitespace to determine the method, and then look for the appropriate handler for that method.
The handler is provided with the request and is responsible for managing the connection and sending
the response.

This directory also contains types and configuration files that are shared across the servers.

___

## [Server Config](./src/server_config/include/server_config.h)

This file contains the configuration for the servers. It includes the following header files:

### [Server Defaults](./src/server_config/include/server_defaults.h)

This file contains the default values for the servers. The default values are used if the
configuration file does not specify a value for a particular option.

___

## [Server Types](./src/server_types/include/server_types.h)

This file contains the types used by the servers. It includes the following header files:

### [Abstract Server](./src/server_types/include/abstract_server.h)

This file contains the abstract server class. It defines the interface that all servers must
implement.

### [Server Mode](./src/server_types/include/server_mode.h)

This file contains the server mode enum. It defines the different modes that the servers can run
in. **NOTE**: The server mode is not currently used by any of the servers since they all run in
the single-threaded mode.

___

## [TcpServer](./src/tcp_server/include/tcp_server.h)

A simple TCP server that listens on a port on the specified interface. It expects the following
format for the request:

    <request length [4 bytes]> <protocol> <data>

After receiving the request, it looks for the appropriate handler for the protocol and passes the
request to that handler. The request contains the address of the sender and a valid socket. The
handler can use the socket to send a response to the sender through the server. The handler is
responsible for managing the connection and sending the response.

___

## [UdpServer](./src/udp_server/include/udp_server.h)

A simple UDP server that listens on a port in the specified multicast group on all the specified
interfaces. It expects the following format for the request:

    <protocol> <data>

After receiving the request, it looks for the appropriate handler for the protocol and passes the
request to that handler. The request contains the address of the sender. However, the file
descriptor is not a valid socket, so the handler cannot send a response to the sender through the
server. Instead, the handler is responsible for managing the connection and sending the response.

___

