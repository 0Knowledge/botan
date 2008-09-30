/*************************************************
* Memory Mapping Allocator Header File           *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_MMAP_ALLOCATOR_H__
#define BOTAN_MMAP_ALLOCATOR_H__

#include <botan/mem_pool.h>

namespace Botan {

/*************************************************
* Memory Mapping Allocator                       *
*************************************************/
class MemoryMapping_Allocator : public Pooling_Allocator
   {
   public:
      MemoryMapping_Allocator(Mutex* m) : Pooling_Allocator(m) {}
      std::string type() const { return "mmap"; }
   private:
      void* alloc_block(length_type);
      void dealloc_block(void*, length_type);
   };

}

#endif
