/*************************************************
* EMSA1 Source File                              *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/emsa.h>
#include <botan/lookup.h>

namespace Botan {

/*************************************************
* EMSA1 Update Operation                         *
*************************************************/
void EMSA1::update(const byte input[], length_type length)
   {
   hash->update(input, length);
   }

/*************************************************
* Return the raw (unencoded) data                *
*************************************************/
SecureVector<byte> EMSA1::raw_data()
   {
   return hash->final();
   }

/*************************************************
* EMSA1 Encode Operation                         *
*************************************************/
SecureVector<byte> EMSA1::encoding_of(const MemoryRegion<byte>& msg,
                                      length_type output_bits)
   {
   if(msg.size() != hash->OUTPUT_LENGTH)
      throw Encoding_Error("EMSA1::encoding_of: Invalid size for input");
   if(8*msg.size() <= output_bits)
      return msg;

   length_type shift = 8*msg.size() - output_bits;

   length_type byte_shift = shift / 8, bit_shift = shift % 8;
   SecureVector<byte> digest(msg.size() - byte_shift);

   for(length_type j = 0; j != msg.size() - byte_shift; ++j)
      digest[j] = msg[j];

   if(bit_shift)
      {
      byte carry = 0;
      for(length_type j = 0; j != digest.size(); ++j)
         {
         byte temp = digest[j];
         digest[j] = (temp >> bit_shift) | carry;
         carry = (temp << (8 - bit_shift));
         }
      }
   return digest;
   }

/*************************************************
* EMSA1 Decode/Verify Operation                  *
*************************************************/
bool EMSA1::verify(const MemoryRegion<byte>& coded,
                   const MemoryRegion<byte>& raw, length_type key_bits) throw()
   {
   try {
      SecureVector<byte> our_coding = encoding_of(raw, key_bits);

      if(our_coding == coded) return true;
      if(our_coding[0] != 0) return false;
      if(our_coding.size() <= coded.size()) return false;

      length_type offset = 0;
      while(our_coding[offset] == 0 && offset < our_coding.size())
         ++offset;
      if(our_coding.size() - offset != coded.size())
         return false;

      for(length_type j = 0; j != coded.size(); ++j)
         if(coded[j] != our_coding[j+offset])
            return false;

      return true;
      }
   catch(Invalid_Argument)
      {
      return false;
      }
   }

/*************************************************
* EMSA1 Constructor                              *
*************************************************/
EMSA1::EMSA1(const std::string& hash_name) :
   hash(get_hash(hash_name))
   {
   }

}
