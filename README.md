# SERVERCC

This project aims to provide simple, easy to use, and efficient servers and clients for multiple
transport layer protocols. The servers and clients are designed to make as few assumptions as
possible about the data being sent and received and about the flow of the data. To achieve this,
the servers and clients are designed to be as generic as possible and to provide a simple interface
for the user to implement their own handlers for the data.

However, as a result of this design decision, the servers and clients do not provide much in the
way of functionality. The servers and clients are designed to be used as a base for other projects
essentially providing a wrapper around the transport layer protocol.

This library also provides a simple distributed system framework that can be used to build
distributed systems. The framework provides a simple interface for the user to implement their
distribute services by handling requests and sending responses. It also manages the communication
between the multiple servers and clients in the distributed system.
