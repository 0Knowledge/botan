/*
* TLS Record Writing
* (C) 2004-2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#include <botan/tls_record.h>
#include <botan/internal/tls_session_key.h>
#include <botan/internal/tls_handshake_hash.h>
#include <botan/lookup.h>
#include <botan/internal/rounding.h>
#include <botan/internal/assert.h>
#include <botan/loadstor.h>
#include <botan/libstate.h>

#if defined(BOTAN_HAS_COMPRESSOR_ZLIB)
  #include <botan/zlib.h>
#endif

#include <stdio.h>
#include <botan/hex.h>

namespace Botan {

namespace TLS {

/*
* Record_Writer Constructor
*/
Record_Writer::Record_Writer(std::tr1::function<void (const byte[], size_t)> out) :
   m_output_fn(out), m_writebuf(TLS_HEADER_SIZE + MAX_CIPHERTEXT_SIZE)
   {
   m_compressor_filter = 0;
   m_mac = 0;
   reset();
   set_maximum_fragment_size(0);
   }

void Record_Writer::set_maximum_fragment_size(size_t max_fragment)
   {
   if(max_fragment == 0)
      m_max_fragment = MAX_PLAINTEXT_SIZE;
   else
      m_max_fragment = clamp(max_fragment, 128, MAX_PLAINTEXT_SIZE);
   }

/*
* Reset the state
*/
void Record_Writer::reset()
   {
   set_maximum_fragment_size(0);
   m_cipher.reset();

   try
      {
      m_compressor.end_msg();
      }
   catch(...) {}

   try
      {
      m_compressor.reset();
      }
   catch(...) {}

   delete m_mac;
   m_mac = 0;

   m_compressor_filter = 0;

   zeroise(m_writebuf);

   m_version = Protocol_Version();
   m_block_size = 0;
   m_mac_size = 0;
   m_iv_size = 0;

   m_seq_no = 0;
   }

/*
* Set the version to use
*/
void Record_Writer::set_version(Protocol_Version version)
   {
   m_version = version;
   }

/*
* Set the keys for writing
*/
void Record_Writer::activate(const Ciphersuite& suite,
                             const Session_Keys& keys,
                             Connection_Side side,
                             byte compression_method)
   {
   m_cipher.reset();
   delete m_mac;
   m_mac = 0;

   /*
   RFC 4346:
     A sequence number is incremented after each record: specifically,
     the first record transmitted under a particular connection state
     MUST use sequence number 0
   */
   m_seq_no = 0;

   SymmetricKey mac_key, cipher_key;
   InitializationVector iv;

   if(side == CLIENT)
      {
      cipher_key = keys.client_cipher_key();
      iv = keys.client_iv();
      mac_key = keys.client_mac_key();
      }
   else
      {
      cipher_key = keys.server_cipher_key();
      iv = keys.server_iv();
      mac_key = keys.server_mac_key();
      }

   const std::string cipher_algo = suite.cipher_algo();
   const std::string mac_algo = suite.mac_algo();

   if(have_block_cipher(cipher_algo))
      {
      m_cipher.append(get_cipher(
                       cipher_algo + "/CBC/NoPadding",
                       cipher_key, iv, ENCRYPTION)
         );
      m_block_size = block_size_of(cipher_algo);

      if(m_version >= Protocol_Version::TLS_V11)
         m_iv_size = m_block_size;
      else
         m_iv_size = 0;
      }
   else if(have_stream_cipher(cipher_algo))
      {
      m_cipher.append(get_cipher(cipher_algo, cipher_key, ENCRYPTION));
      m_block_size = 0;
      m_iv_size = 0;
      }
   else
      throw Invalid_Argument("Record_Writer: Unknown cipher " + cipher_algo);

   if(have_hash(mac_algo))
      {
      Algorithm_Factory& af = global_state().algorithm_factory();

      if(m_version == Protocol_Version::SSL_V3)
         m_mac = af.make_mac("SSL3-MAC(" + mac_algo + ")");
      else
         m_mac = af.make_mac("HMAC(" + mac_algo + ")");

      m_mac->set_key(mac_key);
      m_mac_size = m_mac->output_length();
      }
   else
      throw Invalid_Argument("Record_Writer: Unknown hash " + mac_algo);

   if(compression_method == DEFLATE_COMPRESSION)
      {
#if defined(BOTAN_HAS_COMPRESSOR_ZLIB)
      m_compressor_filter = new Zlib_Compression(1, true);
      m_compressor.append(m_compressor_filter);
#else
      throw TLS_Exception(INTERNAL_ERROR,
                          "Negotiated deflate, but zlib not available");
#endif
      }
   else if(compression_method != NO_COMPRESSION)
      throw TLS_Exception(INTERNAL_ERROR,
                          "Negotiated an unknown compression method");

   m_compressor.start_msg();
   }

/*
* Send one or more records to the other side
*/
void Record_Writer::send(byte type, const byte input[], size_t length)
   {
   if(length == 0)
      return;

   /*
   * If using CBC mode in SSLv3/TLS v1.0, send a single byte of
   * plaintext to randomize the (implicit) IV of the following main
   * block. If using a stream cipher, or TLS v1.1, this isn't
   * necessary.
   *
   * An empty record also works but apparently some implementations do
   * not like this (https://bugzilla.mozilla.org/show_bug.cgi?id=665814)
   *
   * See http://www.openssl.org/~bodo/tls-cbc.txt for background.
   */
   if((type == APPLICATION) && (m_block_size > 0) && (m_iv_size == 0))
      {
      send_record(type, &input[0], 1);
      input += 1;
      length -= 1;
      }

   while(length)
      {
      const size_t sending = std::min(length, m_max_fragment);
      send_record(type, &input[0], sending);

      input += sending;
      length -= sending;
      }
   }

/*
* Encrypt and send the record
*/
void Record_Writer::send_record(byte type, const byte input[], size_t length)
   {
   if(length >= MAX_PLAINTEXT_SIZE)
      throw Internal_Error("Record_Writer: Compressed packet is too big");

   if(m_mac_size == 0)
      {
      const byte header[TLS_HEADER_SIZE] = {
         type,
         m_version.major_version(),
         m_version.minor_version(),
         get_byte<u16bit>(0, length),
         get_byte<u16bit>(1, length)
      };

      m_output_fn(header, TLS_HEADER_SIZE);
      m_output_fn(input, length);
      }
   else
      {
      m_compressor.write(buf, length);

      if(m_compressor_filter)
         {
         // FIXME!
         dynamic_cast<Zlib_Compression*>(m_compressor_filter)->flush();
         }

      SecureVector<byte> plaintext = m_compressor.read_all(Pipe::LAST_MESSAGE);

      printf("Uncompressed plaintext %s\n", hex_encode(buf, length).c_str());
      printf("Compressed plaintext: %s\n", hex_encode(plaintext).c_str());

      m_mac->update_be(m_seq_no);
      m_mac->update(type);

      if(m_version != Protocol_Version::SSL_V3)
         {
         m_mac->update(m_version.major_version());
         m_mac->update(m_version.minor_version());
         }

      m_mac->update(get_byte<u16bit>(0, length));
      m_mac->update(get_byte<u16bit>(1, length));
      m_mac->update(input, length);

      const size_t buf_size = round_up(m_iv_size + length +
                                       m_mac->output_length() +
                                       (m_block_size ? 1 : 0),
                                       m_block_size);

      if(buf_size >= MAX_CIPHERTEXT_SIZE)
         throw Internal_Error("Record_Writer: Record is too big");

      BOTAN_ASSERT(m_writebuf.size() >= TLS_HEADER_SIZE + MAX_CIPHERTEXT_SIZE,
                   "Write buffer is big enough");

      // TLS record header
      m_writebuf[0] = type;
      m_writebuf[1] = m_version.major_version();
      m_writebuf[2] = m_version.minor_version();
      m_writebuf[3] = get_byte<u16bit>(0, buf_size);
      m_writebuf[4] = get_byte<u16bit>(1, buf_size);

      byte* buf_write_ptr = &m_writebuf[TLS_HEADER_SIZE];

      if(m_iv_size)
         {
         RandomNumberGenerator& rng = global_state().global_rng();
         rng.randomize(buf_write_ptr, m_iv_size);
         buf_write_ptr += m_iv_size;
         }

      copy_mem(buf_write_ptr, input, length);
      buf_write_ptr += length;

      m_mac->final(buf_write_ptr);
      buf_write_ptr += m_mac->output_length();

      if(m_block_size)
         {
         const size_t pad_val =
            buf_size - (m_iv_size + length + m_mac->output_length() + 1);

         for(size_t i = 0; i != pad_val + 1; ++i)
            {
            *buf_write_ptr = pad_val;
            buf_write_ptr += 1;
            }
         }

      // FIXME: this could be done in-place without copying
      m_cipher.process_msg(&m_writebuf[TLS_HEADER_SIZE], buf_size);
      const size_t got_back = m_cipher.read(&m_writebuf[TLS_HEADER_SIZE], buf_size, Pipe::LAST_MESSAGE);

      BOTAN_ASSERT_EQUAL(got_back, buf_size, "Cipher encrypted full amount");

      BOTAN_ASSERT_EQUAL(m_cipher.remaining(Pipe::LAST_MESSAGE), 0,
                         "No data remains in pipe");

      m_output_fn(&m_writebuf[0], TLS_HEADER_SIZE + buf_size);

      m_seq_no++;
      }
   }

/*
* Send an alert
*/
void Record_Writer::alert(Alert_Level level, Alert_Type type)
   {
   byte alert[2] = { level, type };
   send(ALERT, alert, sizeof(alert));
   }

}

}
