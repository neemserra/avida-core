/*
 *  nReaction.h
 *  Avida
 *
 *  Created by David on 10/6/05.
 *  Copyright 1999-2008 Michigan State University. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef nReaction_h
#define nReaction_h

namespace nReaction {
  enum {
    PROCTYPE_ADD = 0,
    PROCTYPE_MULT,
    PROCTYPE_POW,
    PROCTYPE_LIN,
    PROCTYPE_ENERGY,
    PROCTYPE_ENZYME,
    PROCTYPE_EXP
  };
}

#endif