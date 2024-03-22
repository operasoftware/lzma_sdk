#include "StdAfx.h"

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
