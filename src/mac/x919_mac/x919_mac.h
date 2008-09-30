/*************************************************
* ANSI X9.19 MAC Header File                     *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_ANSI_X919_MAC_H__
#define BOTAN_ANSI_X919_MAC_H__

#include <botan/base.h>

namespace Botan {

/*************************************************
* ANSI X9.19 MAC                                 *
*************************************************/
class BOTAN_DLL ANSI_X919_MAC : public MessageAuthenticationCode
   {
   public:
      void clear() throw();
      std::string name() const;
      MessageAuthenticationCode* clone() const;

      ANSI_X919_MAC(BlockCipher*);
      ~ANSI_X919_MAC();
   private:
      void add_data(const byte[], length_type);
      void final_result(byte[]);
      void key(const byte[], length_type);
      BlockCipher* e;
      BlockCipher* d;
      SecureBuffer<byte, 8> state;
      length_type position;
   };

}

#endif
