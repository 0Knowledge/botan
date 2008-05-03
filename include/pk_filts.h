/*************************************************
* PK Filters Header File                         *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_PK_FILTERS_H__
#define BOTAN_PK_FILTERS_H__

#include <botan/filter.h>
#include <botan/pubkey.h>

namespace Botan {

/*************************************************
* PK_Encryptor Filter                            *
*************************************************/
class BOTAN_DLL PK_Encryptor_Filter : public Filter
   {
   public:
      void write(const byte[], length_type);
      void end_msg();
      PK_Encryptor_Filter(PK_Encryptor* c) : cipher(c) {}
      ~PK_Encryptor_Filter() { delete cipher; }
   private:
      PK_Encryptor* cipher;
      SecureVector<byte> buffer;
   };

/*************************************************
* PK_Decryptor Filter                            *
*************************************************/
class BOTAN_DLL PK_Decryptor_Filter : public Filter
   {
   public:
      void write(const byte[], length_type);
      void end_msg();
      PK_Decryptor_Filter(PK_Decryptor* c) : cipher(c) {}
      ~PK_Decryptor_Filter() { delete cipher; }
   private:
      PK_Decryptor* cipher;
      SecureVector<byte> buffer;
   };

/*************************************************
* PK_Signer Filter                               *
*************************************************/
class BOTAN_DLL PK_Signer_Filter : public Filter
   {
   public:
      void write(const byte[], length_type);
      void end_msg();
      PK_Signer_Filter(PK_Signer* s) : signer(s) {}
      ~PK_Signer_Filter() { delete signer; }
   private:
      PK_Signer* signer;
   };

/*************************************************
* PK_Verifier Filter                             *
*************************************************/
class BOTAN_DLL PK_Verifier_Filter : public Filter
   {
   public:
      void write(const byte[], length_type);
      void end_msg();

      void set_signature(const byte[], length_type);
      void set_signature(const MemoryRegion<byte>&);

      PK_Verifier_Filter(PK_Verifier* v) : verifier(v) {}
      PK_Verifier_Filter(PK_Verifier*, const byte[], length_type);
      PK_Verifier_Filter(PK_Verifier*, const MemoryRegion<byte>&);
      ~PK_Verifier_Filter() { delete verifier; }
   private:
      PK_Verifier* verifier;
      SecureVector<byte> signature;
   };

}

#endif
