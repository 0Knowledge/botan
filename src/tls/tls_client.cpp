/*
* TLS Client
* (C) 2004-2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#include <botan/tls_client.h>
#include <botan/internal/tls_handshake_state.h>
#include <botan/internal/tls_messages.h>
#include <botan/internal/stl_util.h>
#include <memory>

namespace Botan {

namespace TLS {

/*
* TLS Client Constructor
*/
Client::Client(std::tr1::function<void (const byte[], size_t)> output_fn,
               std::tr1::function<void (const byte[], size_t, Alert)> proc_fn,
               std::tr1::function<bool (const Session&)> handshake_fn,
               Session_Manager& session_manager,
               Credentials_Manager& creds,
               const Policy& policy,
               RandomNumberGenerator& rng,
               const std::string& hostname,
               std::tr1::function<std::string (std::vector<std::string>)> next_protocol) :
   Channel(output_fn, proc_fn, handshake_fn),
   policy(policy),
   rng(rng),
   session_manager(session_manager),
   creds(creds),
   m_hostname(hostname)
   {
   m_writer.set_version(Protocol_Version::SSL_V3);

   m_state = new Handshake_State(new Stream_Handshake_Reader);
   m_state->set_expected_next(SERVER_HELLO);

   m_state->client_npn_cb = next_protocol;

   const std::string srp_identifier = creds.srp_identifier("tls-client", hostname);

   const bool send_npn_request = static_cast<bool>(next_protocol);

   if(hostname != "")
      {
      Session session_info;
      if(session_manager.load_from_host_info(hostname, 0, session_info))
         {
         if(session_info.srp_identifier() == srp_identifier)
            {
            m_state->client_hello = new Client_Hello(
               m_writer,
               m_state->hash,
               policy,
               rng,
               m_secure_renegotiation.for_client_hello(),
               session_info,
               send_npn_request);

            m_state->resume_master_secret = session_info.master_secret();
            }
         }
      }

   if(!m_state->client_hello) // not resuming
      {
      m_state->client_hello = new Client_Hello(
         m_writer,
         m_state->hash,
         policy,
         rng,
         m_secure_renegotiation.for_client_hello(),
         send_npn_request,
         hostname,
         srp_identifier);
      }

   m_secure_renegotiation.update(m_state->client_hello);
   }

/*
* Send a new client hello to renegotiate
*/
void Client::renegotiate(bool force_full_renegotiation)
   {
   if(m_state && m_state->client_hello)
      return; // currently in active handshake

   delete m_state;
   m_state = new Handshake_State(new Stream_Handshake_Reader);

   m_state->set_expected_next(SERVER_HELLO);

   if(!force_full_renegotiation)
      {
      Session session_info;
      if(session_manager.load_from_host_info(m_hostname, 0, session_info))
         {
         m_state->client_hello = new Client_Hello(
            m_writer,
            m_state->hash,
            policy,
            rng,
            m_secure_renegotiation.for_client_hello(),
            session_info);

         m_state->resume_master_secret = session_info.master_secret();
         }
      }

   if(!m_state->client_hello)
      {
      m_state->client_hello = new Client_Hello(
         m_writer,
         m_state->hash,
         policy,
         rng,
         m_secure_renegotiation.for_client_hello());
      }

   m_secure_renegotiation.update(m_state->client_hello);
   }

void Client::alert_notify(const Alert& alert)
   {
   if(alert.type() == Alert::NO_RENEGOTIATION)
      {
      if(m_handshake_completed && m_state)
         {
         delete m_state;
         m_state = 0;
         }
      }
   }

/*
* Process a handshake message
*/
void Client::process_handshake_msg(Handshake_Type type,
                                   const MemoryRegion<byte>& contents)
   {
   if(m_state == 0)
      throw Unexpected_Message("Unexpected handshake message from server");

   if(type == HELLO_REQUEST)
      {
      Hello_Request hello_request(contents);

      // Ignore request entirely if we are currently negotiating a handshake
      if(m_state->client_hello)
         return;

      if(!m_secure_renegotiation.supported() && policy.require_secure_renegotiation())
         {
         delete m_state;
         m_state = 0;

         // RFC 5746 section 4.2
         send_alert(Alert(Alert::NO_RENEGOTIATION));
         return;
         }

      renegotiate(false);

      return;
      }

   m_state->confirm_transition_to(type);

   if(type != HANDSHAKE_CCS && type != FINISHED)
      m_state->hash.update(type, contents);

   if(type == SERVER_HELLO)
      {
      m_state->server_hello = new Server_Hello(contents);

      if(!m_state->client_hello->offered_suite(m_state->server_hello->ciphersuite()))
         {
         throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                             "Server replied with ciphersuite we didn't send");
         }

      if(!value_exists(m_state->client_hello->compression_methods(),
                       m_state->server_hello->compression_method()))
         {
         throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                             "Server replied with compression method we didn't send");
         }

      if(!m_state->client_hello->next_protocol_notification() &&
         m_state->server_hello->next_protocol_notification())
         {
         throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                             "Server sent next protocol but we didn't request it");
         }

      if(m_state->server_hello->supports_session_ticket())
         {
         if(!m_state->client_hello->supports_session_ticket())
            throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                                "Server sent session ticket extension but we did not");
         }

      m_state->set_version(m_state->server_hello->version());

      m_writer.set_version(m_state->version());
      m_reader.set_version(m_state->version());

      m_secure_renegotiation.update(m_state->server_hello);

      m_peer_supports_heartbeats = m_state->server_hello->supports_heartbeats();
      m_heartbeat_sending_allowed = m_state->server_hello->peer_can_send_heartbeats();

      m_state->suite = Ciphersuite::by_id(m_state->server_hello->ciphersuite());

      const bool server_returned_same_session_id =
         !m_state->server_hello->session_id().empty() &&
         (m_state->server_hello->session_id() == m_state->client_hello->session_id());

      if(server_returned_same_session_id)
         {
         // successful resumption

         /*
         * In this case, we offered the version used in the original
         * session, and the server must resume with the same version.
         */
         if(m_state->server_hello->version() != m_state->client_hello->version())
            throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                                "Server resumed session but with wrong version");

         m_state->keys = Session_Keys(m_state,
                                      m_state->resume_master_secret,
                                      true);

         if(m_state->server_hello->supports_session_ticket())
            m_state->set_expected_next(NEW_SESSION_TICKET);
         else
            m_state->set_expected_next(HANDSHAKE_CCS);
         }
      else
         {
         // new session

         if(m_state->version() > m_state->client_hello->version())
            {
            throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                                "Client: Server replied with bad version");
            }

         if(m_state->version() < policy.min_version())
            {
            throw TLS_Exception(Alert::PROTOCOL_VERSION,
                                "Client: Server is too old for specified policy");
            }

         if(m_state->suite.sig_algo() != "")
            {
            m_state->set_expected_next(CERTIFICATE);
            }
         else if(m_state->suite.kex_algo() == "PSK")
            {
            /* PSK is anonymous so no certificate/cert req message is
            ever sent. The server may or may not send a server kex,
            depending on if it has an identity hint for us.

            (EC)DHE_PSK always sends a server key exchange for the
            DH exchange portion.
            */

            m_state->set_expected_next(SERVER_KEX);
            m_state->set_expected_next(SERVER_HELLO_DONE);
            }
         else if(m_state->suite.kex_algo() != "RSA")
            {
            m_state->set_expected_next(SERVER_KEX);
            }
         else
            {
            m_state->set_expected_next(CERTIFICATE_REQUEST); // optional
            m_state->set_expected_next(SERVER_HELLO_DONE);
            }
         }
      }
   else if(type == CERTIFICATE)
      {
      if(m_state->suite.kex_algo() != "RSA")
         {
         m_state->set_expected_next(SERVER_KEX);
         }
      else
         {
         m_state->set_expected_next(CERTIFICATE_REQUEST); // optional
         m_state->set_expected_next(SERVER_HELLO_DONE);
         }

      m_state->server_certs = new Certificate(contents);

      m_peer_certs = m_state->server_certs->cert_chain();
      if(m_peer_certs.size() == 0)
         throw TLS_Exception(Alert::HANDSHAKE_FAILURE,
                             "Client: No certificates sent by server");

      try
         {
         creds.verify_certificate_chain("tls-client", m_hostname, m_peer_certs);
         }
      catch(std::exception& e)
         {
         throw TLS_Exception(Alert::BAD_CERTIFICATE, e.what());
         }

      std::auto_ptr<Public_Key> peer_key(m_peer_certs[0].subject_public_key());

      if(peer_key->algo_name() != m_state->suite.sig_algo())
         throw TLS_Exception(Alert::ILLEGAL_PARAMETER,
                             "Certificate key type did not match ciphersuite");
      }
   else if(type == SERVER_KEX)
      {
      m_state->set_expected_next(CERTIFICATE_REQUEST); // optional
      m_state->set_expected_next(SERVER_HELLO_DONE);

      m_state->server_kex = new Server_Key_Exchange(contents,
                                                    m_state->suite.kex_algo(),
                                                    m_state->suite.sig_algo(),
                                                    m_state->version());

      if(m_state->suite.sig_algo() != "")
         {
         if(!m_state->server_kex->verify(m_peer_certs[0], m_state))
            {
            throw TLS_Exception(Alert::DECRYPT_ERROR,
                                "Bad signature on server key exchange");
            }
         }
      }
   else if(type == CERTIFICATE_REQUEST)
      {
      m_state->set_expected_next(SERVER_HELLO_DONE);
      m_state->cert_req = new Certificate_Req(contents, m_state->version());
      }
   else if(type == SERVER_HELLO_DONE)
      {
      m_state->server_hello_done = new Server_Hello_Done(contents);

      if(m_state->received_handshake_msg(CERTIFICATE_REQUEST))
         {
         const std::vector<std::string>& types =
            m_state->cert_req->acceptable_cert_types();

         std::vector<X509_Certificate> client_certs =
            creds.cert_chain(types,
                             "tls-client",
                             m_hostname);

         m_state->client_certs = new Certificate(m_writer,
                                                 m_state->hash,
                                                 client_certs);
         }

      m_state->client_kex =
         new Client_Key_Exchange(m_writer,
                                 m_state,
                                 creds,
                                 m_peer_certs,
                                 m_hostname,
                                 rng);

      m_state->keys = Session_Keys(m_state,
                                   m_state->client_kex->pre_master_secret(),
                                   false);

      if(m_state->received_handshake_msg(CERTIFICATE_REQUEST) &&
         !m_state->client_certs->empty())
         {
         Private_Key* private_key =
            creds.private_key_for(m_state->client_certs->cert_chain()[0],
                                  "tls-client",
                                  m_hostname);

         m_state->client_verify = new Certificate_Verify(m_writer,
                                                         m_state,
                                                         rng,
                                                         private_key);
         }

      m_writer.send(CHANGE_CIPHER_SPEC, 1);

      m_writer.activate(CLIENT, m_state->suite, m_state->keys,
                        m_state->server_hello->compression_method());

      if(m_state->server_hello->next_protocol_notification())
         {
         const std::string protocol =
            m_state->client_npn_cb(m_state->server_hello->next_protocols());

         m_state->next_protocol = new Next_Protocol(m_writer, m_state->hash, protocol);
         }

      m_state->client_finished = new Finished(m_writer, m_state, CLIENT);

      if(m_state->server_hello->supports_session_ticket())
         m_state->set_expected_next(NEW_SESSION_TICKET);
      else
         m_state->set_expected_next(HANDSHAKE_CCS);
      }
   else if(type == NEW_SESSION_TICKET)
      {
      m_state->new_session_ticket = new New_Session_Ticket(contents);

      m_state->set_expected_next(HANDSHAKE_CCS);
      }
   else if(type == HANDSHAKE_CCS)
      {
      m_state->set_expected_next(FINISHED);

      m_reader.activate(CLIENT, m_state->suite, m_state->keys,
                        m_state->server_hello->compression_method());
      }
   else if(type == FINISHED)
      {
      m_state->set_expected_next(HELLO_REQUEST);

      m_state->server_finished = new Finished(contents);

      if(!m_state->server_finished->verify(m_state, SERVER))
         throw TLS_Exception(Alert::DECRYPT_ERROR,
                             "Finished message didn't verify");

      m_state->hash.update(type, contents);

      if(!m_state->client_finished) // session resume case
         {
         m_writer.send(CHANGE_CIPHER_SPEC, 1);

         m_writer.activate(CLIENT, m_state->suite, m_state->keys,
                           m_state->server_hello->compression_method());

         m_state->client_finished = new Finished(m_writer, m_state, CLIENT);
         }

      m_secure_renegotiation.update(m_state->client_finished, m_state->server_finished);

      MemoryVector<byte> session_id = m_state->server_hello->session_id();

      const MemoryRegion<byte>& session_ticket = m_state->session_ticket();

      if(session_id.empty() && !session_ticket.empty())
         session_id = make_hello_random(rng);

      Session session_info(
         session_id,
         m_state->keys.master_secret(),
         m_state->server_hello->version(),
         m_state->server_hello->ciphersuite(),
         m_state->server_hello->compression_method(),
         CLIENT,
         m_secure_renegotiation.supported(),
         m_state->server_hello->fragment_size(),
         m_peer_certs,
         session_ticket,
         m_hostname,
         ""
         );

      if(m_handshake_fn(session_info))
         session_manager.save(session_info);
      else
         session_manager.remove_entry(session_info.session_id());

      delete m_state;
      m_state = 0;
      m_handshake_completed = true;
      }
   else
      throw Unexpected_Message("Unknown handshake message received");
   }

}

}
