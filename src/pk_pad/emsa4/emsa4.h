/*************************************************
* EMSA4 Header File                              *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_EMSA4_H__
#define BOTAN_EMSA4_H__

#include <botan/pk_pad.h>
#include <botan/kdf.h>

namespace Botan {

/*************************************************
* EMSA4                                          *
*************************************************/
class BOTAN_DLL EMSA4 : public EMSA
   {
   public:
      EMSA4(HashFunction*);
      EMSA4(HashFunction*, length_type);

      ~EMSA4() { delete hash; delete mgf; }
   private:
      void update(const byte[], length_type);
      SecureVector<byte> raw_data();

      SecureVector<byte> encoding_of(const MemoryRegion<byte>&,
                                     length_type,
                                     RandomNumberGenerator& rng);
      bool verify(const MemoryRegion<byte>&, const MemoryRegion<byte>&,
                  length_type) throw();

      u32bit SALT_SIZE;
      HashFunction* hash;
      const MGF* mgf;
   };

}

#endif
