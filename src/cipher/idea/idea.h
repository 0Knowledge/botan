/*************************************************
* IDEA Header File                               *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_IDEA_H__
#define BOTAN_IDEA_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* IDEA                                           *
*************************************************/
class BOTAN_DLL IDEA : public BlockCipher
   {
   public:
      void clear() throw() { EK.clear(); DK.clear(); }
      std::string name() const { return "IDEA"; }
      BlockCipher* clone() const { return new IDEA; }
      IDEA() : BlockCipher(8, 16) {}
   private:
      void enc(const byte[], byte[]) const;
      void dec(const byte[], byte[]) const;
      void key(const byte[], length_type);
      SecureBuffer<u16bit, 52> EK, DK;
   };

}

#endif
