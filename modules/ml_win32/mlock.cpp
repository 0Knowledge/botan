/*************************************************
* Memory Locking Functions Source File           *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/util.h>
#include <windows.h>

namespace Botan {

/*************************************************
* Lock an area of memory into RAM                *
*************************************************/
void lock_mem(void* ptr, length_type bytes)
   {
   VirtualLock(ptr, bytes);
   }

/*************************************************
* Unlock a previously locked region of memory    *
*************************************************/
void unlock_mem(void* ptr, length_type bytes)
   {
   VirtualUnlock(ptr, bytes);
   }

}
