/*************************************************
* Turing Header File                             *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_TURING_H__
#define BOTAN_TURING_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* Turing                                         *
*************************************************/
class BOTAN_DLL Turing : public StreamCipher
   {
   public:
      void clear() throw();
      std::string name() const { return "Turing"; }
      StreamCipher* clone() const { return new Turing; }
      Turing() : StreamCipher(4, 32, 4) { position = 0; }
   private:
      void cipher(const byte[], byte[], length_type);
      void key(const byte[], length_type);
      void resync(const byte[], length_type);
      void generate();

      static u32bit fixedS(u32bit);
      static void gen_sbox(MemoryRegion<u32bit>&, u32bit,
                           const MemoryRegion<u32bit>&);

      static const u32bit Q_BOX[256];
      static const byte SBOX[256];
      static const byte OFFSETS[272];

      SecureBuffer<u32bit, 256> S0, S1, S2, S3;
      SecureBuffer<u32bit, 17> R;
      SecureVector<u32bit> K;
      SecureBuffer<byte, 340> buffer;
      length_type position;
   };

}

#endif
