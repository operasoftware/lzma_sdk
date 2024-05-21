#include "StdAfx.h"

#ifdef OPERA_CUSTOM_CODE

#include "Flags.h"

#include "../../../../Common/MyString.h"

namespace opera::flags {
namespace {
const UString kServerTrackingBlob = UString("--server-tracking-blob=");
const UString kCustomizationPackage = UString("--customization-package=");
}  // namespace

const UString& ServerTrackingBlob() {
  return kServerTrackingBlob;
}

const UString& CustomizationPackage() {
  return kCustomizationPackage;
}

}  // namespace opera::flags

#endif OPERA_CUSTOM_CODE
