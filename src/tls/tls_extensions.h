/*
* TLS Extensions
* (C) 2011-2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_EXTENSIONS_H__
#define BOTAN_TLS_EXTENSIONS_H__

#include <botan/secmem.h>
#include <botan/tls_magic.h>
#include <vector>
#include <string>
#include <map>

namespace Botan {

namespace TLS {

class TLS_Data_Reader;

enum Handshake_Extension_Type {
   TLSEXT_SERVER_NAME_INDICATION = 0,
   TLSEXT_MAX_FRAGMENT_LENGTH    = 1,
   TLSEXT_CLIENT_CERT_URL        = 2,
   TLSEXT_TRUSTED_CA_KEYS        = 3,
   TLSEXT_TRUNCATED_HMAC         = 4,

   TLSEXT_CERTIFICATE_TYPES      = 9,
   TLSEXT_USABLE_ELLIPTIC_CURVES = 10,
   TLSEXT_EC_POINT_FORMATS       = 11,
   TLSEXT_SRP_IDENTIFIER         = 12,
   TLSEXT_SIGNATURE_ALGORITHMS   = 13,
   TLSEXT_HEARTBEAT_SUPPORT      = 15,

   TLSEXT_SESSION_TICKET         = 35,

   TLSEXT_NEXT_PROTOCOL          = 13172,

   TLSEXT_SAFE_RENEGOTIATION     = 65281,
};

/**
* Base class representing a TLS extension of some kind
*/
class Extension
   {
   public:
      virtual Handshake_Extension_Type type() const = 0;

      virtual MemoryVector<byte> serialize() const = 0;

      virtual bool empty() const = 0;

      virtual ~Extension() {}
   };

/**
* Server Name Indicator extension (RFC 3546)
*/
class Server_Name_Indicator : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SERVER_NAME_INDICATION; }

      Handshake_Extension_Type type() const { return static_type(); }

      Server_Name_Indicator(const std::string& host_name) :
         sni_host_name(host_name) {}

      Server_Name_Indicator(const MemoryRegion<byte>& buf);

      std::string host_name() const { return sni_host_name; }

      MemoryVector<byte> serialize() const;

      bool empty() const { return sni_host_name == ""; }
   private:
      std::string sni_host_name;
   };

/**
* SRP identifier extension (RFC 5054)
*/
class SRP_Identifier : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SRP_IDENTIFIER; }

      Handshake_Extension_Type type() const { return static_type(); }

      SRP_Identifier(const std::string& identifier) :
         srp_identifier(identifier) {}

      SRP_Identifier(const MemoryRegion<byte>& buf);

      std::string identifier() const { return srp_identifier; }

      MemoryVector<byte> serialize() const;

      bool empty() const { return srp_identifier == ""; }
   private:
      std::string srp_identifier;
   };

/**
* Renegotiation Indication Extension (RFC 5746)
*/
class Renegotation_Extension : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SAFE_RENEGOTIATION; }

      Handshake_Extension_Type type() const { return static_type(); }

      Renegotation_Extension() {}

      Renegotation_Extension(const MemoryRegion<byte>& buf,
                             bool decoding = false);

      const MemoryVector<byte>& renegotiation_info() const
         { return reneg_data; }

      MemoryVector<byte> serialize() const;

      bool empty() const { return false; } // always send this
   private:
      MemoryVector<byte> reneg_data;
   };

/**
* Maximum Fragment Length Negotiation Extension (RFC 4366 sec 3.2)
*/
class Maximum_Fragment_Length : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_MAX_FRAGMENT_LENGTH; }

      Handshake_Extension_Type type() const { return static_type(); }

      bool empty() const { return val != 0; }

      size_t fragment_size() const;

      MemoryVector<byte> serialize() const
         {
         return MemoryVector<byte>(&val, 1);
         }

      /**
      * @param max_fragment specifies what maximum fragment size to
      *        advertise. Currently must be one of 512, 1024, 2048, or
      *        4096.
      */
      Maximum_Fragment_Length(size_t max_fragment);

      Maximum_Fragment_Length(const MemoryRegion<byte>& buf);

   private:
      byte val;
   };

/**
* Next Protocol Negotiation
* http://technotes.googlecode.com/git/nextprotoneg.html
*
* This implementation requires the semantics defined in the Google
* spec (implemented in Chromium); the internet draft leaves the format
* unspecified.
*/
class Next_Protocol_Notification : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_NEXT_PROTOCOL; }

      Handshake_Extension_Type type() const { return static_type(); }

      const std::vector<std::string>& protocols() const
         { return m_protocols; }

      /**
      * Empty extension, used by client
      */
      Next_Protocol_Notification() {}

      /**
      * List of protocols, used by server
      */
      Next_Protocol_Notification(const std::vector<std::string>& protocols) :
         m_protocols(protocols) {}

      Next_Protocol_Notification(const MemoryRegion<byte>& buf);

      MemoryVector<byte> serialize() const;

      bool empty() const { return false; }
   private:
      std::vector<std::string> m_protocols;
   };

class Session_Ticket : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SESSION_TICKET; }

      Handshake_Extension_Type type() const { return static_type(); }

      const MemoryVector<byte>& contents() const { return m_ticket; }

      /**
      * Create empty extension, used by both client and server
      */
      Session_Ticket() {}

      Session_Ticket(const MemoryRegion<byte>& session_ticket) :
         m_ticket(session_ticket) {}

      MemoryVector<byte> serialize() const { return m_ticket; }

      bool empty() const { return false; }
   private:
      MemoryVector<byte> m_ticket;
   };

/**
* Supported Elliptic Curves Extension (RFC 4492)
*/
class Supported_Elliptic_Curves : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USABLE_ELLIPTIC_CURVES; }

      Handshake_Extension_Type type() const { return static_type(); }

      static std::string curve_id_to_name(u16bit id);
      static u16bit name_to_curve_id(const std::string& name);

      const std::vector<std::string>& curves() const { return m_curves; }

      MemoryVector<byte> serialize() const;

      Supported_Elliptic_Curves(const std::vector<std::string>& curves) :
         m_curves(curves) {}

      Supported_Elliptic_Curves(const MemoryRegion<byte>& buf);

      bool empty() const { return m_curves.empty(); }
   private:
      std::vector<std::string> m_curves;
   };

/**
* Signature Algorithms Extension for TLS 1.2 (RFC 5246)
*/
class Signature_Algorithms : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SIGNATURE_ALGORITHMS; }

      Handshake_Extension_Type type() const { return static_type(); }

      static std::string hash_algo_name(byte code);
      static byte hash_algo_code(const std::string& name);

      static std::string sig_algo_name(byte code);
      static byte sig_algo_code(const std::string& name);

      std::vector<std::pair<std::string, std::string> >
         supported_signature_algorthms() const
         {
         return m_supported_algos;
         }

      MemoryVector<byte> serialize() const;

      bool empty() const { return false; }

      Signature_Algorithms(const std::vector<std::pair<std::string, std::string> >& algos) :
         m_supported_algos(algos) {}

      Signature_Algorithms(const MemoryRegion<byte>& buf);
   private:
      std::vector<std::pair<std::string, std::string> > m_supported_algos;
   };

/**
* Heartbeat Extension (RFC 6520)
*/
class Heartbeat_Support_Indicator : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_HEARTBEAT_SUPPORT; }

      Handshake_Extension_Type type() const { return static_type(); }

      bool peer_allowed_to_send() const { return m_peer_allowed_to_send; }

      MemoryVector<byte> serialize() const;

      bool empty() const { return false; }

      Heartbeat_Support_Indicator(bool peer_allowed_to_send) :
         m_peer_allowed_to_send(peer_allowed_to_send) {}

      Heartbeat_Support_Indicator(const MemoryRegion<byte>& val);

   private:
      bool m_peer_allowed_to_send;
   };

/**
* Represents a block of extensions in a hello message
*/
class Extensions
   {
   public:
      template<typename T>
      T* get() const
         {
         Handshake_Extension_Type type = T::static_type();

         std::map<Handshake_Extension_Type, Extension*>::const_iterator i =
            extensions.find(type);

         if(i != extensions.end())
            return dynamic_cast<T*>(i->second);
         return 0;
         }

      void add(Extension* extn)
         {
         delete extensions[extn->type()]; // or hard error if already exists?
         extensions[extn->type()] = extn;
         }

      MemoryVector<byte> serialize() const;

      Extensions() {}

      Extensions(TLS_Data_Reader& reader); // deserialize

      ~Extensions();
   private:
      Extensions(const Extensions&) {}
      Extensions& operator=(const Extensions&) { return (*this); }

      std::map<Handshake_Extension_Type, Extension*> extensions;
   };

}

}

#endif
