using namespace std;

#include "IntegerEncoder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<BytesRef> IntegerEncoder::encode(std::deque<wchar_t> &buffer,
                                            int offset, int length)
{
  int payload = ArrayUtil::parseInt(
      buffer, offset,
      length); // TODO: improve this so that we don't have to new Strings
  std::deque<char> bytes = PayloadHelper::encodeInt(payload);
  shared_ptr<BytesRef> result = make_shared<BytesRef>(bytes);
  return result;
}
} // namespace org::apache::lucene::analysis::payloads