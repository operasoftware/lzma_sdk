#ifdef OPERA_CUSTOM_CODE

#ifndef OPERA_FLAGS_H_
#define OPERA_FLAGS_H_

#include "../../../../Common/MyString.h"

// Definitions of Opera flags that require special attention before they can be
// passed to embedded application.
namespace opera::flags {
const UString& ServerTrackingBlob();
const UString& CustomizationPackage();
}  // namespace flags

#endif // OPERA_FLAGS_H_

#endif // OPERA_CUSTOM_CODE
