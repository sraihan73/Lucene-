using namespace std;

#include "FailOnNonBulkMergesInfoStream.h"

namespace org::apache::lucene::util
{

FailOnNonBulkMergesInfoStream::~FailOnNonBulkMergesInfoStream() {}

bool FailOnNonBulkMergesInfoStream::isEnabled(const wstring &component)
{
  return true;
}

void FailOnNonBulkMergesInfoStream::message(const wstring &component,
                                            const wstring &message)
{
  assert(!message.find(L"non-bulk merges") != wstring::npos);
}
} // namespace org::apache::lucene::util