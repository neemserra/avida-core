/*
 *  cFixedBlock.h
 *  Avida
 *
 *  Called "fixed_block.hh" prior to 12/7/05.
 *  Copyright 1999-2007 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
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

#ifndef cFixedBlock_h
#define cFixedBlock_h

/**
 * Class used by @ref cBlockStruct.
 **/

class cFixedBlock {
private:
  int start_point;   // Starting point in the cScaledBlock array.
public:
  cFixedBlock() : start_point(0) { ; }
  ~cFixedBlock() { ; }

  inline int GetStart() { return start_point; }
  inline void SetStart(int in_sp) { start_point = in_sp; }

  /**   
   * Serialize to and from archive.
   **/  
  template<class Archive>
  void serialize(Archive & a, const unsigned int version){
    a.ArkvObj("start_point", start_point);
  }   
};

#endif
