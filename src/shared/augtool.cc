/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file augtool.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <vector>
#include <string>
#include <functional>
#include <cxxtools/split.h>

#include "augtool.h"

using namespace shared;

std::string augtool::get_cmd_out(std::string cmd, bool key_value,
                                 std::string sep,
                                 std::function<bool(std::string)> filter) {
    std::string in = get_cmd_out_raw(cmd);
    std::vector<std::string> spl;
    bool not_first = false;
    std::string out;
    cxxtools::split("\n", in, std::back_inserter(spl));
    if(spl.size() >= 3) {
        spl.erase(spl.begin());
        spl.pop_back();
    } else {
        return out;
    }
    for(auto i : spl) {
        auto pos = i.find_first_of("=");
        if(pos == std::string::npos) {
            if(key_value)
                continue;
            if(not_first)
                out += sep;
            if(filter(i))
                continue;
            out += i;
        } else {
            if(not_first)
                out += sep;
            if(filter(i.substr(pos+2)))
                continue;
            out += i.substr(pos+2);
        }
        not_first = true;
    }
    return out;
}


std::string augtool::get_cmd_out_raw(std::string command) {
    std::string ret;
    bool err = false;
    mux.lock();
    if(command.empty() || command.back() != '\n')
        command += "\n";
    if(write(prc->getStdin(), command.c_str(), command.length()) < 1)
        err = true;
    ret = wait_read_all(prc->getStdout());
    mux.unlock();
    return err ? "" : ret;
}

void augtool::run_cmd(std::string cmd) {
    get_cmd_out_raw(cmd);
}

void augtool::clear() {
    static std::mutex clear_mux;
    clear_mux.lock();
    run_cmd("");
    run_cmd("load");
    clear_mux.unlock();
}

augtool* augtool::get_instance() {
    static augtool inst;
    static std::mutex in_mux;
    std::string nil;

    // Initialization of augtool subprocess if needed
    in_mux.lock();
    if(inst.prc == NULL) {
        Argv exe = { "sudo", "augtool", "-e" };
        inst.prc = new shared::SubProcess(exe,
                          SubProcess::STDOUT_PIPE | SubProcess::STDIN_PIPE);
    }
    if(!inst.prc->isRunning()) {
        inst.prc->run();
        nil = inst.get_cmd_out_raw("help");
        if(!inst.prc->isRunning() || nil.find("match") == nil.npos) {
            delete inst.prc;
            inst.prc = NULL;
            return NULL;
        }
    }
    in_mux.unlock();
    inst.clear();
    return &inst;
}

