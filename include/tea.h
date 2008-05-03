/*************************************************
* TEA Header File                                *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_TEA_H__
#define BOTAN_TEA_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* TEA                                            *
*************************************************/
class BOTAN_DLL TEA : public BlockCipher
   {
   public:
      void clear() throw() { K.clear(); }
      std::string name() const { return "TEA"; }
      BlockCipher* clone() const { return new TEA; }
      TEA() : BlockCipher(8, 16) {}
   private:
      void enc(const byte[], byte[]) const;
      void dec(const byte[], byte[]) const;
      void key(const byte[], length_type);
      SecureBuffer<u32bit, 4> K;
   };

}

#endif
