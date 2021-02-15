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

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <cstdio>
#include <SyscDrv.h>
#include <SyscJson.h>

namespace SyscDrv
{
    using namespace std;
    using namespace SyscMsg;
    using namespace SyscMsg::Chars;
    using namespace SyscJson;

    // =============================================================================
    // Class DrvErr
    // =============================================================================
    DrvErr::DrvErr(string s)
    {
        this->err_msg = s;
    }

    DrvErr::~DrvErr(void)
    {
    }

    string
    DrvErr::get_msg(void)
    {
        return "DrvErr reports" + SP + this->err_msg;
    }

    // =============================================================================
    // Class DrvClient
    // =============================================================================
    /** \brief Constructor for DrvClient debug instance
     *
     *  The arg_msgid parameter specifies that debug messages are desired,
     *  and is used to prefix all debug messages.
     *
     *  The arg_exe parameter specifies the server executable
     */
    DrvClient::DrvClient(string & arg_msgid, string & arg_exe)
    {
        this->msg  = unique_ptr<Msg>(new Msg(arg_msgid));

        try
        {
            this->srv_exe = arg_exe;
            this->srv_nam = srv_exe.substr(srv_exe.find_last_of('/') + 1);
            this->srv_up();
        }
        catch(DrvErr & err)
        {
            this->msg->cerr_err("catch() in constructor");
            this->msg->cerr_err(err.get_msg());
            throw DrvErr("failure in DrvClient constructor");
        }
    }

    /** \brief Constructor for DrvClient non-debug instance
     *
     *  The arg_exe parameter specifies the server executable
     */
    DrvClient::DrvClient(string & arg_exe)
    {
        this->msg  = unique_ptr<Msg>(nullptr);

        try
        {
            this->srv_exe = arg_exe;
            this->srv_nam = srv_exe.substr(srv_exe.find_last_of('/') + 1);
            this->srv_up();
        }
        catch(DrvErr & err)
        {
            throw DrvErr("failure in DrvClient constructor");
        }
    }

    /** \brief Destructor for DrvClient
     *
     *  Kills the server executable.
     */
    DrvClient::~DrvClient(void)
    {
        this->srv_dn();
    }

    void
    DrvClient::srv_up(void)
    {
        int req_pipe[2];
        int res_pipe[2];

        if (pipe(req_pipe))
        {
            perror("DrvClient::srv_up() req_pipe");
            throw DrvErr("failure opening req_pipe in DrvClient::srv_up()");
        }

        if (pipe(res_pipe))
        {
            perror("DrvClient::srv_up() res_pipe");
            throw DrvErr("failure opening res_pipe in DrvClient::srv_up()");
        }

        this->req_pipe_in  = req_pipe[1];
        this->req_pipe_out = req_pipe[0];
        this->res_pipe_in  = res_pipe[1];
        this->res_pipe_out = res_pipe[0];

        this->srv_pid = fork();

        if (this->srv_pid == -1)
        {
            perror("DrvClient::srv_up() fork");
            throw DrvErr("failure to fork in DrvClient::srv_up()");
        }

        if (this->srv_pid == 0)
        {
            if (close(this->req_pipe_in))
            {
                perror("DrvClient::srv_up() child closing req_pipe_in");
                throw DrvErr("failure of child to close req_pipe_in in DrvClient::srv_up()");
            }

            if (close(this->res_pipe_out))
            {
                perror("DrvClient::srv_up() child closing res_pipe_out side of res_pipe");
                throw DrvErr("failure of child to close res_pipe_out side of res_pipe in DrvClient::srv_up()");
            }

            if (dup2(this->req_pipe_out, STDIN_FILENO) == -1)
            {
                perror("DrvClient::srv_up() child dup2() of req_pipe");
                throw DrvErr("failure of child to dup2() req_pipe in DrvClient::srv_up()");
            }
          
            if (dup2(this->res_pipe_in, STDOUT_FILENO) == -1)
            {
                perror("DrvClient::srv_up() child dup2() of res_pipe");
                throw DrvErr("failure of child to dup2() res_pipe in DrvClient::srv_up()");
            }

            if (execl(this->srv_exe.c_str(), this->srv_nam.c_str(), (char *) NULL))
            {
                perror("DrvClient::srv_up() child execl() failed");
                throw DrvErr
                (
                    "failure of child to execl("
                    + DQ + this->srv_exe + DQ + CM + SP
                    + DQ + this->srv_nam + DQ
                    + ") in DrvClient::srv_up()"
                );
            }

            throw DrvErr("child failure");
        }
        else
        {
            if (close(this->req_pipe_out))
            {
                perror("DrvClient::srv_up() parent closing req_pipe_out");
                throw DrvErr("failure of parent to close req_pipe_out in DrvClient::srv_up()");
            }

            if (close(this->res_pipe_in))
            {
                perror("DrvClient::srv_up() parent closing res_pipe_in");
                throw DrvErr("failure of parent to close res_pipe_in in DrvClient::srv_up()");
            }

            char   buf;
            string str_request = "{\"request\": \"startup_callback\", \"parm\": {}}\n";
            string obs_return  = "";
            string exp_return  = "{\"response\": \"ACK\", \"handler\": \"startup_callback\", \"data\": {}}";

            for (size_t i = 0 ; i < str_request.size() ; i++)
            {
                buf = str_request.at(i);
                write(this->req_pipe_in, &buf, 1);
            }

            while (read(this->res_pipe_out, &buf, 1) > 0)
            {
                if (buf == '\n')
                {
                    break;
                }
                else
                {
                    obs_return.push_back(buf);
                }
            }

            if (obs_return.compare(exp_return) != 0)
            {
                throw DrvErr("DrvClient::srv_up: unexpected server startup response");
            }

            cerr << "[INF] DrvClient: started server pid" << SP << dec << this->srv_pid << endl << flush;
        }
    }

    void
    DrvClient::srv_dn(void)
    {
        if (this->srv_pid == 0)
        {
            return;
        }

        if (kill(this->srv_pid, SIGKILL))
        {
            perror("DrvClient::srv_dn() kill failure");
            throw DrvErr("failure of kill in DrvClient::srv_dn()");
        }

        if (waitpid(this->srv_pid, NULL, 0) == -1)
        {
            perror("DrvClient::srv_dn() waitpid failure");
            throw DrvErr("failure of waitpid in DrvClient::srv_dn()");
        }

        cerr << "[INF] DrvClient: killed server pid" << SP << dec << this->srv_pid << endl << flush;

        this->srv_pid = 0;
        return;
    }

    void
    DrvClient::req_encap(string & arg_req, string & arg_handler, string & arg_parm)
    {
        JsonStr req;

        req.add_obj_bgn();
            req.add_key("request");
            req.add_str(arg_handler);
            req.add_key("parm");
            req.add_val(arg_parm);
        req.add_obj_end();

        arg_req = req.get_str();
    }

    void
    DrvClient::res_decap(string & arg_return, string & arg_handler)
    {
        JsonStr    json_pstr_status;
        JsonStr    json_pstr_handler;
        JsonStr    json_pstr_data;
        JsonFind   json_ret;
        string     tmp;

        json_pstr_status.add_obj_bgn();
            json_pstr_status.add_key("response");
            json_pstr_status.add_tru();
        json_pstr_status.add_obj_end();

        json_pstr_handler.add_obj_bgn();
            json_pstr_handler.add_key("handler");
            json_pstr_handler.add_tru();
        json_pstr_handler.add_obj_end();

        json_pstr_data.add_obj_bgn();
            json_pstr_data.add_key("data");
            json_pstr_data.add_obj_bgn();
            json_pstr_data.add_obj_end();
        json_pstr_data.add_obj_end();

        // parse return
        json_ret.set_search_context(arg_return);

        // check return status
        if (true)
        {
            json_ret.set_search_path(json_pstr_status.get_str());
            json_ret.find();

            if (!json_ret.context_is_str())
            {
                throw DrvErr("failure to find return status in res_decap()");
            }

            json_ret.get_context_string(tmp);

            if (tmp.compare("ACK") != 0)
            {
                if (tmp.compare(0, 4, "NAK ") == 0)
                {
                    tmp.erase(0, 4);
                    throw DrvErr("request NAK in res_decap(), NAK message is" + SP + DQ + tmp + DQ);
                }
                else
                {
                    throw DrvErr("unknown return status in res_decap()");
                }
            }
        }

        // check handler
        if (true)
        {
            json_ret.set_search_path(json_pstr_handler.get_str());
            json_ret.find();

            if (!json_ret.context_is_str())
            {
                throw DrvErr("failure to find handler in res_decap()");
            }

            json_ret.get_context_string(tmp);

            if (tmp.compare(arg_handler) != 0)
            {
                throw DrvErr("bad handler (" + DQ + tmp + DQ + ") in res_decap()");
            }
        }

        // check data
        if (true)
        {
            json_ret.set_search_path(json_pstr_data.get_str());
            json_ret.find();

            if (!json_ret.context_is_obj_bgn())
            {
                throw DrvErr("failure to find data in res_decap()");
            }

            json_ret.get_context_string(tmp);
            arg_return = tmp;
        }

        // dump return
        if ((false) && (this->msg != nullptr))
        {
            string       tmp        = arg_return;
            const char * char_bytes = tmp.data();

            for (unsigned i = 0 ; i < tmp.size() ; i++)
            {
                printf("\"%c\" %02X\n", char_bytes[i], char_bytes[i]);
            }
        }
    }

    /** \brief Method for requesting data
     *
     *  The arg_return argument is a string that is loaded with JSON from the
     *  server in response to the request parameter.
     *
     *  The arg_handler argument specifies a handler instantiated within the
     *  server.
     *
     *  The arg_parm argument is a JSON string specifying a request parameter
     *  that is passed to the handler.
     */
    void
    DrvClient::request(string & arg_return, string & arg_handler, string & arg_parm)
    {
        string str_request    = "";
        string str_stdio_data = "";
        char   buf;

        arg_return = "";

        this->req_encap(str_request, arg_handler, arg_parm);

        if (this->msg != nullptr)
        {
            this->msg->cerr_inf("request is" + SP + str_request);
        }

        str_request = str_request + NL;

        for (size_t i = 0 ; i < str_request.size() ; i++)
        {
            buf = str_request.at(i);
            write(this->req_pipe_in, &buf, 1);
        }

        while (read(this->res_pipe_out, &buf, 1) > 0)
        {
            if (buf == '\n')
            {
                break;
            }
            else
            {
                arg_return.push_back(buf);
            }
        }

        if (this->msg != nullptr)
        {
            this->msg->cerr_inf("response is" + SP + DQ + arg_return + DQ);
        }

        if (arg_return.size() == 0)
        {
            throw DrvErr("server failure in DrvClient::request()");
        }

        this->res_decap(arg_return, arg_handler);
    }
}
