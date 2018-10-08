using namespace std;

#include "FloatEncoder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<BytesRef> FloatEncoder::encode(std::deque<wchar_t> &buffer,
                                          int offset, int length)
{
  float payload = stof(wstring(
      buffer, offset,
      length)); // TODO: improve this so that we don't have to new Strings
  std::deque<char> bytes = PayloadHelper::encodeFloat(payload);
  shared_ptr<BytesRef> result = make_shared<BytesRef>(bytes);
  return result;
}
} // namespace org::apache::lucene::analysis::payloads