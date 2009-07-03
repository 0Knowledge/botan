/*
* RSA
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RSA_H__
#define BOTAN_RSA_H__

#include <botan/if_algo.h>

namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public virtual PK_Encrypting_Key,
                                public virtual PK_Verifying_with_MR_Key,
                                public IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RSA"; }

      SecureVector<byte> encrypt(const byte[], u32bit,
                                 RandomNumberGenerator& rng) const;

      SecureVector<byte> verify(const byte[], u32bit) const;

      X509_Encoder* x509_encoder() const
         { return IF_Scheme_PublicKey::x509_encoder(); }
      X509_Decoder* x509_decoder()
         { return IF_Scheme_PublicKey::x509_decoder(); }

      u32bit max_input_bits() const { return (n.bits() - 1); }

      RSA_PublicKey() {}
      RSA_PublicKey(const BigInt& n, const BigInt& e);
   protected:
      BigInt public_op(const BigInt&) const;
   };

/**
* RSA Private Key class.
*/
class BOTAN_DLL RSA_PrivateKey : public virtual PK_Decrypting_Key,
                                 public virtual PK_Signing_Key,
                                 public virtual IF_Scheme_PrivateKey
   {
   public:
      std::string algo_name() const { return "RSA"; }

      /**
        Return a new public key matching this private key
      */
      RSA_PublicKey* public_key() const;

      SecureVector<byte> sign(const byte[], u32bit,
                              RandomNumberGenerator&) const;

      SecureVector<byte> decrypt(const byte[], u32bit) const;

      bool check_key(RandomNumberGenerator& rng, bool) const;

      PKCS8_Encoder* pkcs8_encoder() const
         { return IF_Scheme_PrivateKey::pkcs8_encoder(); }
      PKCS8_Decoder* pkcs8_decoder(RandomNumberGenerator& rng)
         { return IF_Scheme_PrivateKey::pkcs8_decoder(rng); }

      /**
      * Default constructor, does not set any internal values. Use this
      * constructor if you wish to decode a DER or PEM encoded key.
      */
      RSA_PrivateKey() {}

      /**
      * Construct a private key from the specified parameters.
      * @param rng the random number generator to use
      * @param prime1 the first prime
      * @param prime2 the second prime
      * @param exp the exponent
      * @param d_exp if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     const BigInt& p, const BigInt& q, const BigInt& e,
                     const BigInt& d = 0, const BigInt& n = 0);

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     u32bit bits, u32bit exp = 65537);
   private:
      BigInt private_op(const byte[], u32bit) const;
   };

}

#endif
