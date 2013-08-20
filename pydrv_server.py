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

"""
Subclasses Pydrv to provide server executable and test callbacks
"""
#
from pydrv        import *
from scapy.all    import *
#
# ------------------------------------------------------------------------------
def get_byte_list(scapy_pkt):
    """
    Converts scapy packet to a list of strings, with each string
    encoding a byte value in hex
    """
    scapy_pkt_str = str(scapy_pkt)
    byte_list     = []
    #
    for char in scapy_pkt_str:
        hex_str   = "%02X" % (ord(char))
        byte_list.append(hex_str)
    #
    return byte_list
#
# ------------------------------------------------------------------------------
class test1(PydrvCallback):
    """
    Example callback
    """
    def __init__(self):
        PydrvCallback.__init__(self)
    #
    def cb(self, req_str):
        req = self.load_json(req_str)
        print >> sys.stderr, "[INF] test1() callback for" + SP + req_str
        return json.dumps(True)
    #
#
class dot3_by_len(PydrvCallback):
    """
    Packet data callback
        parm: {size: json_number}
        data: {frame: ["XX", ...]}
    """
    def __init__(self):
        PydrvCallback.__init__(self)
    #
    def cb(self, req_str):
        siz = 64
        pad = ""
        req = self.load_json(req_str)
        #
        print >> sys.stderr, "[INF] dot3_by_len() callback for" + SP + req_str
        #
        try:
            siz = req["size"]
        except:
            raise PydrvException("no key 'size' in parm")
        #
        if (int(siz) < 64):
            raise PydrvException("requested MAC client data field too small")
        #
        if (int(siz) > 1500):
            raise PydrvException("requested MAC client data field too large")
        #
        for i in range (siz):
            j = (i % 256)
            pad = pad + chr(j)
        #
        L2 = Dot3(dst="CA:BB:BB:BB:BB:BB", src="5A:AA:AA:AA:AA:AA")/Padding(load=pad)
        L2.len = siz
        #
        ret = {"frame": get_byte_list(L2)}
        #
        return json.dumps(ret)
    #
#
# ------------------------------------------------------------------------------
if (__name__ == "__main__"):
    disp      = PyDrv()
    cb_test_1 = test1()
    cb_test_2 = dot3_by_len()
    #
    disp.register(cb_test_1)
    disp.register(cb_test_2)
    disp.serve()
    #
    exit(0)
#
