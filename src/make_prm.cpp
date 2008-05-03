/*************************************************
* Prime Generation Source File                   *
* (C) 1999-2007 Jack Lloyd                       *
*************************************************/

#include <botan/numthry.h>
#include <botan/parsing.h>
#include <botan/libstate.h>
#include <algorithm>

namespace Botan {

/*************************************************
* Generate a random prime                        *
*************************************************/
BigInt random_prime(length_type bits, const BigInt& coprime,
                    length_type equiv, length_type modulo)
   {
   if(bits < 48)
      throw Invalid_Argument("random_prime: Can't make a prime of " +
                             to_string(bits) + " bits");

   if(coprime <= 0)
      throw Invalid_Argument("random_prime: coprime must be > 0");
   if(modulo % 2 == 1 || modulo == 0)
      throw Invalid_Argument("random_prime: Invalid modulo value");
   if(equiv >= modulo || equiv % 2 == 0)
      throw Invalid_Argument("random_prime: equiv must be < modulo, and odd");

   while(true)
      {
      BigInt p = random_integer(bits);
      p.set_bit(bits - 2);
      p.set_bit(0);

      if(p % modulo != equiv)
         p += (modulo - p % modulo) + equiv;

      const length_type sieve_size = std::min(bits / 2, PRIME_TABLE_SIZE);
      SecureVector<length_type> sieve(sieve_size);

      for(length_type j = 0; j != sieve.size(); ++j)
         sieve[j] = p % PRIMES[j];

      length_type counter = 0;
      while(true)
         {
         if(counter == 4096 || p.bits() > bits)
            break;

         bool passes_sieve = true;
         ++counter;
         p += modulo;

         for(length_type j = 0; j != sieve.size(); ++j)
            {
            sieve[j] = (sieve[j] + modulo) % PRIMES[j];
            if(sieve[j] == 0)
               passes_sieve = false;
            }

         if(!passes_sieve || gcd(p - 1, coprime) != 1)
            continue;
         if(passes_mr_tests(p))
            return p;
         }
      }
   }

}
