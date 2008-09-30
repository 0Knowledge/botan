/*************************************************
* KDF2 Header File                               *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_KDF2_H__
#define BOTAN_KDF2_H__

#include <botan/kdf.h>
#include <botan/base.h>

namespace Botan {

/*************************************************
* KDF2                                           *
*************************************************/
class BOTAN_DLL KDF2 : public KDF
   {
   public:
      SecureVector<byte> derive(length_type, const byte[], length_type,
                                const byte[], length_type) const;

      KDF2(HashFunction* h) : hash(h) {}
      KDF2(const KDF2& other) : KDF(), hash(other.hash->clone()) {}
      ~KDF2() { delete hash; }
   private:
      HashFunction* hash;
   };

}

#endif
