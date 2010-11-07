/*
 *  cProbSchedule.h
 *  Avida
 *
 *  Called "prob_schedule.hh" prior to 12/7/05.
 *  Copyright 1999-2009 Michigan State University. All rights reserved.
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

#ifndef cDemeProbSchedule_h
#define cDemeProbSchedule_h

#ifndef cRandom_h
#include "cRandom.h"
#endif
#ifndef cSchedule_h
#include "cSchedule.h"
#endif
#ifndef cWeightedIndex_h
#include "cWeightedIndex.h"
#endif
#ifndef tArray_h
#include "tArray.h"
#endif


/**
 * The DemeProbiblistic Schedule has the chance for an item to
 * be scheduled within each Deme proportional to the merit of that 
 * item relative to the rest of the deme.
 * 
 * @AWC -- further implementation notes in CC file.
 **/

class cMerit;

class cDemeProbSchedule : public cSchedule
{
private:

  // Keep our own RNG so as to better preserve consistancy.
  cRandom m_rng; 

  // Array of WeightedIndex tree's to farm out the scheduling.
  tArray<cWeightedIndex*> chart;

  // how many demes are there?
  int num_demes;

  // size of each deme... num_cells/num_demes
  int deme_size;

  // what deme should GetNextID give the next CPU cycle to?
  int curr_deme;

  
  cDemeProbSchedule(const cDemeProbSchedule&); // @not_implemented
  cDemeProbSchedule& operator=(const cDemeProbSchedule&); // @not_implemented


public:
  cDemeProbSchedule(int num_cells, int seed, int ndemes) : cSchedule(num_cells), m_rng(seed), num_demes(ndemes)
  {
    deme_size = num_cells / num_demes;

    for(int i = 0; i < num_demes; i++) chart.Push(new cWeightedIndex(deme_size));
  }
  ~cDemeProbSchedule() { for (int i = 0; i < chart.GetSize(); i++) delete chart[i]; }

  virtual void Adjust(int item_id, const cMerit& merit, int deme_id = 0);

  int GetNextID();
};


#ifdef ENABLE_UNIT_TESTS
namespace nProbSchedule {
  /**
   * Run unit tests
   *
   * @param full Run full test suite; if false, just the fast tests.
   **/
  void UnitTests(bool full = false);
}
#endif  

#endif