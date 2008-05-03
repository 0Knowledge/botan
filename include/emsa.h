/*************************************************
* EMSA Header File                               *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_EMSA_H__
#define BOTAN_EMSA_H__

#include <botan/pk_util.h>

namespace Botan {

/*************************************************
* EMSA1                                          *
*************************************************/
class BOTAN_DLL EMSA1 : public EMSA
   {
   public:
      EMSA1(const std::string&);
      ~EMSA1() { delete hash; }
   private:
      void update(const byte[], length_type);
      SecureVector<byte> encoding_of(const MemoryRegion<byte>&, length_type);
      SecureVector<byte> raw_data();
      bool verify(const MemoryRegion<byte>&, const MemoryRegion<byte>&,
                  length_type) throw();
      HashFunction* hash;
   };

/*************************************************
* EMSA2                                          *
*************************************************/
class BOTAN_DLL EMSA2 : public EMSA
   {
   public:
      EMSA2(const std::string&);
      ~EMSA2() { delete hash; }
   private:
      void update(const byte[], length_type);
      SecureVector<byte> encoding_of(const MemoryRegion<byte>&, length_type);
      SecureVector<byte> raw_data();
      SecureVector<byte> empty_hash;
      HashFunction* hash;
      byte hash_id;
   };

/*************************************************
* EMSA3                                          *
*************************************************/
class BOTAN_DLL EMSA3 : public EMSA
   {
   public:
      EMSA3(const std::string&);
      ~EMSA3() { delete hash; }
   private:
      void update(const byte[], length_type);
      SecureVector<byte> encoding_of(const MemoryRegion<byte>&, length_type);
      SecureVector<byte> raw_data();
      HashFunction* hash;
      SecureVector<byte> hash_id;
   };

/*************************************************
* EMSA4                                          *
*************************************************/
class BOTAN_DLL EMSA4 : public EMSA
   {
   public:
      EMSA4(const std::string&, const std::string&);
      EMSA4(const std::string&, const std::string&, length_type);
      ~EMSA4() { delete hash; delete mgf; }
   private:
      void update(const byte[], length_type);
      SecureVector<byte> encoding_of(const MemoryRegion<byte>&, length_type);
      SecureVector<byte> raw_data();
      bool verify(const MemoryRegion<byte>&, const MemoryRegion<byte>&,
                  length_type) throw();
      const length_type SALT_SIZE;
      HashFunction* hash;
      const MGF* mgf;
   };

/*************************************************
* EMSA-Raw                                       *
*************************************************/
class BOTAN_DLL EMSA_Raw : public EMSA
   {
   private:
      void update(const byte[], length_type);
      SecureVector<byte> encoding_of(const MemoryRegion<byte>&, length_type);
      SecureVector<byte> raw_data();
      SecureVector<byte> message;
   };

}

#endif
