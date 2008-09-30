/*************************************************
* XTEA Source File                               *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/xtea.h>
#include <botan/loadstor.h>
#include <botan/parsing.h>

namespace Botan {

/*************************************************
* XTEA Encryption                                *
*************************************************/
void XTEA::enc(const byte in[], byte out[]) const
   {
   u32bit L = load_be<u32bit>(in, 0), R = load_be<u32bit>(in, 1);

   for(length_type j = 0; j != 32; ++j)
      {
      L += (((R << 4) ^ (R >> 5)) + R) ^ EK[2*j];
      R += (((L << 4) ^ (L >> 5)) + L) ^ EK[2*j+1];
      }

   store_be(out, L, R);
   }

/*************************************************
* XTEA Decryption                                *
*************************************************/
void XTEA::dec(const byte in[], byte out[]) const
   {
   u32bit L = load_be<u32bit>(in, 0), R = load_be<u32bit>(in, 1);

   for(length_type j = 32; j > 0; --j)
      {
      R -= (((L << 4) ^ (L >> 5)) + L) ^ EK[2*j - 1];
      L -= (((R << 4) ^ (R >> 5)) + R) ^ EK[2*j - 2];
      }

   store_be(out, L, R);
   }

/*************************************************
* XTEA Key Schedule                              *
*************************************************/
void XTEA::key(const byte key[], length_type)
   {
   static const u32bit DELTAS[64] = {
      0x00000000, 0x9E3779B9, 0x9E3779B9, 0x3C6EF372, 0x3C6EF372, 0xDAA66D2B,
      0xDAA66D2B, 0x78DDE6E4, 0x78DDE6E4, 0x1715609D, 0x1715609D, 0xB54CDA56,
      0xB54CDA56, 0x5384540F, 0x5384540F, 0xF1BBCDC8, 0xF1BBCDC8, 0x8FF34781,
      0x8FF34781, 0x2E2AC13A, 0x2E2AC13A, 0xCC623AF3, 0xCC623AF3, 0x6A99B4AC,
      0x6A99B4AC, 0x08D12E65, 0x08D12E65, 0xA708A81E, 0xA708A81E, 0x454021D7,
      0x454021D7, 0xE3779B90, 0xE3779B90, 0x81AF1549, 0x81AF1549, 0x1FE68F02,
      0x1FE68F02, 0xBE1E08BB, 0xBE1E08BB, 0x5C558274, 0x5C558274, 0xFA8CFC2D,
      0xFA8CFC2D, 0x98C475E6, 0x98C475E6, 0x36FBEF9F, 0x36FBEF9F, 0xD5336958,
      0xD5336958, 0x736AE311, 0x736AE311, 0x11A25CCA, 0x11A25CCA, 0xAFD9D683,
      0xAFD9D683, 0x4E11503C, 0x4E11503C, 0xEC48C9F5, 0xEC48C9F5, 0x8A8043AE,
      0x8A8043AE, 0x28B7BD67, 0x28B7BD67, 0xC6EF3720 };

   static const byte KEY_INDEX[64] = {
      0x00, 0x03, 0x01, 0x02, 0x02, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03,
      0x02, 0x02, 0x03, 0x01, 0x00, 0x00, 0x01, 0x00, 0x02, 0x03, 0x03, 0x02,
      0x00, 0x01, 0x01, 0x01, 0x02, 0x00, 0x03, 0x03, 0x00, 0x02, 0x01, 0x01,
      0x02, 0x01, 0x03, 0x00, 0x00, 0x03, 0x01, 0x02, 0x02, 0x01, 0x03, 0x01,
      0x00, 0x00, 0x01, 0x03, 0x02, 0x02, 0x03, 0x02, 0x00, 0x01, 0x01, 0x00,
      0x02, 0x03, 0x03, 0x02 };

   SecureBuffer<u32bit, 4> UK;
   for(length_type j = 0; j != 4; ++j)
      UK[j] = load_be<u32bit>(key, j);

   for(length_type j = 0; j != 64; ++j)
      EK[j] = DELTAS[j] + UK[KEY_INDEX[j]];
   }

}
