/*************************************************
* HMAC Header File                               *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_HMAC_H__
#define BOTAN_HMAC_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* HMAC                                           *
*************************************************/
class BOTAN_DLL HMAC : public MessageAuthenticationCode
   {
   public:
      void clear() throw();
      std::string name() const;
      MessageAuthenticationCode* clone() const;

      HMAC(HashFunction* hash);
      ~HMAC() { delete hash; }
   private:
      void add_data(const byte[], length_type);
      void final_result(byte[]);
      void key(const byte[], length_type);
      HashFunction* hash;
      SecureVector<byte> i_key, o_key;
   };

}

#endif
