/*************************************************
* Bzip Compressor Header File                    *
* (C) 2001 Peter J Jones                         *
*     2001-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_BZIP2_H__
#define BOTAN_BZIP2_H__

#include <botan/filter.h>

namespace Botan {

/*************************************************
* Bzip Compression Filter                        *
*************************************************/
class Bzip_Compression : public Filter
   {
   public:
      void write(const byte input[], length_type length);
      void start_msg();
      void end_msg();

      void flush();

      Bzip_Compression(length_type = 9);
      ~Bzip_Compression() { clear(); }
   private:
      void clear();

      const length_type level;
      SecureVector<byte> buffer;
      class Bzip_Stream* bz;
   };

/*************************************************
* Bzip Decompression Filter                      *
*************************************************/
class Bzip_Decompression : public Filter
   {
   public:
      void write(const byte input[], length_type length);
      void start_msg();
      void end_msg();

      Bzip_Decompression(bool = false);
      ~Bzip_Decompression() { clear(); }
   private:
      void clear();

      const bool small_mem;
      SecureVector<byte> buffer;
      class Bzip_Stream* bz;
      bool no_writes;
   };

}

#endif
