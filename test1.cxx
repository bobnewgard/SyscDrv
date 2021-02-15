/*
 * Copyright 2013-2021 Robert Newgard
 *
 * This file is part of SyscDrv.
 *
 * SyscDrv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SyscDrv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SyscDrv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <systemc.h>
#include <iomanip>
#include <SyscDrv.h>
#include <SyscJson.h>

using namespace std;
using namespace SyscMsg;
using namespace SyscDrv;
using namespace SyscMsg::Chars;
using namespace SyscJson;

bool enable_test_01 = true;
bool enable_test_02 = true;

int
sc_main(int argc, char *argv[])
{
    bool        pass         = true;
    string      req_server   = "./pydrv_server.py";
    string      req_handler  = "";
    string      req_parm     = "";
    string      response     = "";
    string      expected     = "";
    string      msgid        = "debug";

    Msg         msg("test1:");
    DrvClient   client(req_server);
    JsonStr     json_req;
    JsonStr     json_exp;

    if (enable_test_01)
    {
        msg.cerr_inf("sub-test 01, get frame data");

        json_req.add_obj_bgn();
            json_req.add_key("size");
            json_req.add_num("66");
        json_req.add_obj_end();

        json_exp.add_obj_bgn();
            json_exp.add_key("frame");
            json_exp.add_arr_bgn();
                json_exp.add_str("CA");
                for (int i = 0 ; i < 5 ; i++) { json_exp.add_str("BB"); }
                json_exp.add_str("5A");
                for (int i = 0 ; i < 5 ; i++) { json_exp.add_str("AA"); }
                json_exp.add_str("00");
                json_exp.add_str("42");
                for (int i = 0 ; i < 66 ; i++)
                {
                    ostringstream os;
                    os << hex << setfill('0') << uppercase << setw(2) << i;
                    json_exp.add_str(os.str().c_str());
                }
            json_exp.add_arr_end();
        json_exp.add_obj_end();

        req_parm     = json_req.get_str();
        req_handler  = "dot3_by_len";
        expected     = json_exp.get_str();

        try
        {
            client.request(response, req_handler, req_parm);
        }
        catch(DrvErr & err)
        {
            msg.cerr_err(err.get_msg());
            return 1;
        }

        if (response.compare(expected) == 0)
        {
            msg.cerr_inf("[01], pass");
        }
        else
        {
            msg.cerr_err("[01], miscompare");
            msg.cerr_err("[01], expected" + SP + expected);
            msg.cerr_err("[01], observed" + SP + response);
            msg.cerr_err("[01], FAIL");
            pass = false;
        }
    }

    if (enable_test_02)
    {
        msg.cerr_inf("sub-test 02, test request with bad handler name");

        req_parm     = "null";
        req_handler  = "xxxx";

        bool tmp = false;

        try
        {
            client.request(response, req_handler, req_parm);
        }
        catch(DrvErr & err)
        {
            msg.cerr_inf("[02], exception:" + SP + err.get_msg());
            tmp = true;
        }

        if (tmp)
        {
            msg.cerr_inf("[02], pass");
        }
        else
        {
            msg.cerr_err("[02], did not catch bad handler name");
            msg.cerr_err("[02], FAIL");
            pass = false;
        }
    }

    if (pass)
    {
        msg.cerr_inf("pass");
    }
    else
    {
        msg.cerr_err("fail");
    }

    return 0;
}
