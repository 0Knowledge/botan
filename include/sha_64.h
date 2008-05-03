/*************************************************
* SHA-{384,512} Header File                      *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_SHA_64BIT_H__
#define BOTAN_SHA_64BIT_H__

#include <botan/mdx_hash.h>

namespace Botan {

/*************************************************
* SHA-{384,512} Base                             *
*************************************************/
class BOTAN_DLL SHA_64_BASE : public MDx_HashFunction
   {
   protected:
      void clear() throw();
      SHA_64_BASE(length_type out) :
         MDx_HashFunction(out, 128, true, true, 16) {}
      SecureBuffer<u64bit, 8> digest;
   private:
      void hash(const byte[]);
      void copy_out(byte[]);

      SecureBuffer<u64bit, 80> W;
   };

/*************************************************
* SHA-384                                        *
*************************************************/
class BOTAN_DLL SHA_384 : public SHA_64_BASE
   {
   public:
      void clear() throw();
      std::string name() const { return "SHA-384"; }
      HashFunction* clone() const { return new SHA_384; }
      SHA_384() : SHA_64_BASE(48) { clear(); }
   };

/*************************************************
* SHA-512                                        *
*************************************************/
class BOTAN_DLL SHA_512 : public SHA_64_BASE
   {
   public:
      void clear() throw();
      std::string name() const { return "SHA-512"; }
      HashFunction* clone() const { return new SHA_512; }
      SHA_512() : SHA_64_BASE(64) { clear(); }
   };

}

#endif
