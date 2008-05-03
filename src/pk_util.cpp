/*************************************************
* PK Utility Classes Source File                 *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/pk_util.h>

namespace Botan {

/*************************************************
* Encode a message                               *
*************************************************/
SecureVector<byte> EME::encode(const byte msg[], length_type msg_len,
                               length_type key_bits) const
   {
   return pad(msg, msg_len, key_bits);
   }

/*************************************************
* Encode a message                               *
*************************************************/
SecureVector<byte> EME::encode(const MemoryRegion<byte>& msg,
                               length_type key_bits) const
   {
   return pad(msg, msg.size(), key_bits);
   }

/*************************************************
* Decode a message                               *
*************************************************/
SecureVector<byte> EME::decode(const byte msg[], length_type msg_len,
                               length_type key_bits) const
   {
   return unpad(msg, msg_len, key_bits);
   }

/*************************************************
* Decode a message                               *
*************************************************/
SecureVector<byte> EME::decode(const MemoryRegion<byte>& msg,
                               length_type key_bits) const
   {
   return unpad(msg, msg.size(), key_bits);
   }

/*************************************************
* Default signature decoding                     *
*************************************************/
bool EMSA::verify(const MemoryRegion<byte>& coded,
                  const MemoryRegion<byte>& raw,
                  length_type key_bits) throw()
   {
   try {
      return (coded == encoding_of(raw, key_bits));
      }
   catch(Invalid_Argument)
      {
      return false;
      }
   }

}
