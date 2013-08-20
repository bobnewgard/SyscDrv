## Introduction

SyscDrv provides an object that requests data from a server.
The server is an executable that replies to JSON requests.

SyscDrv manages the lifecycle of the server instance.  The server
is started in the SyscDrv constructor and is stopped in the SyscDrv
destructor.

### SyscDrv Use Cases

#### Supported Use Cases

* Single SyscDrv instance supporting multiple requesters and handlers
* Testbench vector source
* Testbench configuration source
* IP Client/Server Emulation

#### Unsupported Use Cases

* TBD

### SyscDrv IPC

Communication between the server and SyscDrv is via a pair of pipes.
SyscDrv creates the pipes and dups the server ends of the pipes to
stdin and stdout.
Thus, from the perspective of the server, requests are read from stdin and
responses are written to stdout; from the perspective of SyscDrv, requests
are written to a pipe file descriptor (FD) and responses are read from
another pipe FD.  In the figure below, the server is implemented by PyDrv.


                          +--------------+
                          |   req_pipe   |
                          |--------------|
               +--------->|[1]        [0]+-----------+
               |          +--------------+           | stdin
               |                                     v
         +-----+-----+                          +---------+
         |  SyscDrv  |                          |  PyDrv  |
         +-----------+    +--------------+      +----+----+
               ^          |   res_pipe   |           |
               |          |--------------|           | stdout
               +----------+[0]        [1]|<----------+
                          +--------------+


Requests are JSON that contain a handler specificaton and a parameter specification.
The parameter is handler-specific.  The request has the form

    {"request": "%s", "parm": %s}

The first string argument is the handler name and the second is
the handler-specific parameter.

Responses are JSON that come in two forms, acknowledgement and non-acknowledgement.  The
acknowledgement response has the form

    {"response": "ACK", "handler": "%s", "data": %s}

The first string argument is the handler name and the second is
the handler-specific response data.  The non-acknowledgement response has
the form

    {"response": "NAK %s", "handler": "%s", "data": %s}

The first string argument is an error message, the second is
the handler name and the third is handler-specific data.

### Server

The pydrv module is a Python implementation of a SyscDrv server.
The pydrv module provides the base class for all pydrv server
executables.
The pydrv\_server module is a server instance that extends
pydrv with a callback for generating packets.

### Unit Tests

Unit tests check good and bad handler requests.

## Validated Environments

The unit tests have been run successfully in the following environments

| Linux                | libc  | gcc   | SystemC | make | bash   |
|----------------------|-------|-------|---------|------|--------|
| Debian 3.2.0-4-amd64 | 2.13  | 4.7.2 | 2.3.0   | 3.81 | 4.2.37 |

## Other Dependencies

* SyscMsg
* SyscJson

## Installation

1. Make sure you have installed the components shown in the
   "Validated Environments" section
    * Install SystemC from source, since it is unlikely to be
      packaged
1. Clone repos listed in "Other Dependencies"
    * Clone such that SyscDrv, SyscJson and SyscMsg repos are
      in the same directory
1. Modify the path to the SystemC installation, SYSC\_DIR,
   in Makefile
1. execute "make" for hints about available targets

## Issues

Issues are tracked at [github.com/bobnewgard/SyscDrv/issues](https://github.com/bobnewgard/SyscDrv/issues)

## Pull Requests

Pull requests are found at [github.com/bobnewgard/SyscDrv/pulls](https://github.com/bobnewgard/SyscDrv/pulls)

## License

### License For Code

The code in this project is licensed under the GPLv3

### License for This Project Summary

This work is licensed under the Creative Commons Attribution-ShareAlike 3.0
Unported License. To view a copy of this license, visit
http://creativecommons.org/licenses/by-sa/3.0/.
