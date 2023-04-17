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

## Table of Contents

- [SERVERCC](#servercc)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
  - [Modules](#modules)

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for
development and testing purposes. See [deployment](#deployment) for notes on how to deploy the
project on a live system.

### Prerequisites

This project requires the following libraries:
- [libcc](https://github.com/OtavioPiza/libcc)

These libraries are automatically downloaded and built when building this project. However, if
you can provide a path to the libcc directory, the build process will use the provided libcc
directory instead of downloading and building libcc. This can be done by setting the `LIBCC_DIR`
when configuring the CMake project.

To build the project, you will need to have CMake installed. The project has been tested with
CMake version 3.10.2.

To build the project, you will need to have a C++ compiler installed. The project has been tested
with GCC version 7.3.0 using the C++20 standard.

## Modules


