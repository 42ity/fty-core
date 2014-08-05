/* cli.h: command line interface
 
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>
 
Description: definitions for command line interface
References: BIOS-245
*/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/*! \brief global options for cli */
struct global_opts {
    bool show_help;     /*! when --help is there */
    int verbosity;      /*! 0-5 to setup verbosity, 0 is default */
    bool use_pager;     /*! to use the pager */
};

/*! \brief maps command to pointers to do_ functions */
struct command {
    const char* command;
	const char* description;
    int (*do_command) (const int, const char **, const struct global_opts *);
};

/*! \brief Parse global options
 *          everything until first string without initial '-'
 *
 * Example:
 *
 *   const char *argv8[] = {"cli", "--unknown", NULL};
 *   optind = handle_global_options(2, argv8, &gopts);
 *
 *  \param argc argument count
 *  \param argv and array of strings with arguments
 *  \param gopts pointer to structure containing the global options - things will be handled there
 *
 *  \return negative, zero or positive. Where negative is an index to first
 *   unknown argument, zero means bad input arguments or --help
 *   (gopts->show_help will be true) and positive is an index to first command
 *   in argv.
 */
int handle_global_options(const int argc, const char **argv, struct global_opts *gopts);

/*! Get builtin command
 * 
 * \param commands a list of commands to search
 * \param name name of command to find in global builtin_commands array
 * \return pointer to struct command or NULL
 */
const struct command* get_builtin_command(const struct command *commands, const char *name);

/*! \bried 'network' command  */
int do_network(const int argc, const char **argv, const struct global_opts *opts);

#ifdef __cplusplus
}
#endif
