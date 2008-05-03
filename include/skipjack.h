/*************************************************
* Skipjack Header File                           *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_SKIPJACK_H__
#define BOTAN_SKIPJACK_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* Skipjack                                       *
*************************************************/
class BOTAN_DLL Skipjack : public BlockCipher
   {
   public:
      void clear() throw();
      std::string name() const { return "Skipjack"; }
      BlockCipher* clone() const { return new Skipjack; }
      Skipjack() : BlockCipher(8, 10) {}
   private:
      void enc(const byte[], byte[]) const;
      void dec(const byte[], byte[]) const;
      void key(const byte[], length_type);
      void step_A(u16bit&, u16bit&, length_type) const;
      void step_B(u16bit&, u16bit&, length_type) const;
      void step_Ai(u16bit&, u16bit&, length_type) const;
      void step_Bi(u16bit&, u16bit&, length_type) const;
      SecureBuffer<byte, 256> FTABLE[10];
   };

}

#endif
