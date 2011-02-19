/*
 *  cAvidaConfig.h
 *  Avida
 *
 *  Created by David on 10/16/05.
 *  Designed by Charles.
 *  Copyright 1999-2010 Michigan State University. All rights reserved.
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

#ifndef cAvidaConfig_h
#define cAvidaConfig_h

// This class is designed to easily allow the construction of a dynamic 
// configuration object that will handle the loading and management of all
// variables declared at runtime.  It will also allow for some variables to
// be transparently set at compile time and allow the programmer to easily
// add in new settings with a single line of code.

#include <iostream>

#include "cMutex.h"
#include "cString.h"
#include "cStringList.h"
#include "cStringUtil.h"
#include "tList.h"

class cUserFeedback;
template <class T> class tDictionary;

using namespace std;


// The following macro will create all of the code needed to include an
// entry for a new setting in the configuration.
//
// To step through what we are doing:
// 1 - Load in four variables representing the setting:
//     NAME = The name of the variable in the config file.
//     TYPE = The type of the variable (int, double, or cString)
//     DEFAULT = Value to use if this isn't set by user.
//     DESC = a brief description of the setting (a string, enclosed in quotes)
// 2 - Build a new class using the setting name with the prefix cEntry_
//     This class will contain all of the info about this setting, and will
//     be derived from the cBaseConfigEntry class so that we may easily refer
//     to any of these dynamically created classes.
// 3 - Create a private value for this setting.
// 4 - Create a LoadString() method to load the settings in from a string.
// 5 - Create a EqualsString() method to determine if the values are the same.
// 6 - Create a constructor that passes all of the information to the base
//     class that it can manage to.
// 7 - Initialize the value of this setting to its default.
// 8 - Insert the newly built object into the full list of settings objects.
// 9 - Create Get() and Set() methods to act as accessors.
// 10 - Setup a method to return the value of this setting as a string.
// 11 - Declare a variable of this class's type to use in the future.  Since
//     accessors were declared above, we can refer to this setting by the
//     setting name inside of config.

#define CONFIG_ADD_VAR(NAME, TYPE, DEFAULT, DESC)                     /* 1 */ \
class cEntry_ ## NAME : public cBaseConfigEntry {                     /* 2 */ \
private:                                                                      \
  TYPE value;                                                         /* 3 */ \
public:                                                                       \
  void LoadString(const cString& str_value) {                         /* 4 */ \
    value = cStringUtil::Convert(str_value, value);                           \
  }                                                                           \
  bool EqualsString(const cString& str_value) const {                 /* 5 */ \
    return (value == cStringUtil::Convert(str_value, value));                 \
  }                                                                           \
  cEntry_ ## NAME() : cBaseConfigEntry(#NAME,#TYPE,#DEFAULT,DESC) {   /* 6 */ \
    LoadString(GetDefault());                                         /* 7 */ \
    global_group_list.GetLast()->AddEntry(this);                      /* 8 */ \
  }                                                                           \
  TYPE Get() const { return value; }                                  /* 9 */ \
  void Set(TYPE in_value) { value = in_value; }                               \
  cString AsString() const { return cStringUtil::Convert(value); }    /* 10 */\
} NAME                                                                /* 11 */\


// Sometimes we will want to have multiple names associated with a setting.
// This macro allows the programmer to add such an alias to the most recent
// setting created.

#define CONFIG_ADD_ALIAS(ALIAS)                                      \
class cAlias_ ## ALIAS {                                             \
private:                                                             \
  cBaseConfigEntry * primary_entry;                                  \
public:                                                              \
  cAlias_ ## ALIAS() {                                               \
    primary_entry = global_group_list.GetLast()->GetLastEntry();     \
    primary_entry->AddAlias(#ALIAS);				     \
  }                                                                  \
} ALIAS

// Now we're going to make another macro to deal with groups.  This time its
// a bit simpler since there is only one type of group.  The reason that we
// want to make a new class for each new group is so that we can set default
// values for the class with its name and description.  Otherwise we would
// need to send those parameters in elsewhere.

#define CONFIG_ADD_GROUP(NAME, DESC)                                       \
class cGroup_ ## NAME : public cBaseConfigGroup {                          \
public:                                                                    \
  cGroup_ ## NAME() : cBaseConfigGroup(#NAME, DESC) { ; }                  \
} NAME



#define CONFIG_ADD_CUSTOM_FORMAT(NAME, DESC)                               \
class cCustomFormat_ ## NAME : public cBaseConfigCustomFormat {            \
public:                                                                    \
  cCustomFormat_ ## NAME() : cBaseConfigCustomFormat(#NAME, DESC) { ; }    \
} NAME


#define CONFIG_ADD_FORMAT_VAR(NAME, DESC)                                  \
private:                                                                   \
  class cFormatEntry_ ## NAME : public cBaseConfigFormatEntry {            \
  public:                                                                  \
    cFormatEntry_ ## NAME() : cBaseConfigFormatEntry(#NAME, DESC) { ; }    \
  } NAME;                                                                  \
public:                                                                    \


class cAvidaConfig {
private:
  // The cBaseConfigEntry class is a bass class for all configuration entries.
  // It is used to manage the various types of entries in a dynamic fashion.
  class cBaseConfigEntry {
  private:
    tArray<cString> config_name;  // Names for this setting (first is main name; remainder are aliases)
    const cString type;           // What type does this entry return?
    cString default_value;        // Value to use if not found in config file.
    const cString description;    // Explaination of the use of this setting
    
    // If we automatically regenerate this file to optimize its performace,
    // we can explicitly build some of these classes to return constant 
    // values (not changeable at run time).  Should this instance be one of
    // those classes?
    bool use_overide;
    
  public:
    cBaseConfigEntry(const cString& _name, const cString& _type, const cString& _def, const cString& _desc);
    virtual ~cBaseConfigEntry() { ; }
    
    virtual void LoadString(const cString& str_value) = 0;
    virtual bool EqualsString(const cString& str_value) const = 0;
    
    const cString& GetName(int id=0) const { return config_name[id]; }
    const tArray<cString>& GetNames() const { return config_name; }
    const cString& GetType() const { return type; }
    const cString& GetDefault() const { return default_value; }
    const cString& GetDesc() const { return description; }
    bool GetUseOveride() const { return use_overide; }
    int GetNumNames() const { return config_name.GetSize(); }

    void AddAlias(const cString & alias) { config_name.Push(alias); }

    virtual cString AsString() const = 0;
  };
  
  // The cBaseConfigGroup class is a base class for objects that collect the
  // configuration entries into logical groups.
  class cBaseConfigGroup {
  private:
    cString group_name;
    cString description;
    tList<cBaseConfigEntry> entry_list;
  public:
    cBaseConfigGroup(const cString& _name, const cString& _desc)
    : group_name(_name), description(_desc) { global_group_list.PushRear(this); }
    ~cBaseConfigGroup() { ; }
    
    const cString& GetName() const { return group_name; }
    const cString& GetDesc() const { return description; }
    tList<cBaseConfigEntry>& GetEntryList() { return entry_list; }
    const tList<cBaseConfigEntry>& GetEntryList() const { return entry_list; }
    cBaseConfigEntry * GetLastEntry() { return entry_list.GetLast(); }
    
    void AddEntry(cBaseConfigEntry* _entry) { entry_list.PushRear(_entry); }
  };
  
  // The cConfigCustomFormat class is a class for objects that collect the custom format configuration entries into
  // a single named custom format.
  class cBaseConfigFormatEntry;
  class cBaseConfigCustomFormat {
  private:
    cString m_format_name;
    cString m_description;
    tList<cBaseConfigFormatEntry> m_entry_list;
    cStringList m_value;
    
  public:
    cBaseConfigCustomFormat(const cString& _name, const cString& _desc)
      : m_format_name(_name), m_description(_desc) { global_format_list.PushRear(this); }
    ~cBaseConfigCustomFormat() { ; }
    
    const cString& GetName() const { return m_format_name; }
    const cString& GetDesc() const { return m_description; }
    tList<cBaseConfigFormatEntry>& GetEntryList() { return m_entry_list; }
    const tList<cBaseConfigFormatEntry>& GetEntryList() const { return m_entry_list; }
    
    void AddEntry(cBaseConfigFormatEntry* _entry) { m_entry_list.PushRear(_entry); }
    
    cStringList& Get() { return m_value; }
    const cStringList& Get() const { return m_value; }
    void Add(const cString& value) { m_value.PushRear(value); }
  };
  
  // The cConfigFormatEntry class is a bass class for all configuration entries.
  // It is used to manage the various types of entries in a dynamic fashion.
  class cBaseConfigFormatEntry {
  private:
    const cString m_name;   // Name of this setting
    const cString m_description;   // Explaination of the use of this setting
    cBaseConfigCustomFormat* m_format;
    
  public:
    cBaseConfigFormatEntry(const cString& _name, const cString& _desc)
      : m_name(_name), m_description(_desc), m_format(global_format_list.GetLast())
    {
      m_format->AddEntry(this);
    }
    ~cBaseConfigFormatEntry() { ; }
    
    void LoadString(const cString& str_value) { cString lname(m_name); m_format->Add(lname + " " + str_value); }
    
    const cString& GetName() const { return m_name; }
    const cString& GetDesc() const { return m_description; }    
  };
  
  
  // We need to keep track of all configuration groups and the entry objects
  // that they contain.  To do this, we create a global list of groups that
  // all groups must register themselves in.  A new entry object will always
  // use the last group in the list, so we merely need to make sure to keep
  // things in order.  When all configuration objects are built, we then
  // transfer the list to the local cAvidaConfig object (which occurs in the
  // constructor.)
  static cMutex global_list_mutex;
  static tList<cBaseConfigGroup> global_group_list;
  static tList<cBaseConfigCustomFormat> global_format_list;
  tList<cBaseConfigGroup> m_group_list;
  tList<cBaseConfigCustomFormat> m_format_list;
  
  
public:
  cAvidaConfig()
  {
    m_group_list.Transfer(global_group_list);
    m_format_list.Transfer(global_format_list);
    global_list_mutex.Unlock();
  }
  ~cAvidaConfig() { ; }
  
#ifdef OVERRIDE_CONFIG
#include "config_overrides.h"
#else
  // Since the global lists are static and shared across all threads, we need to have a lock on a mutex to prevent multiple
  // concurrent instantiations from messing each other up.  This class should be instantiated first, locking the mutex
  // that is then unlocked at the completion of the constructor.
  class cGlobalListLockAcquire
  {
  public:
    cGlobalListLockAcquire() { global_list_mutex.Lock(); }
  } __ListLock;
  
  // -------- General config options --------
  CONFIG_ADD_GROUP(GENERAL_GROUP, "General Settings");
  CONFIG_ADD_VAR(VERBOSITY, int, 1, "0 = No output at all\n1 = Normal output\n2 = Verbose output, detailing progress\n3 = High level of details, as available\n4 = Print Debug Information, as applicable");
  CONFIG_ADD_VAR(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)");
  CONFIG_ADD_VAR(HARDWARE_TYPE, int, 0, "0 = Default, heads-based CPUs\n1 = New SMT CPUs\n2 = Transitional SMT\n3 = Experimental CPU\n4 = Gene Expression CPU");
  CONFIG_ADD_VAR(SPECULATIVE, bool, 1, "Enable speculative execution\n(pre-execute instructions that don't affect other organisms)");
  CONFIG_ADD_VAR(POPULATION_CAP, int, 0, "Carrying capacity in number of organisms (use 0 for no cap)");
  CONFIG_ADD_VAR(POP_CAP_ELDEST, int, 0, "Carrying capacity in number of organisms (use 0 for no cap). Will kill oldest organism in population, but still use birth method to place new offspring."); 
  
  
  // -------- Topology config options --------
  CONFIG_ADD_GROUP(TOPOLOGY_GROUP, "World topology");
  CONFIG_ADD_VAR(WORLD_X, int, 60, "Width of the Avida world");
  CONFIG_ADD_VAR(WORLD_Y, int, 60, "Height of the Avida world");
  CONFIG_ADD_VAR(WORLD_GEOMETRY, int, 2, "1 = Bounded Grid (WOLRD_X x WORLD_Y)\n2 = Toroidal Grid (WOLRD_X x WORLD_Y; wraps at edges\n3 = Clique (all population cells are connected)\n4 = Hexagonal grid\n5 = Partial\n6 = 3D Lattice (under development)\n7 = Random connected\n8 = Scale-free (detailed below)");
  CONFIG_ADD_VAR(SCALE_FREE_M, int, 3, "Number of connections per cell in a scale-free geometry");
  CONFIG_ADD_VAR(SCALE_FREE_ALPHA, double, 1.0, "Attachment power (1=linear)");
  CONFIG_ADD_VAR(SCALE_FREE_ZERO_APPEAL, double, 0.0, "Appeal of cells with zero connections");	
    
  // -------- Configuration File config options --------
  CONFIG_ADD_GROUP(CONFIG_FILE_GROUP, "Other configuration Files");
  CONFIG_ADD_VAR(DATA_DIR, cString, "data", "Directory in which config files are found");
  CONFIG_ADD_VAR(INST_SET, cString, "-", "Instruction set file ('-' = use default for hardware type)");
  CONFIG_ADD_VAR(INST_SET_LOAD_LEGACY, int, 1, "Load legacy format instruction set file format");
  CONFIG_ADD_VAR(EVENT_FILE, cString, "events.cfg", "File containing list of events during run");
  CONFIG_ADD_VAR(ANALYZE_FILE, cString, "analyze.cfg", "File used for analysis mode");
  CONFIG_ADD_VAR(ENVIRONMENT_FILE, cString, "environment.cfg", "File that describes the environment");
  CONFIG_ADD_VAR(START_ORGANISM, cString, "default-heads.org", "Organism to seed the population");
  CONFIG_ADD_ALIAS(START_CREATURE);
  
  
  // -------- Mutation config options --------
  CONFIG_ADD_GROUP(MUTATION_GROUP, "Mutation rates");  
  CONFIG_ADD_VAR(COPY_MUT_PROB, double, 0.0075, "Mutation rate (per copy)");
  CONFIG_ADD_VAR(COPY_INS_PROB, double, 0.0, "Insertion rate (per copy)");
  CONFIG_ADD_VAR(COPY_DEL_PROB, double, 0.0, "Deletion rate (per copy)");
  CONFIG_ADD_VAR(COPY_UNIFORM_PROB, double, 0.0, "Uniform mutation probability (per copy)\n- Randomly apply insertion, deletion or point mutation");
  CONFIG_ADD_VAR(COPY_SLIP_PROB, double, 0.0, "Slip rate (per copy)");
  
  CONFIG_ADD_VAR(POINT_MUT_PROB, double, 0.0, "Mutation rate (per-location per update)");

  CONFIG_ADD_VAR(DIV_MUT_PROB, double, 0.0, "Mutation rate (per site, applied on divide)");
  CONFIG_ADD_VAR(DIV_INS_PROB, double, 0.0, "Insertion rate (per site, applied on divide)");
  CONFIG_ADD_VAR(DIV_DEL_PROB, double, 0.0, "Deletion rate (per site, applied on divide)");
  CONFIG_ADD_VAR(DIV_UNIFORM_PROB, double, 0.0, "Uniform mutation probability (per site, applied on divide)\n- Randomly apply insertion, deletion or point mutation");
  CONFIG_ADD_VAR(DIV_SLIP_PROB, double, 0.0, "Slip rate (per site, applied on divide)");
  
  CONFIG_ADD_VAR(DIVIDE_MUT_PROB, double, 0.0, "Mutation rate (max one, per divide)");
  CONFIG_ADD_VAR(DIVIDE_INS_PROB, double, 0.05, "Insertion rate (max one, per divide)");
  CONFIG_ADD_VAR(DIVIDE_DEL_PROB, double, 0.05, "Deletion rate (max one, per divide)");
  CONFIG_ADD_VAR(DIVIDE_UNIFORM_PROB, double, 0.0, "Uniform mutation probability (per divide)\n- Randomly apply insertion, deletion or point mutation");
  CONFIG_ADD_VAR(DIVIDE_SLIP_PROB, double, 0.0, "Slip rate (per divide) - creates large deletions/duplications");
  
  CONFIG_ADD_VAR(DIVIDE_POISSON_MUT_MEAN, double, 0.0, "Mutation rate (Poisson distributed, per divide)");
  CONFIG_ADD_VAR(DIVIDE_POISSON_INS_MEAN, double, 0.0, "Insertion rate (Poisson distributed, per divide)");
  CONFIG_ADD_VAR(DIVIDE_POISSON_DEL_MEAN, double, 0.0, "Deletion rate (Poisson distributed, per divide)");
  CONFIG_ADD_VAR(DIVIDE_POISSON_SLIP_MEAN, double, 0.0, "Slip rate (Poisson distributed, per divide)");
    
  CONFIG_ADD_VAR(INJECT_INS_PROB, double, 0.0, "Insertion rate (per site, applied on inject)");
  CONFIG_ADD_VAR(INJECT_DEL_PROB, double, 0.0, "Deletion rate (per site, applied on inject)");
  CONFIG_ADD_VAR(INJECT_MUT_PROB, double, 0.0, "Mutation rate (per site, applied on inject)");
  
  CONFIG_ADD_VAR(SLIP_FILL_MODE, int, 0, "Fill insertions from slip mutations with:\n0 = Duplication\n1 = nop-X\n2 = Random\n3 = scrambled\n4 = nop-C");
  CONFIG_ADD_VAR(SLIP_COPY_MODE, int, 0, "How to handle 'on-copy' slip mutations:\n0 = actual read head slip\n1 = instant large mutation (obeys slip mode)");
  CONFIG_ADD_VAR(PARENT_MUT_PROB, double, 0.0, "Per-site, in parent, on divide");
  CONFIG_ADD_VAR(SPECIAL_MUT_LINE, int, -1, "If this is >= 0, ONLY this line is mutated");
  CONFIG_ADD_VAR(META_COPY_MUT, double, 0.0, "Prob. of copy mutation rate changing (per gen)");
  CONFIG_ADD_VAR(META_STD_DEV, double, 0.0, "Standard deviation of meta mutation size.");
  CONFIG_ADD_VAR(MUT_RATE_SOURCE, int, 1, "1 = Mutation rates determined by environment.\n2 = Mutation rates inherited from parent.");
  
  
  // -------- Birth and Death config options --------
  CONFIG_ADD_GROUP(REPRODUCTION_GROUP, "Birth and Death config options");
	CONFIG_ADD_VAR(DIVIDE_FAILURE_RESETS, int, 0, "When Divide fails, organisms are interally reset");
  CONFIG_ADD_VAR(BIRTH_METHOD, int, 0, "Which organism should be replaced when a birth occurs?\n0 = Random organism in neighborhood\n1 = Oldest in neighborhood\n2 = Largest Age/Merit in neighborhood\n3 = None (use only empty cells in neighborhood)\n4 = Random from population (Mass Action)\n5 = Oldest in entire population\n6 = Random within deme\n7 = Organism faced by parent\n8 = Next grid cell (id+1)\n9 = Largest energy used in entire population\n10 = Largest energy used in neighborhood\n11 = Local neighborhood dispersal");
  CONFIG_ADD_VAR(PREFER_EMPTY, int, 1, "Overide BIRTH_METHOD to preferentially choose empty cells for offsping?");
  CONFIG_ADD_VAR(ALLOW_PARENT, int, 1, "Should parents be considered when deciding where to place offspring?");
  CONFIG_ADD_VAR(DISPERSAL_RATE, double, 0.0, "Rate of dispersal under birth method 11\n(poisson distributed random connection list hops)");
  CONFIG_ADD_VAR(DEATH_PROB, double, 0.0, "Probability of death when dividing.");
  CONFIG_ADD_VAR(DEATH_METHOD, int, 2, "When should death by old age occur?\n0 = Never\n1 = When executed AGE_LIMIT (+deviation) total instructions\n2 = When executed genome_length * AGE_LIMIT (+dev) instructions");
  CONFIG_ADD_VAR(AGE_LIMIT, int, 20, "See DEATH_METHOD");
  CONFIG_ADD_VAR(AGE_DEVIATION, int, 0, "Creates a normal distribution around AGE_LIMIT for time of death");
  CONFIG_ADD_VAR(ALLOC_METHOD, int, 0, "When allocating blank tape, how should it be initialized?\n0 = Allocated space is set to default instruction.\n1 = Set to section of dead genome (creates potential for recombination)\n2 = Allocated space is set to random instruction.");
  CONFIG_ADD_VAR(DIVIDE_METHOD, int, 1, "0 = Divide leaves state of mother untouched.\n1 = Divide resets state of mother(effectively creating 2 offspring)\n2 = Divide resets state of current thread only (use with parasites)");
  CONFIG_ADD_VAR(EPIGENETIC_METHOD, int, 0, "Inheritance of state information other than genome\n0 = none\n1 = offspring inherits registers and stacks of first thread\n1 = parent maintains registers and stacks of first thread\n\n1 = offspring and parent keep state information");
  CONFIG_ADD_VAR(GENERATION_INC_METHOD, int, 1, "0 = Only increase generation of offspring on divide.\n1 = Increase generation of both parent and offspring\n   (suggested with DIVIDE_METHOD 1).");
  CONFIG_ADD_VAR(RESET_INPUTS_ON_DIVIDE, int, 0, "Reset environment inputs of parent upon successful divide.");
  CONFIG_ADD_VAR(INHERIT_MERIT, int, 1, "Should merit be inhereted from mother parent? (in asexual)");
  CONFIG_ADD_VAR(INHERIT_MULTITHREAD, int, 0, "Should offspring of parents with multiple threads be marked multithreaded?");
  CONFIG_ADD_ALIAS(INHERIT_MULTI_THREAD_CLASSIFICATION);
  
	

  // -------- Divide Restrictions config options --------
  CONFIG_ADD_GROUP(DIVIDE_GROUP, "Divide restrictions and triggers - settings describe conditions for a successful divide");
  CONFIG_ADD_VAR(OFFSPRING_SIZE_RANGE, double, 2.0, "Maximal differential between offspring and parent length.\n(Checked BEFORE mutations applied on divide.)");
  CONFIG_ADD_ALIAS(CHILD_SIZE_RANGE);
  CONFIG_ADD_VAR(MIN_COPIED_LINES, double, 0.5, "Code fraction that must be copied before divide");
  CONFIG_ADD_VAR(MIN_EXE_LINES, double, 0.5, "Code fraction that must be executed before divide");
  CONFIG_ADD_VAR(MIN_GENOME_SIZE, int, 0, "Minimum number of instructions allowed in a genome. 0 = OFF");
  CONFIG_ADD_VAR(MAX_GENOME_SIZE, int, 0, "Maximum number of instructions allowed in a genome. 0 = OFF");
  CONFIG_ADD_VAR(REQUIRE_ALLOCATE, int, 1, "(Original CPU Only) Require allocate before divide?");
  CONFIG_ADD_VAR(REQUIRED_TASK, int, -1, "Task ID required for successful divide");
  CONFIG_ADD_VAR(IMMUNITY_TASK, int, -1, "Task providing immunity from the required task");
  CONFIG_ADD_VAR(REQUIRED_REACTION, int, -1, "Reaction ID required for successful divide");
  CONFIG_ADD_VAR(IMMUNITY_REACTION, int, -1, "Reaction ID that provides immunity for successful divide");
  CONFIG_ADD_VAR(REQUIRE_SINGLE_REACTION, int, 0, "If set to 1, at least one reaction is required for a successful divide");
  CONFIG_ADD_VAR(REQUIRED_BONUS, double, 0.0, "Required bonus to divide");
  CONFIG_ADD_VAR(REQUIRE_EXACT_COPY, int, 0, "Require offspring to be an exact copy (checked before divide mutations)");
  CONFIG_ADD_VAR(REQUIRED_RESOURCE, int, -1, "ID of resource required in organism's internal bins for successful\n  divide (resource not consumed)");
  CONFIG_ADD_VAR(REQUIRED_RESOURCE_LEVEL, double, 0.0, "Level of resource needed for REQUIRED_RESOURCE");  
  CONFIG_ADD_VAR(IMPLICIT_REPRO_BONUS, int, 0, "Call Inst_Repro to divide upon achieving this bonus. 0 = OFF");  
  CONFIG_ADD_VAR(IMPLICIT_REPRO_CPU_CYCLES, int, 0, "Call Inst_Repro after this many cpu cycles. 0 = OFF");  
  CONFIG_ADD_VAR(IMPLICIT_REPRO_TIME, int, 0, "Call Inst_Repro after this time used. 0 = OFF");  
  CONFIG_ADD_VAR(IMPLICIT_REPRO_END, int, 0, "Call Inst_Repro after executing the last instruction in the genome.");  
  CONFIG_ADD_VAR(IMPLICIT_REPRO_ENERGY, double, 0.0, "Call Inst_Repro if organism accumulates this amount of energy.");   

  // -------- Recombination config options --------
  CONFIG_ADD_GROUP(RECOMBINATION_GROUP, "Sexual Recombination and Modularity");
  CONFIG_ADD_VAR(RECOMBINATION_PROB, double, 1.0, "Probability of recombination in div-sex");
  CONFIG_ADD_VAR(MAX_BIRTH_WAIT_TIME, int, -1, "Updates incipiant orgs can wait for crossover (-1 = unlimited)");
  CONFIG_ADD_VAR(MODULE_NUM, int, 0, "Number of modules in the genome");
  CONFIG_ADD_VAR(CONT_REC_REGS, int, 1, "Are (modular) recombination regions continuous?");
  CONFIG_ADD_VAR(CORESPOND_REC_REGS, int, 1, "Are (modular) recombination regions swapped randomly\n or with corresponding positions?");
  CONFIG_ADD_VAR(TWO_FOLD_COST_SEX, int, 0, "0 = Both offspring are born (no two-fold cost)\n1 = only one recombined offspring is born.");
  CONFIG_ADD_VAR(SAME_LENGTH_SEX, int, 0, "0 = Recombine with any genome\n1 = Recombine only w/ same length");
  CONFIG_ADD_VAR(ALLOW_MATE_SELECTION, bool, 0, "Allow organisms to select mates (requires instruction set support)");

	
  // -------- Parasite options --------
  CONFIG_ADD_GROUP(PARASITE_GROUP, "Parasite config options");
  CONFIG_ADD_VAR(INJECT_METHOD, int, 0, "What should happen to a parasite when it gives birth?\n0 = Leave the parasite thread state untouched.\n1 = Resets the state of the calling thread (for SMT parasites, this must be 1)");
  CONFIG_ADD_VAR(INJECT_PROB_FROM_TASKS, int, 1, "Inject occurs based on probability from performing tasks - 11*numTasks");
  CONFIG_ADD_VAR(INJECT_STERILIZES_HOST, int, 0, "Infection causes host steralization");
  CONFIG_ADD_VAR(INJECT_IS_VIRULENT, int, 0, "Infection causes host steralization and takes all cpu cycles (setting this to 1 will override inject_virulence)");
  CONFIG_ADD_VAR(PARASITE_SKIP_REACTIONS, int, 1, "Parasite tasks do not get processed in the environment (1) or they do trigger reactions (0)");
  CONFIG_ADD_VAR(INJECT_IS_TASK_SPECIFIC, int, 0, "Parasites must match a task done by the host they are trying to infect");
  CONFIG_ADD_VAR(INJECT_SKIP_FIRST_TASK, int, 0, "They cannot match the first task the host is doing to infect");
  CONFIG_ADD_VAR(INJECT_DEFAULT_SUCCESS, double, 0.0, "If injection is task specific, with what probability should non-matching parasites infect the host ");
  CONFIG_ADD_VAR(PARASITE_VIRULENCE, double, -1, "The probabalistic percentage of cpu cycles allocated to the parasite instead of the host. Ensure INJECT_IS_VIRULENT is set to 0. This only works for single infection at the moment");
  CONFIG_ADD_VAR(PARASITE_MEM_SPACES, int, 1, "Parasites get their own memory spaces");
  CONFIG_ADD_VAR(PARASITE_NO_COPY_MUT, int, 0, "Parasites do not get copy mutation rates");


  // -------- CPU Archetecture
  CONFIG_ADD_GROUP(ARCHETECTURE_GROUP, "Details on how CPU should work");
  CONFIG_ADD_VAR(IO_EXPIRE, bool, 1, "Is the expiration functionality of '-expire' I/O instructions enabled?");

  
  // -------- Pprocessing of multiple, distributed populations config options --------
  CONFIG_ADD_GROUP(MP_GROUP, "Config options for multiple, distributed populations");
  CONFIG_ADD_VAR(ENABLE_MP, int, 0, "Enable multi-process Avida; 0=disabled (default),\n1=enabled.");
  CONFIG_ADD_VAR(MP_SCHEDULING_STYLE, int, 0, "Style of scheduling:\n0=non-MP aware (default)\n1=MP aware, integrated across worlds.");
	
  
  // -------- Deme config options --------
  CONFIG_ADD_GROUP(DEME_GROUP, "Demes and Germlines");
  CONFIG_ADD_VAR(NUM_DEMES, int, 1, "Number of independent groups in the population");
  CONFIG_ADD_VAR(DEMES_COMPETITION_STYLE, int, 0, "How should demes compete?\n0=Fitness proportional selection\n1=Tournament selection");
  CONFIG_ADD_VAR(DEMES_TOURNAMENT_SIZE, int, 0, "Number of demes that participate in a tournament");
  CONFIG_ADD_VAR(DEMES_OVERRIDE_FITNESS, int, 0, "Should the calculated fitness is used?\n0=yes (default)\n1=no (all fitnesses=1)");
  CONFIG_ADD_VAR(DEMES_USE_GERMLINE, int, 0, "Should demes use a distinct germline?");
  CONFIG_ADD_VAR(DEMES_PREVENT_STERILE, int, 0, "Prevent sterile demes from replicating?");
  CONFIG_ADD_VAR(DEMES_RESET_RESOURCES, int, 0, "Reset resources in demes on replication?\n0 = reset both demes \n1 = reset target deme \n2 = deme resources remain unchanged\n");
  CONFIG_ADD_VAR(DEMES_REPLICATE_SIZE, int, 1, "Number of identical organisms to create or copy from the\nsource deme to the target deme");
  CONFIG_ADD_VAR(LOG_DEMES_REPLICATE, bool, 0, "Log deme replications?");
  CONFIG_ADD_VAR(DEMES_REPLICATE_LOG_START, int, 0, "Update at which to start logging deme replications");
  CONFIG_ADD_VAR(DEMES_PROB_ORG_TRANSFER, double, 0.0, "Probablity of an organism being transferred from the\nsource deme to the target deme");
  CONFIG_ADD_VAR(DEMES_ORGANISM_SELECTION, int, 0, "How should organisms be selected for transfer from\nsource to target during deme replication?\n0 = random with replacement\n1 = sequential");
  CONFIG_ADD_VAR(DEMES_ORGANISM_PLACEMENT, int, 0, "How should organisms be placed during deme replication.\n0 = cell-array middle\n1 = deme center\n2 = random placement\n3 = sequential");
  CONFIG_ADD_VAR(DEMES_ORGANISM_FACING, int, 0, "Which direction should organisms face after deme replication.\n0 = unchanged\n1 = northwest.\n2 = random.");
  CONFIG_ADD_VAR(DEMES_MAX_AGE, int, 500, "The maximum age of a deme (in updates) to be\nused for age-based replication");
  CONFIG_ADD_VAR(DEMES_MAX_BIRTHS, int, 100, "Max number of births that can occur within a deme;\nused with birth-count replication");
  CONFIG_ADD_VAR(DEMES_MIM_EVENTS_KILLED_RATIO, double, 0.7, "Minimum ratio of events killed required for event period to be a success.");
  CONFIG_ADD_VAR(DEMES_MIM_SUCCESSFUL_EVENT_PERIODS, int, 1, "Minimum number of consecutive event periods that must be a success.");
  CONFIG_ADD_VAR(GERMLINE_COPY_MUT, double, 0.0075, "Prob. of copy mutations during germline replication");
  CONFIG_ADD_VAR(GERMLINE_INS_MUT, double, 0.05, "Prob. of insertion mutations during germline replication");
  CONFIG_ADD_VAR(GERMLINE_DEL_MUT, double, 0.05, "Prob. of deletion mutations during germline replication");
  CONFIG_ADD_VAR(DEMES_REPLICATE_CPU_CYCLES, double, 0.0, "Replicate a deme immediately after it has used this many\ncpu cycles per org in deme (0 = OFF).");
  CONFIG_ADD_VAR(DEMES_REPLICATE_TIME, double, 0.0, "Number of CPU cycles used by a deme to trigger its replication\n(normalized by number of orgs in deme and organism merit; 0 = OFF).");
  CONFIG_ADD_VAR(DEMES_REPLICATE_BIRTHS, int, 0, "Number of offspring produced by a deme to trigger its replication (0 = OFF).");
  CONFIG_ADD_VAR(DEMES_REPLICATE_ORGS, int, 0, "Number of organisms in a deme to trigger its replication (0 = OFF).");
  CONFIG_ADD_VAR(DEMES_REPLICATION_ONLY_RESETS, int, 0, "Kin selection mode.  On replication:\n0 = Nothing extra\n1 = reset deme resources\n2 = reset resources and re-inject organisms");
  CONFIG_ADD_VAR(DEMES_MIGRATION_RATE, double, 0.0, "Probability of an offspring being born in a different deme.");
  CONFIG_ADD_VAR(DEMES_MIGRATION_METHOD, int, 0, "Which demes can an org land in when it migrates?\n0 = Any other deme\n1 = Eight neighboring demes\n2 = Two adjacent demes in list\n3 = Proportional based on the number of points");
  CONFIG_ADD_VAR(DEMES_NUM_X, int, 0, "Simulated number of demes in X dimension. Used only for migration. ");
  CONFIG_ADD_VAR(DEMES_SEED_METHOD, int, 0, "Deme seeding method.\n0 = Maintain old consistency\n1 = New method using genotypes");
  CONFIG_ADD_VAR(DEMES_DIVIDE_METHOD, int, 0, "Deme divide method. Only works with DEMES_SEED_METHOD 1\n0 = Replace and target demes\n1 = Replace target deme, reset source deme to founders\n2 = Replace target deme, leave source deme unchanged");
  CONFIG_ADD_VAR(DEMES_DEFAULT_GERMLINE_PROPENSITY, double, 0.0, "Default germline propensity of organisms in deme.\nFor use with DEMES_DIVIDE_METHOD 2.");
  CONFIG_ADD_VAR(DEMES_FOUNDER_GERMLINE_PROPENSITY, double, -1.0, "Default germline propensity of founder organisms in deme.\nFor use with DEMES_DIVIDE_METHOD 2.\n <0 = OFF");
  CONFIG_ADD_VAR(DEMES_PREFER_EMPTY, int, 0, "Give empty demes preference as targets of deme replication?");
  CONFIG_ADD_VAR(DEMES_PROTECTION_POINTS, int, 0, "The number of points a deme receives for each suicide.");
  CONFIG_ADD_VAR(MIGRATION_RATE, double, 0.0, "Uniform probability of offspring migrating to a new deme.");  
  CONFIG_ADD_VAR(DEMES_TRACK_SHANNON_INFO, int, 0, "Enable shannon mutual information tracking for demes.");
  
  
  // -------- Reversion config options --------
  CONFIG_ADD_GROUP(REVERSION_GROUP, "Mutation Reversion\nMost of these slow down avida a lot, and should be set to 0.0 normally.");
  CONFIG_ADD_VAR(REVERT_FATAL, double, 0.0, "Prob of lethal mutations being reverted on birth");
  CONFIG_ADD_VAR(REVERT_DETRIMENTAL, double, 0.0, "Prob of harmful (but non-lethal) mutations reverting on birth");
  CONFIG_ADD_VAR(REVERT_NEUTRAL, double, 0.0, "Prob of neutral mutations being reverted on birth");
  CONFIG_ADD_VAR(REVERT_BENEFICIAL, double, 0.0, "Prob of beneficial mutations being reverted on birth");
  CONFIG_ADD_VAR(REVERT_TASKLOSS, double, 0.0, "Prob of mutations that cause task loss (without any gains) being reverted");
  CONFIG_ADD_VAR(STERILIZE_FATAL, double, 0.0, "Prob of lethal mutations steralizing an offspring (typically no effect!)");
  CONFIG_ADD_VAR(STERILIZE_DETRIMENTAL, double, 0.0, "Prob of harmful (but non-lethal) mutations steralizing an offspring");
  CONFIG_ADD_VAR(STERILIZE_NEUTRAL, double, 0.0, "Prob of neutral mutations steralizing an offspring");
  CONFIG_ADD_VAR(STERILIZE_BENEFICIAL, double, 0.0, "Prob of beneficial mutations steralizing an offspring");
  CONFIG_ADD_VAR(STERILIZE_TASKLOSS, double, 0.0, "Prob of mutations causing task loss steralizing an offspring");
  CONFIG_ADD_VAR(STERILIZE_UNSTABLE, int, 0, "Should genotypes that cannot replicate perfectly not be allowed to replicate?");
  CONFIG_ADD_ALIAS(FAIL_IMPLICIT);
  CONFIG_ADD_VAR(NEUTRAL_MAX,double, 0.0, "Percent benifical change from parent fitness to be considered neutral.");
  CONFIG_ADD_VAR(NEUTRAL_MIN,double, 0.0, "Percent deleterious change from parent fitness to be considered neutral.");

  
  // -------- Time Slicing config options --------
  CONFIG_ADD_GROUP(TIME_GROUP, "Time Slicing");
  CONFIG_ADD_VAR(AVE_TIME_SLICE, int, 30, "Average number of CPU-cycles per org per update");
  CONFIG_ADD_VAR(SLICING_METHOD, int, 1, "0 = CONSTANT: all organisms receive equal number of CPU cycles\n1 = PROBABILISTIC: CPU cycles distributed randomly, proportional to merit.\n2 = INTEGRATED: CPU cycles given out deterministicly, proportional to merit\n3 = DEME_PROBABALISTIC: Demes receive fixed number of CPU cycles, awarded probabalistically to members\n4 = CROSS_DEME_PROBABALISTIC: Demes receive CPU cycles proportional to living population size, awarded probabalistically to members\n5 = CONSTANT BURST: all organisms receive equal number of CPU cycles, in SLICING_BURST_SIZE chunks");
  CONFIG_ADD_VAR(SLICING_BURST_SIZE, int, 1, "Sets the scheduler burst size for SLICING_METHOD 5.");
  CONFIG_ADD_VAR(BASE_MERIT_METHOD, int, 4, "How should merit be initialized?\n0 = Constant (merit independent of size)\n1 = Merit proportional to copied size\n2 = Merit prop. to executed size\n3 = Merit prop. to full size\n4 = Merit prop. to min of executed or copied size\n5 = Merit prop. to sqrt of the minimum size\n6 = Merit prop. to num times MERIT_BONUS_INST is in genome.");
  CONFIG_ADD_VAR(BASE_CONST_MERIT, int, 100, "Base merit valse for BASE_MERIT_METHOD 0");
  CONFIG_ADD_VAR(MERIT_BONUS_INST, int, 0, "Instruction ID to count for BASE_MERIT_METHOD 6"); 
  CONFIG_ADD_VAR(MERIT_BONUS_EFFECT, int, 0, "Amount of merit earn per instruction for BASE_MERIT_METHOD 6 (-1 = penalty, 0 = no effect)"); 
  CONFIG_ADD_VAR(FITNESS_VALLEY, int, 0, "in BASE_MERIT_METHOD 6, this creates valleys from\nFITNESS_VALLEY_START to FITNESS_VALLEY_STOP\n(0 = off, 1 = on)"); 
  CONFIG_ADD_VAR(FITNESS_VALLEY_START, int, 0, "if FITNESS_VALLEY = 1, orgs with num_key_instructions\nfrom FITNESS_VALLEY_START to FITNESS_VALLEY_STOP\nget fitness 1 (lowest)"); 
  CONFIG_ADD_VAR(FITNESS_VALLEY_STOP, int, 0, "if FITNESS_VALLEY = 1, orgs with num_key_instructions\nfrom FITNESS_VALLEY_START to FITNESS_VALLEY_STOP\nget fitness 1 (lowest)"); 
  CONFIG_ADD_VAR(DEFAULT_BONUS, double, 1.0, "Initial bonus before any tasks");
  CONFIG_ADD_VAR(MERIT_DEFAULT_BONUS, int, 0, "Instead of inheriting bonus from parent, use this value instead (0 = off)"); 
  CONFIG_ADD_VAR(MERIT_INC_APPLY_IMMEDIATE, bool, 0, "Should merit increases (above current) be applied immediately, or delayed until divide?");
  CONFIG_ADD_VAR(TASK_REFRACTORY_PERIOD, double, 0.0, "Number of updates after taske until regain full value");
  CONFIG_ADD_VAR(FITNESS_METHOD, int, 0, "0 = default, 1 = sigmoidal, ");
  CONFIG_ADD_VAR(FITNESS_COEFF_1, double, 1.0, "1st FITNESS_METHOD parameter");
  CONFIG_ADD_VAR(FITNESS_COEFF_2, double, 1.0, "2nd FITNESS_METHOD parameter");
  CONFIG_ADD_VAR(MAX_CPU_THREADS, int, 1, "Maximum number of Threads a CPU can spawn");
  CONFIG_ADD_VAR(THREAD_SLICING_METHOD, int, 0, "Formula for allocating CPU cycles across threads in an organism\n  (num_threads-1) * THREAD_SLICING_METHOD + 1\n0 = One thread executed per time slice.\n1 = All threads executed each time slice.\n");
  CONFIG_ADD_VAR(NO_CPU_CYCLE_TIME, int, 0, "Don't count each CPU cycle as part of gestation time\n");
  CONFIG_ADD_VAR(MAX_LABEL_EXE_SIZE, int, 1, "Max nops marked as executed when labels are used");
  CONFIG_ADD_VAR(PRECALC_PHENOTYPE, int, 0, "0 = Disabled\n 1 = Assign precalculated merit at birth (unlimited resources only)\n 2 = Assign precalculated gestation time\n 3 = Assign precalculated merit AND gestation time.\n 4 = Assign last instruction counts \n 5 = Assign last instruction counts and merit\n 6 = Assign last instruction counts and gestation time \n 7 = Assign everything currently supported\nFitness will be evaluated for organism based on these settings.");
  CONFIG_ADD_VAR(FASTFORWARD_UPDATES, int, 0, "Fast-forward if the average generation has not changed in this many updates. (0 = off)");
  CONFIG_ADD_VAR(FASTFORWARD_NUM_ORGS, int, 0, "Fast-forward if population is equal to this");
  CONFIG_ADD_VAR(GENOTYPE_PHENPLAST_CALC, int, 100, "Number of times to test a genotype's\nplasticity during runtime.");
  

  // -------- Altruism config options --------
  CONFIG_ADD_GROUP(ALTRUISM_GROUP, "Altrusim");
  CONFIG_ADD_VAR(MERIT_GIVEN, double, 0.0, "Fraction of merit donated with 'donate' command");
  CONFIG_ADD_VAR(MERIT_RECEIVED, double, 0.0, "Multiplier of merit given with 'donate' command"); 
  CONFIG_ADD_VAR(MAX_DONATE_KIN_DIST, int, -1, "Limit on distance of relation for donate; -1=no max");
  CONFIG_ADD_VAR(MAX_DONATE_EDIT_DIST, int, -1, "Limit on genetic (edit) distance for donate; -1=no max");
  CONFIG_ADD_VAR(MIN_GB_DONATE_THRESHOLD, int, -1, "threshold green beard donates only to orgs above this\ndonation attempt threshold; -1=no thresh");
  CONFIG_ADD_VAR(DONATE_THRESH_QUANTA, int, 10, "The size of steps between quanta donate thresholds");
  CONFIG_ADD_VAR(MAX_DONATES, int, 1000000, "Limit on number of donates organisms are allowed.");


  // -------- Geneology config options --------
  CONFIG_ADD_GROUP(GENEOLOGY_GROUP, "Geneology");
  CONFIG_ADD_VAR(TRACK_MAIN_LINEAGE, int, 1, "Keep all ancestors of the active population?\n0=no, 1=yes, 2=yes,w/sexual population");
  CONFIG_ADD_VAR(THRESHOLD, int, 3, "Number of organisms in a genotype needed for it\n  to be considered viable.");
  CONFIG_ADD_VAR(GENOTYPE_PRINT, int, 0, "0/1 (off/on) Print out all threshold genotypes?");
  CONFIG_ADD_VAR(GENOTYPE_PRINT_DOM, int, 0, "Print out a genotype if it stays dominant for\n  this many updates. (0 = off)");
  CONFIG_ADD_VAR(TEST_CPU_TIME_MOD, int, 20, "Time allocated in test CPUs (multiple of length)");
  CONFIG_ADD_VAR(TRACK_PARENT_DIST, bool, 0, "Track parent distance during run. This is unnecessary when track main lineage is on.");
  

  // -------- Log File config options --------
  CONFIG_ADD_GROUP(LOG_GROUP, "Log Files");
  CONFIG_ADD_VAR(LOG_GENOTYPES, int, 0, "0 = off, 1 = print ALL, 2 = print threshold ONLY.");
  CONFIG_ADD_VAR(LOG_THRESHOLD, bool, 0, "0/1 (off/on) toggle to print file.");
  CONFIG_ADD_VAR(LOG_LINEAGES, bool, 0, "Track lineages over time?\nWARNING: Can slow Avida a lot!");
  CONFIG_ADD_VAR(LINEAGE_CREATION_METHOD, int, 0, "Requires LOG_LINEAGES = 1\n0 = Manual creation (on inject)\n1 = when a child's (potential) fitness is higher than that of its parent.\n2 = when a child's (potential) fitness is higher than max in population.\n3 = when a child's (potential) fitness is higher than max in dom. lineage\n  *and* the child is in the dominant lineage, or (2)\n4 = when a child's (potential) fitness is higher than max in dom. lineage\n  (and that of its own lineage)\n5 = same as child's (potential) fitness is higher than that of the\n  currently dominant organism, and also than that of any organism\n      currently in the same lineage.\n6 = when a child's (potential) fitness is higher than any organism\n  currently in the same lineage.\n7 = when a child's (potential) fitness is higher than that of any\n  organism in its line of descent");
  CONFIG_ADD_VAR(TRACE_EXECUTION, bool, 0, "Trace the execution of all organisms in the population (WARNING: SLOW!)");
  

  // -------- Organism Network config options --------
  CONFIG_ADD_GROUP(ORGANISM_NETWORK_GROUP, "Organism Network Communication");
  CONFIG_ADD_VAR(NET_ENABLED, bool, 0, "Enable Network Communication Support");
  CONFIG_ADD_VAR(NET_DROP_PROB, double, 0.0, "Message drop rate");
  CONFIG_ADD_VAR(NET_MUT_PROB, double, 0.0, "Message corruption probability");
  CONFIG_ADD_VAR(NET_MUT_TYPE, int, 0, "Type of message corruption.  0 = Random Single Bit, 1 = Always Flip Last");
  CONFIG_ADD_VAR(NET_STYLE, int, 0, "Communication Style.  0 = Random Next, 1 = Receiver Facing");
  CONFIG_ADD_VAR(NET_LOG_MESSAGES, int, 0, "Whether all messages are logged; 0=false (default), 1=true.");


  // -------- Organism Messaging config options --------
  CONFIG_ADD_GROUP(ORGANISM_MESSAGING_GROUP, "Organism Message-Based Communication");
  CONFIG_ADD_VAR(MESSAGE_SEND_BUFFER_SIZE, int, 1, "Size of message send buffer (stores messages that were sent)\nTASKS NOT CHECKED ON 0!\n-1=inf, default=1.");
  CONFIG_ADD_VAR(MESSAGE_RECV_BUFFER_SIZE, int, 8, "Size of message receive buffer (stores messages that are received); -1=inf, default=8.");
  CONFIG_ADD_VAR(MESSAGE_RECV_BUFFER_BEHAVIOR, bool, 0, "Behavior of message receive buffer; 0=drop oldest (default), 1=drop incoming");
  CONFIG_ADD_VAR(ACTIVE_MESSAGES_ENABLED, int, 0, "Enable active messages. \n0 = off\n2 = message creates parallel thread");
  

  // -------- Buying and Selling config options --------
  CONFIG_ADD_GROUP(BUY_SELL_GROUP, "Buying and Selling Parameters");
  CONFIG_ADD_VAR(SAVE_RECEIVED, bool, 0, "Enable storage of all inputs bought from other orgs");
  CONFIG_ADD_VAR(BUY_PRICE, int, 0, "price offered by organisms attempting to buy");
  CONFIG_ADD_VAR(SELL_PRICE, int, 0, "price offered by organisms attempting to sell");
  

  // -------- Resource Hoarding (Collect) config options --------
  CONFIG_ADD_GROUP(HOARD_RESOURCE_GROUP, "Resource Hoarding Parameters");
  CONFIG_ADD_VAR(USE_RESOURCE_BINS, bool, 0, "Enable resource bin use.  This serves as a guard on most resource hoarding code.");
  CONFIG_ADD_VAR(ABSORB_RESOURCE_FRACTION, double, .0025, "Fraction of available environmental resource an organism absorbs.");
  CONFIG_ADD_VAR(MULTI_ABSORB_TYPE, int, 0, "What to do if a collect instruction is called on a range of resources.\n 0 = absorb a random resource in the range\n 1 = absorb the first resource in the range\n 2 = absorb the last resource in the range\n 3 = absorb ABSORB_RESOURCE_FRACTION / (# of resources in range) of each resource in the range");
  CONFIG_ADD_VAR(MAX_TOTAL_STORED, double, -1, "Maximum total amount of all resources an organism can store.\n <0 = no maximum");
  CONFIG_ADD_VAR(USE_STORED_FRACTION, double, 1.0, "The fraction of stored resource to use.");
  CONFIG_ADD_VAR(ENV_FRACTION_THRESHOLD, double, 1.0, "The fraction of available environmental resource to compare available stored resource to when deciding whether to use stored resource.");
  CONFIG_ADD_VAR(RETURN_STORED_ON_DEATH, bool, 1, "Return an organism's stored resources to the world when it dies?");
  CONFIG_ADD_VAR(SPLIT_ON_DIVIDE, bool, 1, "Split mother cell's resources between two daughter cells on division?");
  CONFIG_ADD_VAR(COLLECT_SPECIFIC_RESOURCE, int, 0, "Resource to be collected by the \"collect-specific\" instruction.");
  CONFIG_ADD_VAR(RESOURCE_GIVEN_ON_INJECT, double, 0.0, "Units of collect-specific resources given on inject.");  
  CONFIG_ADD_VAR(RESOURCE_GIVEN_AT_BIRTH, double, 0.0, "Units of collect-specific resources given to offspring upon birth (will be added to SPLIT_ON_DIVIDE amount for collect-specific resource if both enabled.");  
  
  
  // -------- Analyze config options --------
  CONFIG_ADD_GROUP(ANALYZE_GROUP, "Analysis Settings");
  CONFIG_ADD_VAR(MAX_CONCURRENCY, int, -1, "Maximum number of analyze threads, -1 == use all available.");
  CONFIG_ADD_VAR(ANALYZE_OPTION_1, cString, "", "String variable accessible from analysis scripts");
  CONFIG_ADD_VAR(ANALYZE_OPTION_2, cString, "", "String variable accessible from analysis scripts");
  

  // -------- Energy Model config options --------
  CONFIG_ADD_GROUP(ENERGY_GROUP, "Energy Settings");
  CONFIG_ADD_VAR(ENERGY_ENABLED, bool, 0, "Enable Energy Model. 0/1 (off/on)");
  CONFIG_ADD_VAR(ENERGY_GIVEN_ON_INJECT, double, 0.0, "Energy given to organism upon injection.");
  CONFIG_ADD_VAR(ENERGY_GIVEN_AT_BIRTH, double, 0.0, "Energy given to offspring upon birth.");
  CONFIG_ADD_VAR(FRAC_PARENT_ENERGY_GIVEN_TO_ORG_AT_BIRTH, double, 0.5, "Fraction of parent's energy given to offspring organism.");
  CONFIG_ADD_VAR(FRAC_PARENT_ENERGY_GIVEN_TO_DEME_AT_BIRTH, double, 0.5, "Fraction of parent's energy given to offspring deme.");
  CONFIG_ADD_VAR(FRAC_ENERGY_DECAY_AT_ORG_BIRTH, double, 0.0, "Fraction of energy lost due to decay during organism reproduction.");
  CONFIG_ADD_VAR(FRAC_ENERGY_DECAY_AT_DEME_BIRTH, double, 0.0, "Fraction of energy lost due to decay during deme reproduction.");
  CONFIG_ADD_VAR(NUM_CYCLES_EXC_BEFORE_0_ENERGY, int, 0, "Number of virtual CPU cycles executed before energy is exhausted.");
  CONFIG_ADD_VAR(ENERGY_CAP, double, -1.0, "Maximum amount of energy that can be stored in an organism.  -1 = no max");  // TODO - is this done?
  CONFIG_ADD_VAR(APPLY_ENERGY_METHOD, int, 0, "When should rewarded energy be applied to current energy?\n0 = on divide\n1 = on completion of task\n2 = on sleep");  
  CONFIG_ADD_VAR(FIX_METABOLIC_RATE, double, -1.0, "Fix organism metobolic rate to value.  This value is static.  Feature disabled by default (value == -1)");
  CONFIG_ADD_VAR(FRAC_ENERGY_TRANSFER, double, 0.0, "Fraction of replaced organism's energy take by new resident");
  CONFIG_ADD_VAR(LOG_SLEEP_TIMES, bool, 0, "Log sleep start and end times. 0/1 (off/on)\nWARNING: may use lots of memory.");
  CONFIG_ADD_VAR(FRAC_ENERGY_RELINQUISH, double, 1.0, "Fraction of organisms energy to relinquish");
  CONFIG_ADD_VAR(ENERGY_PASSED_ON_DEME_REPLICATION_METHOD, int, 0, "Who get energy passed from a parent deme\n0 = Energy divided among organisms injected to offspring deme\n1 = Energy divided among cells in offspring deme");
  CONFIG_ADD_VAR(INHERIT_EXE_RATE, int, 0, "Inherit energy rate from parent? 0=no  1=yes");
  CONFIG_ADD_VAR(ATTACK_DECAY_RATE, double, 0.0, "Percent of cell's energy decayed by attack");
  CONFIG_ADD_VAR(ENERGY_THRESH_LOW, double, .33, "Threshold percent below which energy level is considered low.  Requires ENERGY_CAP.");
  CONFIG_ADD_VAR(ENERGY_THRESH_HIGH, double, .75, "Threshold percent above which energy level is considered high.  Requires ENERGY_CAP.");
  CONFIG_ADD_VAR(ENERGY_COMPARISON_EPSILON, double, 0.0, "Percent difference (relative to executing organism) required in energy level comparisons");
  CONFIG_ADD_VAR(ENERGY_REQUEST_RADIUS, int, 1, "Radius of broadcast energy request messages.");
	

  // -------- Energy Sharing config options --------
  CONFIG_ADD_GROUP(ENERGY_SHARING_GROUP, "Energy Sharing Settings");
  CONFIG_ADD_VAR(ENERGY_SHARING_METHOD, int, 0, "Method for sharing energy.  0=receiver must actively receive/request, 1=energy pushed on receiver");
  CONFIG_ADD_VAR(ENERGY_SHARING_PCT, double, 0.0, "Percent of energy to share");
  CONFIG_ADD_VAR(ENERGY_SHARING_INCREMENT, double, 0.01, "Amount to change percent energy shared");
  CONFIG_ADD_VAR(RESOURCE_SHARING_LOSS, double, 0.0, "Fraction of shared resource lost in transfer");
  CONFIG_ADD_VAR(ENERGY_SHARING_UPDATE_METABOLIC, bool, 0, "0/1 (off/on) - Whether to update an organism's metabolic rate on donate or reception/application of energy");
  CONFIG_ADD_VAR(LOG_ENERGY_SHARING, bool, 0, "Whether or not to log energy shares.  0/1 (off/on)");
  

  // -------- Second Pass Metrics config options --------
  CONFIG_ADD_GROUP(SECOND_PASS_GROUP, "Tracking metrics known after the running experiment previously");
  CONFIG_ADD_VAR(TRACK_CCLADES, int, 0, "Enable tracking of coalescence clades");
  CONFIG_ADD_VAR(TRACK_CCLADES_IDS, cString, "coalescence.ids", "File storing coalescence IDs");
  

  // -------- Gene Expression CPU config options --------
  CONFIG_ADD_GROUP(GX_GROUP, "Gene Expression CPU Settings");
  CONFIG_ADD_VAR(MAX_PROGRAMIDS, int, 16, "Maximum number of programids an organism can create.");
  CONFIG_ADD_VAR(MAX_PROGRAMID_AGE, int, 2000, "Max number of CPU cycles a programid executes before it is removed.");
  CONFIG_ADD_VAR(IMPLICIT_GENE_EXPRESSION, int, 0, "Create executable programids from the genome without explicit allocation and copying?");
  CONFIG_ADD_VAR(IMPLICIT_BG_PROMOTER_RATE, double, 0.0, "Relative rate of non-promoter sites creating programids.");
  CONFIG_ADD_VAR(IMPLICIT_TURNOVER_RATE, double, 0.0, "Number of programids recycled per CPU cycle. 0 = OFF");
  CONFIG_ADD_VAR(IMPLICIT_MAX_PROGRAMID_LENGTH, int, 0, "Creation of an executable programid terminates after this many instructions. 0 = disabled");
  

  // -------- Promoters config options --------
  CONFIG_ADD_GROUP(PROMOTER_GROUP, "Promoters");
  CONFIG_ADD_VAR(PROMOTERS_ENABLED, int, 0, "Use the promoter/terminator execution scheme.\nCertain instructions must also be included.");
  CONFIG_ADD_VAR(PROMOTER_INST_MAX, int, 0, "Maximum number of instructions to execute before terminating. 0 = off");
  CONFIG_ADD_VAR(PROMOTER_PROCESSIVITY, double, 1.0, "Chance of not terminating after each cpu cycle.");
  CONFIG_ADD_VAR(PROMOTER_PROCESSIVITY_INST, double, 1.0, "Chance of not terminating after each instruction.");
  CONFIG_ADD_VAR(PROMOTER_TO_REGISTER, int, 0, "Place a promoter's base bit code in register BX when starting execution from it?");
  CONFIG_ADD_VAR(TERMINATION_RESETS, int, 0, "Does termination reset the thread's state?");
  CONFIG_ADD_VAR(NO_ACTIVE_PROMOTER_EFFECT, int, 0, "What happens when there are no active promoters?\n0 = Start execution at the beginning of the genome.\n1 = Kill the organism.\n2 = Stop the organism from executing any further instructions.");
  CONFIG_ADD_VAR(PROMOTER_CODE_SIZE, int, 24, "Size of a promoter code in bits. (Maximum value is 32)");
  CONFIG_ADD_VAR(PROMOTER_EXE_LENGTH, int, 3, "Length of promoter windows used to determine execution.");
  CONFIG_ADD_VAR(PROMOTER_EXE_THRESHOLD, int, 2, "Minimum number of bits that must be set in a promoter window to allow execution.");
  CONFIG_ADD_VAR(INST_CODE_LENGTH, int, 3, "Instruction binary code length (number of bits)");
  CONFIG_ADD_VAR(INST_CODE_DEFAULT_TYPE, int, 0, "Default value of instruction binary code value.\n0 = All zeros\n1 = Based off the instruction number");
  CONFIG_ADD_VAR(CONSTITUTIVE_REGULATION, int, 0, "Sense a new regulation value before each CPU cycle?");
  

  // -------- Output Colors config options --------
  CONFIG_ADD_GROUP(COLORS_GROUP, "Output colors for when data files are printed in HTML mode.\nThere are two sets of these; the first are for lineages,\nand the second are for mutation tests.");
  CONFIG_ADD_VAR(COLOR_DIFF, cString, "CCCCFF", "Color to flag stat that has changed since parent.");
  CONFIG_ADD_VAR(COLOR_SAME, cString, "FFFFFF", "Color to flag stat that has NOT changed since parent.");
  CONFIG_ADD_VAR(COLOR_NEG2, cString, "FF0000", "Color to flag stat that is significantly worse than parent.");
  CONFIG_ADD_VAR(COLOR_NEG1, cString, "FFCCCC", "Color to flag stat that is minorly worse than parent.");
  CONFIG_ADD_VAR(COLOR_POS1, cString, "CCFFCC", "Color to flag stat that is minorly better than parent.");
  CONFIG_ADD_VAR(COLOR_POS2, cString, "00FF00", "Color to flag stat that is significantly better than parent.");
  
  CONFIG_ADD_VAR(COLOR_MUT_POS,    cString, "00FF00", "Color to flag stat that has changed since parent.");
  CONFIG_ADD_VAR(COLOR_MUT_NEUT,   cString, "FFFFFF", "Color to flag stat that has changed since parent.");
  CONFIG_ADD_VAR(COLOR_MUT_NEG,    cString, "FFFF00", "Color to flag stat that has changed since parent.");
  CONFIG_ADD_VAR(COLOR_MUT_LETHAL, cString, "FF0000", "Color to flag stat that has changed since parent.");
  
  
  // -------- Synchronization config options --------
  CONFIG_ADD_GROUP(MOVEMENT_GROUP, "Movement Features Settings");
  CONFIG_ADD_VAR(MOVEMENT_COLLISIONS_LETHAL, int, 0, "Are collisions during movement lethal? (0=no, use swap; 1=yes, use collision selection type; 2=no, but movement fails)"); 
  CONFIG_ADD_VAR(MOVEMENT_COLLISIONS_SELECTION_TYPE, int, 0, "0 = 50% chance\n1 = binned vitality based");
  CONFIG_ADD_VAR(VITALITY_BIN_EXTREMES, double, 1.0, "vitality multiplier for extremes (> 1 stddev from the mean population age)");
  CONFIG_ADD_VAR(VITALITY_BIN_CENTER, double, 10.0, "vitality multiplier for center bin (with 1 stddev of the mean population age)");
  
  
  // -------- Pheromone config options --------
  CONFIG_ADD_GROUP(PHEROMONE_GROUP, "Pheromone Settings");
  CONFIG_ADD_VAR(PHEROMONE_ENABLED, bool, 0, "Enable pheromone usage. 0/1 (off/on)");
  CONFIG_ADD_VAR(PHEROMONE_AMOUNT, double, 1.0, "Amount of pheromone to add per drop");
  CONFIG_ADD_VAR(PHEROMONE_DROP_MODE, int, 0, "Where to drop pheromone\n0 = Half amount at src, half at dest\n1 = All at source\n2 = All at dest");
  CONFIG_ADD_VAR(EXPLOIT_EXPLORE_PROB, double, 0.00, "Probability of random exploration\ninstead of pheromone trail following");
  CONFIG_ADD_VAR(LOG_PHEROMONE, bool, 0, "Log pheromone drops.  0/1 (off/on)");
  CONFIG_ADD_VAR(PHEROMONE_LOG_START, int, 0, "Update at which to start logging pheromone drops");
  CONFIG_ADD_VAR(EXPLOIT_LOG_START, int, 0, "Update at which to start logging exploit moves");
  CONFIG_ADD_VAR(EXPLORE_LOG_START, int, 0, "Update at which to start logging explore moves");
  CONFIG_ADD_VAR(MOVETARGET_LOG_START, int, 0, "Update at which to start logging movetarget moves");
  CONFIG_ADD_VAR(LOG_INJECT, bool, 0, "Log injection of organisms.  0/1 (off/on)");
  CONFIG_ADD_VAR(INJECT_LOG_START, int, 0, "Update at which to start logging injection of\norganisms");
  

  // -------- Synchronization config options --------
  CONFIG_ADD_GROUP(SYNCHRONIZATION_GROUP, "Synchronization settings");
  CONFIG_ADD_VAR(SYNC_FITNESS_WINDOW, int, 100, "Number of updates over which to calculate fitness (default=100).");
  CONFIG_ADD_VAR(SYNC_FLASH_LOSSRATE, double, 0.0, "P() to lose a flash send (0.0==off).");
  CONFIG_ADD_VAR(SYNC_TEST_FLASH_ARRIVAL, int, -1, "CPU cycle at which an organism will receive a flash (off=-1, default=-1, analyze mode only.)");	
	

  // -------- Consensus config options --------
  CONFIG_ADD_GROUP(CONSENSUS_GROUP, "Consensus settings");	
  CONFIG_ADD_VAR(CONSENSUS_HOLD_TIME, int, 1, "Number of updates that consensus must be held for.");
  

  // -------- Instruction Set Definition --------
  // Setup technique to define the instruction set in avida.cfg file.
  CONFIG_ADD_CUSTOM_FORMAT(INSTSETS, "Instruction Set Definition");
  CONFIG_ADD_FORMAT_VAR(INSTSET, "Define an instruction set (must supply name:hw_type=$hardware_type)");
  CONFIG_ADD_FORMAT_VAR(INST, "Instruction entry in the instruction set");
	
	
  // -------- Reputation config options --------
  CONFIG_ADD_GROUP(REPUTATION_GROUP, "Reputation Settings");
  CONFIG_ADD_VAR(RAW_MATERIAL_AMOUNT, int, 100, "Number of raw materials an organism starts with");
  CONFIG_ADD_VAR(AUTO_REPUTATION, int, 0, "Is an organism's reputation automatically computed based on its donations\n0=no\n1=increment for each donation + standing\n2=+1 for donations given -1 for donations received\n3=1 for donors -1 for recivers who have not donated\n4=+1 for donors\n5=+1 for donors during task check");
  CONFIG_ADD_VAR(ALT_BENEFIT, int, 1.00, "Number multiplied by the number of raw materials received from another organism to compute reward");
  CONFIG_ADD_VAR(ALT_COST, int, 1.00, "Number multiplied by the number of your raw materials");
  CONFIG_ADD_VAR(ROTATE_ON_DONATE, int, 0, "Rotate an organism to face its donor 0/1 (off/on)");
  CONFIG_ADD_VAR(REPUTATION_REWARD, int, 0, "Reward an organism for having a good reputation");
  CONFIG_ADD_VAR(DONATION_FAILURE_PERCENT, int, 0, "Percentage of times that a donation fails");
  CONFIG_ADD_VAR(RANDOMIZE_RAW_MATERIAL_AMOUNT, int, 0, "Should all the organisms receive the same amount 0/1 (off/on)");
  CONFIG_ADD_VAR(DONATION_RESTRICTIONS, int, 0, "0=none\n1=inter-species only\n2=different tag only");
  CONFIG_ADD_VAR(INHERIT_REPUTATION, int, 0, "0=reputations are not inherited\n1=reputations are inherited\n2=tags are inherited");
  CONFIG_ADD_VAR(SPECIALISTS, int, 0, "0=generalists allowed\n1=only specialists");
  CONFIG_ADD_VAR(STRING_AMOUNT_CAP, int, -1, "-1=no cap on string amounts\n#=CAP");
  CONFIG_ADD_VAR(MATCH_ALREADY_PRODUCED, int, 0, "0=off\n1=on");
	

  // -------- Group formation config options --------
  CONFIG_ADD_GROUP(GROUP_FORMATION_GROUP, "Group Formation Settings");
  CONFIG_ADD_VAR(USE_FORM_GROUPS, int, 0, "Enable organisms to form groups. 0=off,\n 1=on no restrict,\n 2=on restrict to defined");
  CONFIG_ADD_VAR(DEFAULT_GROUP, int, -1, "Default group to assign to organisms not asserting a group membership (-1 indicates disabled)");
	

  // -------- Deme network config options --------
  CONFIG_ADD_GROUP(DEME_NETWORK_GROUP, "Deme network settings");
  CONFIG_ADD_VAR(DEME_NETWORK_TYPE, int, 0, "0=topology, structure of network determines fitness.");
  CONFIG_ADD_VAR(DEME_NETWORK_REQUIRES_CONNECTEDNESS, int, 1, "Whether the deme's network must be connected before an actual fitness is calculated.");
  CONFIG_ADD_VAR(DEME_NETWORK_TOPOLOGY_FITNESS, int, 0, "Network measure used to determine fitness; see cDemeTopologyNetwork.h.");
  CONFIG_ADD_VAR(DEME_NETWORK_LINK_DECAY, int, 0, "Number of updates after which a link decays; 0=no decay (default).");
	CONFIG_ADD_VAR(DEME_NETWORK_REMOVE_NODE_ON_DEATH, int, 0, "Whether death of an organism in\nthe deme removes its links;\n0=no (default);\n1=yes.");

	
  // -------- Horizontal Gene Transfer (HGT) config options --------
  CONFIG_ADD_GROUP(HGT_GROUP, "Horizontal gene transfer settings");
  CONFIG_ADD_VAR(ENABLE_HGT, int, 0, "Whether HGT is enabled; 0=false (default),\n1=true.");
  CONFIG_ADD_VAR(HGT_SOURCE, int, 0, "Source of HGT fragments; 0=dead organisms (default),\n1=parent.");
  CONFIG_ADD_VAR(HGT_FRAGMENT_SELECTION, int, 0, "Method used to select fragments for HGT mutation; 0=random (default),\n1=trimmed selection\n2=random placement.");
  CONFIG_ADD_VAR(HGT_FRAGMENT_SIZE_MEAN, double, 10, "Mean size of fragments (default=10).");
  CONFIG_ADD_VAR(HGT_FRAGMENT_SIZE_VARIANCE, double, 2, "Variance of fragments (default=2).");
  CONFIG_ADD_VAR(HGT_MAX_FRAGMENTS_PER_CELL, int, 100, "Max. allowed number of fragments per cell (default=100).");
  CONFIG_ADD_VAR(HGT_DIFFUSION_METHOD, int, 0, "Method to use for diffusion of genome fragments; 0=none (default).");
  CONFIG_ADD_VAR(HGT_COMPETENCE_P, double, 0.0, "Probability that an HGT 'natural competence' mutation will occur on divide (default=0.0).");
  CONFIG_ADD_VAR(HGT_INSERTION_MUT_P, double, 0.0, "Probability that an HGT mutation will result in an insertion (default=0.0).");
  CONFIG_ADD_VAR(HGT_CONJUGATION_METHOD, int, 0, "Method used to select the receiver and/or donor of an HGT conjugation;\n0=random from neighborhood (default);\n1=faced.");
  CONFIG_ADD_VAR(HGT_CONJUGATION_P, double, 0.0, "Probability that an HGT conjugation mutation will occur on divide (default=0.0).");
  CONFIG_ADD_VAR(HGT_FRAGMENT_XFORM, int, 0, "Transformation to apply to each fragment prior to incorporation into offspring's genome; 0=none (default),\n1=random shuffle,\n2=replace with random instructions.");


  // -------- Resource Dependent Instructions config options --------
  CONFIG_ADD_GROUP(INST_RES_GROUP, "Resource-Dependent Instructions Settings");
  CONFIG_ADD_VAR(INST_RES, cString, "", "Resource upon which the execution of certain instruction depends");
  CONFIG_ADD_VAR(INST_RES_FLOOR, double, 0.0, "Assumed lower level of resource in environment.  Used for probability dist.");
  CONFIG_ADD_VAR(INST_RES_CEIL, double, 0.0, "Assumed upper level of resource in environment.  Used for probability dist.");
	

  // -------- Opinion-setting config options --------
  CONFIG_ADD_GROUP(OPINION_GROUP, "Organism opinion settings");
  CONFIG_ADD_VAR(OPINION_BUFFER_SIZE, int, 1, "Size of the opinion buffer (stores opinions set over the organism's lifetime); -1=inf, default=1, cannot be 0.");
	

  // -------- Alarm config options --------
  CONFIG_ADD_GROUP(ALARM_GROUP, "Alarm Settings");
  CONFIG_ADD_VAR(BCAST_HOPS, int, 1, "Number of hops to broadcast an alarm");
  CONFIG_ADD_VAR(ALARM_SELF, bool, 0, "Does sending an alarm move sender IP to alarm label?\n0=no\n1=yes");

	
  //--------- Division of Labor --------------------
  CONFIG_ADD_GROUP(DIVISION_OF_LABOR_GROUP, "Division of Labor settings");	
  CONFIG_ADD_VAR(AGE_POLY_TRACKING, bool, 0, "Print data for an age-task histogram");
  CONFIG_ADD_VAR(REACTION_THRESH, int, 0, "The number of times the deme must perform each reaction in order to replicate");
  CONFIG_ADD_VAR(TASK_SWITCH_PENALTY, int, 0, "Cost of task switching in cycles");
  CONFIG_ADD_VAR(TASK_SWITCH_PENALTY_TYPE, int, 0, "Type of task switch cost: (0) none (1) learning, (2) retooling or context, (3) centrifuge");
  CONFIG_ADD_VAR(RES_FOR_DEME_REP, int, 0, "The amount of resources that must be consumed prior to automatic deme replication");


  // -------- DEPRECATED ---------
  CONFIG_ADD_GROUP(DEPRECATED_GROUP, "DEPRECATED (New functionality listed in comments)");
  CONFIG_ADD_VAR(ANALYZE_MODE, int, 0, "0 = Disabled\n1 = Enabled\n2 = Interactive\nDEPRICATED: use command line options -a[nalyze] or -i[nteractive])");
  CONFIG_ADD_VAR(REPRO_METHOD, int, 1, "Replace existing organism: 1=yes\nDEPRICATED: Use BIRTH_METHOD 3 instead.");
  CONFIG_ADD_VAR(LEGACY_GRID_LOCAL_SELECTION, bool, 0, "Enable legacy grid local mate selection.\nDEPRICATED: Birth chameber now uses population structure)");
  CONFIG_ADD_VAR(SPECIES_THRESHOLD, int, 2, "max failure count for organisms to be same species");
  CONFIG_ADD_VAR(SPECIES_RECORDING, int, 0, "1 = full, 2 = limited search (parent only)");
  CONFIG_ADD_VAR(SPECIES_PRINT, int, 0, "0/1 (off/on) Print out all species?");
  CONFIG_ADD_VAR(LOG_SPECIES, bool, 0, "0/1 (off/on) toggle to print file.");


  // -------- UNDER CONSTRUCTION ----------
  CONFIG_ADD_GROUP(DEVEL_GROUP, "IN DEVELOPMENT (May not function correctly)");
  CONFIG_ADD_VAR(WORLD_Z, int, 1, "Depth of the Avida world");
  CONFIG_ADD_VAR(STATE_GRID_REQUIRED, bool, 0, "Organism must be in a 0 state cell in state grid to reproduce");
  CONFIG_ADD_VAR(SPIT_STATE_GRID, bool, 0, "Spit out position on state grid");


	
#endif
  
  bool Load(const cString& filename, const cString& working_dir, cUserFeedback* feedback = NULL,
            const tDictionary<cString>* mappings = NULL, bool warn_default = true);
  void Print(const cString& filename);
  void Status();
  void PrintReview();
  
  
  bool Get(const cString& entry, cString& ret) const;
  cString GetAsString(const cString& entry) const;
  bool HasEntry(const cString& entry) const { cString rtn; return Get(entry, rtn); }
  
  bool Set(const cString& entry, const cString& val);
  void Set(tDictionary<cString>& sets);
  
  void GenerateOverides();
};



// Concept:
// Setup #define to build class that will manage the specific variable inside
// of config, as well as any other necessary functions.  Each class must have
// * A Get() method
// * A Set(in_value) method
// * A Load(config file) method
// * Get Name
// * Get Desc
//
// User must provide in the definition:
//  object name
//  config ID
//  default value
//  description
//  type?
//  
//
// We must also register the class so that the Load function can be called
// automatically when the config class is built. (as well as name and help
// and such can be looked up as needed.)
//
// This will all be easiest if we have an abstract base class.
//
// If we want to specify constant values (as opposed to using the dynamic
// loading from a file), the #define will be rebuilt as nothing and a separate
// header will be included that defines each class manually.  This will only
// happen


#endif
