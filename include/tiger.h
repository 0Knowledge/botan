/*************************************************
* Tiger Header File                              *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_TIGER_H__
#define BOTAN_TIGER_H__

#include <botan/mdx_hash.h>

namespace Botan {

/*************************************************
* Tiger                                          *
*************************************************/
class BOTAN_DLL Tiger : public MDx_HashFunction
   {
   public:
      void clear() throw();
      std::string name() const;
      HashFunction* clone() const { return new Tiger(OUTPUT_LENGTH); }
      Tiger(length_type = 24, length_type = 3);
   private:
      void hash(const byte[]);
      void copy_out(byte[]);

      static void pass(u64bit&, u64bit&, u64bit&, u64bit[8], byte);
      static void mix(u64bit[8]);

      static const u64bit SBOX1[256];
      static const u64bit SBOX2[256];
      static const u64bit SBOX3[256];
      static const u64bit SBOX4[256];

      SecureBuffer<u64bit, 8> X;
      SecureBuffer<u64bit, 3> digest;
      const length_type PASS;
   };

}

#endif
