/*************************************************
* RC2 Header File                                *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_RC2_H__
#define BOTAN_RC2_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* RC2                                            *
*************************************************/
class BOTAN_DLL RC2 : public BlockCipher
   {
   public:
      static byte EKB_code(length_type);

      void clear() throw() { K.clear(); }
      std::string name() const { return "RC2"; }
      BlockCipher* clone() const { return new RC2; }
      RC2() : BlockCipher(8, 1, 32) {}
   private:
      void enc(const byte[], byte[]) const;
      void dec(const byte[], byte[]) const;
      void key(const byte[], length_type);

      SecureBuffer<u16bit, 64> K;
   };

}

#endif
