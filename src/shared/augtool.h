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

#ifndef SRC_SHARED_AUGTOOL_H
//! Guard
#define SRC_SHARED_AUGTOOL_H

/*!
 * \file augtool.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Not yet documented file
 */
#include <string>
#include <mutex>
#include <functional>

#include "subprocess.h"

//! Simple class abstraction over augtool
class augtool {
protected:
    //! Shared mutex
    std::mutex mux;
    //! Subprocess itself
    shared::SubProcess *prc;
    //! Ensures we are in reasonably clean state
    void clear();
public:
    //! Singleton get_instance method
    static augtool* get_instance();
    //! Runs command without returning anything
    void run_cmd(std::string cmd);
    /**
     * \brief Method returning parsed output of augtool
     *
     * If there is more than two lines, omits first and the last one. Returns all
     * values concatenated using settings in optional parameters.
     *
     * @param cmd what to execute
     * @param key_value if true each line is expected in form 'key = value' and only value is outputed
     * @param sep used to separate individual values
     * @param filter values for which it returns true are omitted
     *
     */
    std::string get_cmd_out(std::string cmd, bool key_value = true,
                            std::string sep = "",
                            std::function<bool(std::string)> filter = 
                            [](const std::string) -> bool { return false; });
    //! Return string directly as returned from augtool
    std::string get_cmd_out_raw(std::string cmd);
    //! Saves current state
    void save() { run_cmd("save"); }
};

#endif // SRC_SHARED_AUGTOOL_H
