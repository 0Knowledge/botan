/*************************************************
* MD2 Header File                                *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_MD2_H__
#define BOTAN_MD2_H__

#include <botan/mdx_hash.h>

namespace Botan {

/*************************************************
* MD2                                            *
*************************************************/
class BOTAN_DLL MD2 : public HashFunction
   {
   public:
      void clear() throw();
      std::string name() const { return "MD2"; }
      HashFunction* clone() const { return new MD2; }
      MD2() : HashFunction(16, 16) { clear(); }
   private:
      void add_data(const byte[], length_type);
      void hash(const byte[]);
      void final_result(byte[]);

      SecureBuffer<byte, 48> X;
      SecureBuffer<byte, 16> checksum, buffer;
      length_type position;
   };

}

#endif
