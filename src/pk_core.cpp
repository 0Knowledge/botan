/*************************************************
* PK Algorithm Core Source File                  *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/pk_core.h>
#include <botan/numthry.h>
#include <botan/engine.h>
#include <botan/config.h>
#include <botan/parsing.h>
#include <algorithm>

namespace Botan {

namespace {

const length_type BLINDING_BITS = BOTAN_PRIVATE_KEY_OP_BLINDING_BITS;

}

/*************************************************
* IF_Core Constructor                            *
*************************************************/
IF_Core::IF_Core(const BigInt& e, const BigInt& n, const BigInt& d,
                 const BigInt& p, const BigInt& q,
                 const BigInt& d1, const BigInt& d2, const BigInt& c)
   {
   op = Engine_Core::if_op(e, n, d, p, q, d1, d2, c);

   if(d != 0)
      {
      BigInt k = random_integer(std::min(n.bits()-1, BLINDING_BITS));
      if(k != 0)
         blinder = Blinder(power_mod(k, e, n), inverse_mod(k, n), n);
      }
   }

/*************************************************
* IF_Core Copy Constructor                       *
*************************************************/
IF_Core::IF_Core(const IF_Core& core)
   {
   op = 0;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   }

/*************************************************
* IF_Core Assignment Operator                    *
*************************************************/
IF_Core& IF_Core::operator=(const IF_Core& core)
   {
   delete op;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   return (*this);
   }

/*************************************************
* IF Public Operation                            *
*************************************************/
BigInt IF_Core::public_op(const BigInt& i) const
   {
   return op->public_op(i);
   }

/*************************************************
* IF Private Operation                           *
*************************************************/
BigInt IF_Core::private_op(const BigInt& i) const
   {
   return blinder.unblind(op->private_op(blinder.blind(i)));
   }

/*************************************************
* DSA_Core Constructor                           *
*************************************************/
DSA_Core::DSA_Core(const DL_Group& group, const BigInt& y, const BigInt& x)
   {
   op = Engine_Core::dsa_op(group, y, x);
   }

/*************************************************
* DSA_Core Copy Constructor                      *
*************************************************/
DSA_Core::DSA_Core(const DSA_Core& core)
   {
   op = 0;
   if(core.op)
      op = core.op->clone();
   }

/*************************************************
* DSA_Core Assignment Operator                   *
*************************************************/
DSA_Core& DSA_Core::operator=(const DSA_Core& core)
   {
   delete op;
   if(core.op)
      op = core.op->clone();
   return (*this);
   }

/*************************************************
* DSA Verification Operation                     *
*************************************************/
bool DSA_Core::verify(const byte msg[], length_type msg_length,
                      const byte sig[], length_type sig_length) const
   {
   return op->verify(msg, msg_length, sig, sig_length);
   }

/*************************************************
* DSA Signature Operation                        *
*************************************************/
SecureVector<byte> DSA_Core::sign(const byte in[], length_type length,
                                  const BigInt& k) const
   {
   return op->sign(in, length, k);
   }

/*************************************************
* NR_Core Constructor                            *
*************************************************/
NR_Core::NR_Core(const DL_Group& group, const BigInt& y, const BigInt& x)
   {
   op = Engine_Core::nr_op(group, y, x);
   }

/*************************************************
* NR_Core Copy Constructor                       *
*************************************************/
NR_Core::NR_Core(const NR_Core& core)
   {
   op = 0;
   if(core.op)
      op = core.op->clone();
   }

/*************************************************
* NR_Core Assignment Operator                    *
*************************************************/
NR_Core& NR_Core::operator=(const NR_Core& core)
   {
   delete op;
   if(core.op)
      op = core.op->clone();
   return (*this);
   }

/*************************************************
* NR Verification Operation                      *
*************************************************/
SecureVector<byte> NR_Core::verify(const byte in[], length_type length) const
   {
   return op->verify(in, length);
   }

/*************************************************
* NR Signature Operation                         *
*************************************************/
SecureVector<byte> NR_Core::sign(const byte in[], length_type length,
                                 const BigInt& k) const
   {
   return op->sign(in, length, k);
   }

/*************************************************
* ELG_Core Constructor                           *
*************************************************/
ELG_Core::ELG_Core(const DL_Group& group, const BigInt& y, const BigInt& x)
   {
   op = Engine_Core::elg_op(group, y, x);

   p_bytes = 0;
   if(x != 0)
      {
      const BigInt& p = group.get_p();
      p_bytes = p.bytes();

      BigInt k = random_integer(std::min(p.bits()-1, BLINDING_BITS));
      if(k != 0)
         blinder = Blinder(k, power_mod(k, x, p), p);
      }
   }

/*************************************************
* ELG_Core Copy Constructor                      *
*************************************************/
ELG_Core::ELG_Core(const ELG_Core& core)
   {
   op = 0;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   p_bytes = core.p_bytes;
   }

/*************************************************
* ELG_Core Assignment Operator                   *
*************************************************/
ELG_Core& ELG_Core::operator=(const ELG_Core& core)
   {
   delete op;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   p_bytes = core.p_bytes;
   return (*this);
   }

/*************************************************
* ElGamal Encrypt Operation                      *
*************************************************/
SecureVector<byte> ELG_Core::encrypt(const byte in[], length_type length,
                                     const BigInt& k) const
   {
   return op->encrypt(in, length, k);
   }

/*************************************************
* ElGamal Decrypt Operation                      *
*************************************************/
SecureVector<byte> ELG_Core::decrypt(const byte in[], length_type length) const
   {
   if(length != 2*p_bytes)
      throw Invalid_Argument("ELG_Core::decrypt: Invalid message");

   BigInt a(in, p_bytes);
   BigInt b(in + p_bytes, p_bytes);

   return BigInt::encode(blinder.unblind(op->decrypt(blinder.blind(a), b)));
   }

/*************************************************
* DH_Core Constructor                            *
*************************************************/
DH_Core::DH_Core(const DL_Group& group, const BigInt& x)
   {
   op = Engine_Core::dh_op(group, x);

   const BigInt& p = group.get_p();
   BigInt k = random_integer(std::min(p.bits()-1, BLINDING_BITS));
   if(k != 0)
      blinder = Blinder(k, power_mod(inverse_mod(k, p), x, p), p);
   }

/*************************************************
* DH_Core Copy Constructor                       *
*************************************************/
DH_Core::DH_Core(const DH_Core& core)
   {
   op = 0;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   }

/*************************************************
* DH_Core Assignment Operator                    *
*************************************************/
DH_Core& DH_Core::operator=(const DH_Core& core)
   {
   delete op;
   if(core.op)
      op = core.op->clone();
   blinder = core.blinder;
   return (*this);
   }

/*************************************************
* DH Operation                                   *
*************************************************/
BigInt DH_Core::agree(const BigInt& i) const
   {
   return blinder.unblind(op->agree(blinder.blind(i)));
   }

}
