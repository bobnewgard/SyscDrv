#!/usr/bin/python2.7
#
# Copyright 2013 Robert Newgard
#
# This file is part of SyscDrv.
#
# SyscDrv is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SyscDrv is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SyscDrv.  If not, see <http://www.gnu.org/licenses/>.
#

""" server classes for handling JSON requests

This module provides infrastructure for a simple, JSON-based RPC server
mechanism over TCP.

The server is instantiated by calling the PyDrv class constructor with
parameters for TCP host and port number.

The server should be initialized with user-defined callbacks before
handling requests.

Callbacks subclass PydrvCallback; the PydrvCallback method cb should be
redefined to provide JSON data in response to being called with a
single JSON parameter.

The callback defines the structure of both the request parameter and the
response data.

The request will contain a handler specificaton and a parm specification.
The parm is handler-specific.  The request should look like this:

    {"request": "%s", "parm": %s}

The first string argument is the handler name and the second argument is
the handler-specific parameter.

"""
#
import sys
import json
import traceback
#
SP = '\x20'
BS = '\\'
NL = '\n'
SQ = '\''
DQ = '\"'
TB = '\t'
#
json_patn_ack = '{"response": "ACK", "handler": "%s", "data": %s}'
json_patn_nak = '{"response": "NAK %s", "handler": "%s", "data": %s}'
#
# ------------------------------------------------------------------------------
class PydrvCallback(object):
    "Callback class for PyDrv"
    __list = []
    #
    def __init__(self):
        PydrvCallback.__list.append(self.__class__.__name__)
    #
    def cb(self, req_str):
        print >> sys.stderr, "[INF]" + SP + self.__class__.__name__ + ".cb()"
    #
    def load_json(self, json_str):
        try:
            req = json.loads(json_str)
        except:
            raise PydrvException("cannot parse JSON string")
        #
        return req
    #
#
class user_template(PydrvCallback):
    "Template for user-defined Pydrv callback"
    def __init__(self):
        PydrvCallback.__init__(self)
    #
    def cb(self, req_str):
        PydrvCallback.cb(self, req_str)
        #
        # add user logic below
    #
#
#
# ------------------------------------------------------------------------------
class startup_callback(PydrvCallback):
    """ Callback for confirming server startup

    The request should look like this:

        '{"request": "startup_callback", "parm": {}}'

    The parameter is ignored.  The response looks like this:

        '{"response": "ACK", "handler": "startup_callback", "data": {}}'

    """
    def __init__(self):
        PydrvCallback.__init__(self)
    #
    def cb(self, req_str):
        #
        return "{}"
    #
#
#
# ------------------------------------------------------------------------------
class PydrvException(Exception):
    "Exception for PyDrv"
    __msg_str = ""
    #
    def __init__(self, msg_str):
        self.__msg_str = msg_str
    #
    def get_msg(self):
        return self.__msg_str
    #
#
#
# ------------------------------------------------------------------------------
class PyDrv(object):
    "Dispacher for requests"
    #
    __dispatch_table  = {}
    __dispatch_host   = ""
    __dispatch_port   = ""
    __dispatch_server = None
    #
    def __init__(self):
        self.__dispatch_server = PydrvStdioServer(self.dispatch)
        cb = startup_callback()
        self.register(cb)
    #
    def register(self, callback):
        cb_name = callback.__class__.__name__
        #
        print >> sys.stderr, "[INF] Pydrv: PydrvAgent.register() callback" + NL + TB + "\"%s\" for callback class \"%s\"" % (callback, cb_name)
        #
        if (cb_name in self.__dispatch_table):
            print >> sys.stderr, "[WRN] Pydrv: PydrvAgent.register(), overwriting callback class \"%s\"" % cb_name
        #
        self.__dispatch_table[cb_name] = callback
        #
        return True
    #
    def serve(self):
        try:
            self.__dispatch_server.serve_forever()
        except KeyboardInterrupt:
            print >> sys.stderr, "\n[INF] Pydrv: KeyboardInterrupt"
            self.__dispatch_server.shutdown()
            print >> sys.stderr, "[INF] Pydrv: shutdown() complete"
        #
    #
    def dispatch(self, req_str):
        try:
            recv_json = json.loads(req_str)
        except:
            print >> sys.stderr, "[ERR] Pydrv: -- dispatch req_str bgn --"
            print >> sys.stderr, req_str
            print >> sys.stderr, "[ERR] Pydrv: -- dispatch req_str end --"
            print >> sys.stderr, "[ERR] Pydrv: PydrvAgent.dispatch(), cannot parse request JSON"
            return json_patn_nak % ("cannot parse request JSON", "null", "null")
        #
        try:
            req_cb    = recv_json["request"]
        except:
            return json_patn_nak % ("no handler", "null", "null")
        #
        try:
            req_parm  = recv_json["parm"]
        except:
            return json_patn_nak % ("no parm", req_cb, "null")
        #
        try:
            req_parm  = json.dumps(req_parm)
        except:
            return json_patn_nak % ("cannot serialize parm to JSON", req_cb, "null")
        #
        if (not req_cb in self.__dispatch_table):
            print >> sys.stderr, "[WRN] Pydrv: PydrvAgent.dispatch(), no callback class for \"%s\"" % req_cb
            return json_patn_nak % ("unregistered handler", req_cb, "null")
        #
        if False:
            print >> sys.stderr, "[INF] Pydrv: PydrvAgent.dispatch() for callback class \"%s\"" % req_cb
        #
        # Requirements
        #   1. See non-PydrvException exceptions on console
        #   2. Always return JSON to handle()
        # Issue
        #   * From The Python Language Reference: "If the finally clause raises another
        #     exception or executes a return or break statement, the saved exception is
        #     discarded"
        try:
            res_parm = self.__dispatch_table[req_cb].cb(req_parm)
        except PydrvException as excp:
            res_json = json_patn_nak % (excp.get_msg(), req_cb, "null")
        except:
            print >> sys.stderr, "[ERR] Pydrv: ----------------------------------------"
            traceback.print_exc()
            print >> sys.stderr, "[ERR] Pydrv: ----------------------------------------"
            res_json = json_patn_nak % ("server raised a python exception", req_cb, "null")
        else:
            res_json = json_patn_ack % (req_cb, res_parm)
            #
            if (False):
                print >> sys.stderr, res_json
            #
        finally:
            return res_json
        #
    #
#
#
# ------------------------------------------------------------------------------
class PydrvStdioServer():
    "PyDrv as shell process"
    #
    __dispatch = None
    #
    def __init__(self, dispatch):
        self.__dispatch = dispatch
    #
    def shutdown(self):
        pass
    #
    def serve_forever(self):
        line = ""
        while (True):
            char = sys.stdin.read(1)
            #
            if (char == '\n'):
                self.handle(line)
                line = ""
            else:
                line = line + char
        #
    #
    def handle(self, line):
        res_str = self.__dispatch(line)
        res_str = res_str + NL
        #
        sys.stdout.write(res_str)
        sys.stdout.flush()
    #
#
#
# ------------------------------------------------------------------------------
if (__name__ == "__main__"):
    class test1(PydrvCallback):
        def __init__(self):
            PydrvCallback.__init__(self)
        #
        def cb(self, req_str):
            print >> sys.stderr, "[INF] Pydrv: test1() callback for" + SP + req_str
            return 'true'
        #
    #
    disp      = PyDrv()
    cb_test_1 = test1()
    #
    disp.register(cb_test_1)
    disp.serve()
    #
    exit(0)
#
