using namespace std;

#include "NullInfoStream.h"

namespace org::apache::lucene::util
{

void NullInfoStream::message(const wstring &component, const wstring &message)
{
  assert(component != L"");
  assert(message != L"");
}

bool NullInfoStream::isEnabled(const wstring &component)
{
  assert(component != L"");
  return true; // to actually enable logging, we just ignore on message()
}

NullInfoStream::~NullInfoStream() {}
} // namespace org::apache::lucene::util