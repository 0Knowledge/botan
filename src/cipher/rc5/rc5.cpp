/*************************************************
* RC5 Source File                                *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/rc5.h>
#include <botan/loadstor.h>
#include <botan/bit_ops.h>
#include <botan/parsing.h>
#include <algorithm>

namespace Botan {

/*************************************************
* RC5 Encryption                                 *
*************************************************/
void RC5::enc(const byte in[], byte out[]) const
   {
   u32bit A = load_le<u32bit>(in, 0), B = load_le<u32bit>(in, 1);

   A += S[0]; B += S[1];
   for(length_type j = 0; j != ROUNDS; j += 4)
      {
      A = rotate_left(A ^ B, B % 32) + S[2*j+2];
      B = rotate_left(B ^ A, A % 32) + S[2*j+3];
      A = rotate_left(A ^ B, B % 32) + S[2*j+4];
      B = rotate_left(B ^ A, A % 32) + S[2*j+5];
      A = rotate_left(A ^ B, B % 32) + S[2*j+6];
      B = rotate_left(B ^ A, A % 32) + S[2*j+7];
      A = rotate_left(A ^ B, B % 32) + S[2*j+8];
      B = rotate_left(B ^ A, A % 32) + S[2*j+9];
      }

   store_le(out, A, B);
   }

/*************************************************
* RC5 Decryption                                 *
*************************************************/
void RC5::dec(const byte in[], byte out[]) const
   {
   u32bit A = load_le<u32bit>(in, 0), B = load_le<u32bit>(in, 1);

   for(length_type j = ROUNDS; j != 0; j -= 4)
      {
      B = rotate_right(B - S[2*j+1], A % 32) ^ A;
      A = rotate_right(A - S[2*j  ], B % 32) ^ B;
      B = rotate_right(B - S[2*j-1], A % 32) ^ A;
      A = rotate_right(A - S[2*j-2], B % 32) ^ B;
      B = rotate_right(B - S[2*j-3], A % 32) ^ A;
      A = rotate_right(A - S[2*j-4], B % 32) ^ B;
      B = rotate_right(B - S[2*j-5], A % 32) ^ A;
      A = rotate_right(A - S[2*j-6], B % 32) ^ B;
      }
   B -= S[1]; A -= S[0];

   store_le(out, A, B);
   }

/*************************************************
* RC5 Key Schedule                               *
*************************************************/
void RC5::key(const byte key[], length_type length)
   {
   const length_type WORD_KEYLENGTH = (((length - 1) / 4) + 1),
                     MIX_ROUNDS     = 3*std::max(WORD_KEYLENGTH, S.size());
   S[0] = 0xB7E15163;
   for(length_type j = 1; j != S.size(); ++j)
      S[j] = S[j-1] + 0x9E3779B9;

   SecureBuffer<u32bit, 8> K;
   for(s32bit j = length-1; j >= 0; --j)
      K[j/4] = (K[j/4] << 8) + key[j];

   u32bit A = 0, B = 0;
   for(length_type j = 0; j != MIX_ROUNDS; ++j)
      {
      A = rotate_left(S[j % S.size()] + A + B, 3);
      B = rotate_left(K[j % WORD_KEYLENGTH] + A + B, (A + B) % 32);
      S[j % S.size()] = A;
      K[j % WORD_KEYLENGTH] = B;
      }
   }

/*************************************************
* Return the name of this type                   *
*************************************************/
std::string RC5::name() const
   {
   return "RC5(" + to_string(ROUNDS) + ")";
   }

/*************************************************
* RC5 Constructor                                *
*************************************************/
RC5::RC5(length_type r) : BlockCipher(8, 1, 32), ROUNDS(r)
   {
   if(ROUNDS < 8 || ROUNDS > 32 || (ROUNDS % 4 != 0))
      throw Invalid_Argument(name() + ": Invalid number of rounds");
   S.create(2*ROUNDS + 2);
   }

}
