/*
 * This file is part of cc-common
 *
 * Copyright (c) 2004-2009 The University of Waikato, Hamilton, New Zealand.
 * Authors: Daniel Lawson
 *          Shane Alcock
 *          Perry Lorier
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARSE_CMD_H
#define PARSE_CMD_H 1

void parse_cmd(char *buf, int *pargc, char *pargv[], int MAXTOKENS,
	       const char *command_name);


#endif // PARSE_CMD_H
