/*
 *  Platform.cc
 *  Avida
 *
 *  Created by David on 6/29/07.
 *  Copyright 2007-2010 Michigan State University. All rights reserved.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Platform.h"

#if AVIDA_PLATFORM(APPLE)
# include <mach/mach.h>
#elif AVIDA_PLATFORM(UNIX)
# include <unistd.h>
#endif

#include "FloatingPoint.h"


// Initialize various platform settings and system handlers
void AvidaTools::Platform::Initialize()
{
  SetupFloatingPointEnvironment();
}

  
// Autodetect the number of CPUs on a box, if available.  Otherwise, return 1.
int AvidaTools::Platform::AvailableCPUs()
{
  int ncpus = 1;
  
#if AVIDA_PLATFORM(APPLE)
  kern_return_t kr;
  host_basic_info_data_t p_host_info;     
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&p_host_info, &count);
  if (kr == KERN_SUCCESS) ncpus = (int)(p_host_info.avail_cpus);
#elif AVIDA_PLATFORM(UNIX) && defined(_SC_NPROCESSORS_ONLN)
  ncpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
  
  return ncpus;
}