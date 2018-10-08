using namespace std;

#include "AbstractEncoder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::analysis::payloads
{
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<BytesRef> AbstractEncoder::encode(std::deque<wchar_t> &buffer)
{
  return encode(buffer, 0, buffer.size());
}
} // namespace org::apache::lucene::analysis::payloads