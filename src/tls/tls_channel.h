/*
* TLS Channel
* (C) 2011 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef BOTAN_TLS_CHANNEL_H__
#define BOTAN_TLS_CHANNEL_H__

#include <botan/tls_policy.h>
#include <botan/tls_record.h>
#include <botan/tls_session.h>
#include <botan/tls_alert.h>
#include <botan/x509cert.h>
#include <vector>

namespace Botan {

namespace TLS {

/**
* Generic interface for TLS endpoint
*/
class BOTAN_DLL Channel
   {
   public:
      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      virtual size_t received_data(const byte buf[], size_t buf_size);

      /**
      * Inject plaintext intended for counterparty
      */
      virtual void send(const byte buf[], size_t buf_size);

      /**
      * Send a close notification alert
      */
      void close() { send_alert(Alert(Alert::CLOSE_NOTIFY)); }

      /**
      * @return true iff the connection is active for sending application data
      */
      bool is_active() const { return m_handshake_completed && !is_closed(); }

      /**
      * @return true iff the connection has been definitely closed
      */
      bool is_closed() const { return m_connection_closed; }

      /**
      * Attempt to renegotiate the session
      * @param force_full_renegotiation if true, require a full renegotiation,
      *                                 otherwise allow session resumption
      */
      virtual void renegotiate(bool force_full_renegotiation) = 0;

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      * @param payload will be echoed back
      * @param countents_size size of payload in bytes
      */
      void heartbeat(const byte payload[], size_t payload_size);

      /**
      * Attempt to send a heartbeat message (if negotiated with counterparty)
      */
      void heartbeat() { heartbeat(0, 0); }

      /**
      * @return certificate chain of the peer (may be empty)
      */
      std::vector<X509_Certificate> peer_cert_chain() const { return m_peer_certs; }

      Channel(std::tr1::function<void (const byte[], size_t)> socket_output_fn,
              std::tr1::function<void (const byte[], size_t, Alert)> proc_fn,
              std::tr1::function<bool (const Session&)> handshake_complete);

      virtual ~Channel();
   protected:

      /**
      * Send a TLS alert message. If the alert is fatal, the
      * internal state (keys, etc) will be reset
      * @param level is warning or fatal
      * @param type is the type of alert
      */
      void send_alert(const Alert& alert);

      virtual void read_handshake(byte rec_type,
                                  const MemoryRegion<byte>& rec_buf);

      virtual void process_handshake_msg(Handshake_Type type,
                                         const MemoryRegion<byte>& contents) = 0;

      virtual void alert_notify(const Alert& alert) = 0;

      std::tr1::function<void (const byte[], size_t, Alert)> m_proc_fn;
      std::tr1::function<bool (const Session&)> m_handshake_fn;

      Record_Writer m_writer;
      Record_Reader m_reader;

      std::vector<X509_Certificate> m_peer_certs;

      class Handshake_State* m_state;

      class Secure_Renegotiation_State
         {
         public:
            Secure_Renegotiation_State() : initial_handshake(true),
                                           secure_renegotiation(false)
               {}

            void update(class Client_Hello* client_hello);
            void update(class Server_Hello* server_hello);

            void update(class Finished* client_finished,
                        class Finished* server_finished);

            const MemoryVector<byte>& for_client_hello() const
               { return client_verify; }

            MemoryVector<byte> for_server_hello() const
               {
               MemoryVector<byte> buf = client_verify;
               buf += server_verify;
               return buf;
               }

            bool supported() const { return secure_renegotiation; }
            bool renegotiation() const { return !initial_handshake; }
         private:
            bool initial_handshake;
            bool secure_renegotiation;
            MemoryVector<byte> client_verify, server_verify;
         };

      Secure_Renegotiation_State m_secure_renegotiation;

      bool m_handshake_completed;
      bool m_connection_closed;
      bool m_peer_supports_heartbeats;
      bool m_heartbeat_sending_allowed;
   };

}

}

#endif
