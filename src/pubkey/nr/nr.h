/*
* Nyberg-Rueppel
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_NYBERG_RUEPPEL_H__
#define BOTAN_NYBERG_RUEPPEL_H__

#include <botan/pk_keys.h>
#include <botan/dl_group.h>
#include <botan/nr_core.h>
#include <botan/rng.h>

namespace Botan {

class BOTAN_DLL Nyberg_Rueppel_Key : public virtual Public_Key
   {
   public:
      std::string algo_name() const { return "NR"; }

      /**
      * Get the DL domain parameters of this key.
      * @return the DL domain parameters of this key
      */
      const DL_Group& get_domain() const { return group; }

      /**
      * Get the public value y with y = g^x mod p where x is the secret key.
      */
      const BigInt& get_y() const { return y; }

      /**
      * Get the prime p of the underlying DL group.
      * @return the prime p
      */
      const BigInt& group_p() const { return group.get_p(); }

      /**
      * Get the prime q of the underlying DL group.
      * @return the prime q
      */
      const BigInt& group_q() const { return group.get_q(); }

      /**
      * Get the generator g of the underlying DL group.
      * @return the generator g
      */
      const BigInt& group_g() const { return group.get_g(); }

      u32bit message_parts() const { return 2; }

      /**
      * Return the size of each portion of the sig
      */
      u32bit message_part_size() const { return group_q().bytes(); }

      /**
      * Return the maximum input size in bits
      */
      u32bit max_input_bits() const { return (group_q().bits() - 1); }

      std::pair<AlgorithmIdentifier, MemoryVector<byte> >
         subject_public_key_info() const;
   protected:
      DL_Group group;
      BigInt y;
   };

/**
* Nyberg-Rueppel Public Key
*/
class BOTAN_DLL NR_PublicKey : public Nyberg_Rueppel_Key,
                               public PK_Verifying_with_MR_Key
   {
   public:
      NR_PublicKey(const DL_Group& group, const BigInt& y);

      NR_PublicKey(const AlgorithmIdentifier& alg_id,
                   const MemoryRegion<byte>& key_bits);

      SecureVector<byte> verify(const byte[], u32bit) const;

      bool check_key(RandomNumberGenerator& rng, bool) const;
   private:
      NR_Core core;
   };

/**
* Nyberg-Rueppel Private Key
*/
class BOTAN_DLL NR_PrivateKey : public Nyberg_Rueppel_Key,
                                public PK_Signing_Key
   {
   public:
      SecureVector<byte> sign(const byte msg[], u32bit msg_len,
                              RandomNumberGenerator& rng) const;

      NR_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const MemoryRegion<byte>& key_bits,
                    RandomNumberGenerator& rng);

      NR_PrivateKey(RandomNumberGenerator& rng, const DL_Group& group,
                    const BigInt& x = 0);

      NR_PublicKey public_key() const { return NR_PublicKey(group, y); }

      bool check_key(RandomNumberGenerator& rng, bool strong) const;

      /**
      * Get the secret key x.
      * @return the secret key
      */
      const BigInt& get_x() const { return x; }

      std::pair<AlgorithmIdentifier, SecureVector<byte> >
         pkcs8_encoding() const;
   private:
      NR_Core core;
      BigInt x;
   };

}

#endif
