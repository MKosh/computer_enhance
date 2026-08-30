#include "types.h"
#include "string8.h"
#include "compare.h"

////////////////////////////////////////////////////////////////////////////////
///
i32 compareStringView(const void* x, const void* y)
{
  StringView* a = (StringView*)x;
  StringView* b = (StringView*)y;
  return sv_compare(*a, *b);
}

////////////////////////////////////////////////////////////////////////////////
///
i32 compareCString(const void* x, const void* y)
{
  return strcmp((const char*)x, (const char*)y);
}
