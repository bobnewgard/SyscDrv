/*
 * Copyright 2013 Robert Newgard
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

/** \file  SyscDrv.h
 *  \brief Declares the Drv class.
 *
 *  This file declares the Drv class along with it's helper types and
 *  constants.
 *
 *  It also contains doxygen text that may not fit anywhere else.
 *
 */

#ifndef _SYSCDRV_H_
    #define _SYSCDRV_H_

    #include <string>
    #include <SyscMsg.h>

    namespace SyscDrv
    {
        using SyscMsg::Msg;
        using std::string;
        using std::unique_ptr;

        class DrvErr
        {
            public:
            string err_msg;

            DrvErr(string);
            ~DrvErr(void);

            string get_msg(void);
        };

        class DrvClient
        {
            private:
            unique_ptr<Msg> msg;
            string          srv_exe;
            string          srv_nam;
            pid_t           srv_pid;
            int             req_pipe_in;
            int             req_pipe_out;
            int             res_pipe_in;
            int             res_pipe_out;

            void srv_up(void);
            void srv_dn(void);
            void req_encap(string&, string&, string&);
            void res_decap(string&, string&);

            public:
            DrvClient(string&, string&);
            DrvClient(string&);
            ~DrvClient(void);

            void request(string&, string&, string&);
        };
    }
#endif
