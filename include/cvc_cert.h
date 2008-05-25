/*************************************************
* EAC1_1 CVC Header File                         *
* (C) 2008 Falko Strenzke                        *
*          strenzke@flexsecure.de                *
*************************************************/

#ifndef BOTAN_CVC_EAC_H__
#define BOTAN_CVC_EAC_H__

#include <botan/x509_key.h>
#include <botan/enums.h>
#include <botan/signed_obj.h>
#include <string>
#include <botan/pubkey.h>
#include <botan/ecdsa.h>
#include <botan/ec.h>
#include <botan/eac_obj.h>
#include <botan/cvc_gen_cert.h>
namespace Botan
  {


  /**
  * This class represents TR03110 (EAC) v1.1 CV Certificates
  */
  class EAC1_1_CVC : public EAC1_1_gen_CVC<EAC1_1_CVC>//Signed_Object
    {
        friend class EAC1_1_obj<EAC1_1_CVC>;

    public:

    /**
    * Get the CAR of the certificate.
    * @result the CAR of the certificate
    */
      ASN1_Car get_car() const;

    /**
    * Get the CED of this certificate.
    * @result the CED this certificate
    */
      ASN1_Ced get_ced() const;
    /**
    * Get the CEX of this certificate.
    * @result the CEX this certificate
    */
      ASN1_Cex get_cex() const;

      /**
      * Get the CHAT value.
      * @result the CHAT value
      */
      u32bit get_chat_value() const;

      bool operator==(const EAC1_1_CVC&) const;

    /**
    * Construct a CVC from a data source
    * @param source the data source
    */
      EAC1_1_CVC(std::tr1::shared_ptr<DataSource>& source);
      /**
    * Construct a CVC from a file
    * @param str the path to the certificate file
    */
      EAC1_1_CVC(const std::string& str);
      virtual ~EAC1_1_CVC()
      {}

    protected:

    private:
      void force_decode();
      friend class EAC1_1_CVC_CA;
      EAC1_1_CVC()
      {}
      ASN1_Car m_car;
      ASN1_Ced m_ced;
      ASN1_Cex m_cex;
      byte m_chat_val;
      OID m_chat_oid;


    };

  /*************************************************
  *Comparison                                      *
  *************************************************/
  inline bool operator!=(EAC1_1_CVC const& lhs, EAC1_1_CVC const& rhs)
  {
    return !(lhs == rhs);
  }


}

#endif

