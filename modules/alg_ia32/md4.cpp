/*************************************************
* MD4 Source File                                *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/md4.h>
#include <botan/loadstor.h>

namespace Botan {

extern "C" void md4_core(length_type[4], const byte[64], length_type[16]);

/*************************************************
* MD4 Compression Function                       *
*************************************************/
void MD4::hash(const byte input[])
   {
   md4_core(digest, input, M);
   }

/*************************************************
* Copy out the digest                            *
*************************************************/
void MD4::copy_out(byte output[])
   {
   for(length_type j = 0; j != OUTPUT_LENGTH; ++j)
      output[j] = get_byte(3 - (j % 4), digest[j/4]);
   }

/*************************************************
* Clear memory of sensitive data                 *
*************************************************/
void MD4::clear() throw()
   {
   MDx_HashFunction::clear();
   M.clear();
   digest[0] = 0x67452301;
   digest[1] = 0xEFCDAB89;
   digest[2] = 0x98BADCFE;
   digest[3] = 0x10325476;
   }

}
