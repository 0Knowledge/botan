/*************************************************
* Memory Locking Functions Source File           *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/util.h>

#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
  #include <sys/types.h>
  #include <sys/mman.h>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK)
  #include <windows.h>
#endif

namespace Botan {

/*************************************************
* Lock an area of memory into RAM                *
*************************************************/
void lock_mem(void*, length_type)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   return (mlock(ptr, bytes) == 0);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK)
   return (VirtualLock(ptr, bytes) != 0);
#else
   return false;
#endif
   }

/*************************************************
* Unlock a previously locked region of memory    *
*************************************************/
void unlock_mem(void*, length_type)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   munlock(ptr, bytes);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK)
   VirtualUnlock(ptr, bytes);
#endif
   }

}
