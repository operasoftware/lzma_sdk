#ifdef OPERA_CUSTOM_CODE

#ifndef OPERA_PAYLOAD_FLAG_H_
#define OPERA_PAYLOAD_FLAG_H_

#include "../../../../Common/MyString.h"

namespace opera
{
  class PayloadFlag
  {
  public:
    static const UString& GetPayloadFlag();
    static bool HavePayloadFlag(UString);
  };
}
#endif // OPERA_PAYLOAD_FLAG_H_

#endif // OPERA_CUSTOM_CODE
