#include "StdAfx.h"

#ifdef OPERA_CUSTOM_CODE

#include "PayloadFlag.h"

static const UString kPayloadFlag = UString("--server-tracking-blob=");

const UString& opera::PayloadFlag::GetPayloadFlag()
{
  return kPayloadFlag;
}

bool opera::PayloadFlag::HavePayloadFlag(UString str)
{
  return str.Find(kPayloadFlag) >= 0;
}

#endif OPERA_CUSTOM_CODE
