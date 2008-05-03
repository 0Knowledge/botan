/*************************************************
* Library Internal/Global State Header File      *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_LIB_STATE_H__
#define BOTAN_LIB_STATE_H__

#include <botan/base.h>
#include <botan/enums.h>
#include <botan/init.h>
#include <botan/ui.h>
#include <string>
#include <vector>
#include <map>

namespace Botan {

/*************************************************
* Global State Container Base                    *
*************************************************/
class BOTAN_DLL Library_State
   {
   public:
      Library_State();
      ~Library_State();

      void initialize(const InitializerOptions&, Modules&);

      void load(Modules&);

      void add_engine(class Engine*);

      class BOTAN_DLL Engine_Iterator
         {
         public:
            class Engine* next();
            Engine_Iterator(const Library_State& l) : lib(l) { n = 0; }
         private:
            const Library_State& lib;
            length_type n;
         };
      friend class Engine_Iterator;

      Allocator* get_allocator(const std::string& = "") const;
      void add_allocator(Allocator*);
      void set_default_allocator(const std::string&) const;

      bool rng_is_seeded() const { return rng->is_seeded(); }
      void randomize(byte[], length_type);
      byte random();

      void set_prng(RandomNumberGenerator*);
      void add_entropy_source(EntropySource*, bool = true);
      void add_entropy(const byte[], length_type);
      void add_entropy(EntropySource&, bool);
      length_type seed_prng(bool, length_type);

      class Config& config() const;

      class Mutex* get_mutex() const;
   private:
      Library_State(const Library_State&) {}
      Library_State& operator=(const Library_State&) { return (*this); }

      class Engine* get_engine_n(length_type) const;

      class Mutex_Factory* mutex_factory;
      class Mutex* allocator_lock;
      class Mutex* engine_lock;
      class Mutex* rng_lock;

      mutable class Config* config_obj;

      std::map<std::string, Allocator*> alloc_factory;
      mutable Allocator* cached_default_allocator;

      RandomNumberGenerator* rng;
      std::vector<Allocator*> allocators;
      std::vector<EntropySource*> entropy_sources;
      std::vector<class Engine*> engines;
   };

/*************************************************
* Global State                                   *
*************************************************/
BOTAN_DLL Library_State& global_state();
BOTAN_DLL void set_global_state(Library_State*);
BOTAN_DLL Library_State* swap_global_state(Library_State*);

}

#endif
