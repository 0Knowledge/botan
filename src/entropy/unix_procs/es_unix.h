/*************************************************
* Unix EntropySource Header File                 *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#ifndef BOTAN_ENTROPY_SRC_UNIX_H__
#define BOTAN_ENTROPY_SRC_UNIX_H__

#include <botan/buf_es.h>
#include <botan/unix_cmd.h>
#include <vector>

namespace Botan {

/*************************************************
* Unix Entropy Source                            *
*************************************************/
class Unix_EntropySource : public Buffered_EntropySource
   {
   public:
      void add_sources(const Unix_Program[], length_type);
      Unix_EntropySource(const std::vector<std::string>& path);
   private:
      static void add_default_sources(std::vector<Unix_Program>&);

      void do_fast_poll();
      void do_slow_poll();

      const std::vector<std::string> PATH;
      std::vector<Unix_Program> sources;
   };

}

#endif
