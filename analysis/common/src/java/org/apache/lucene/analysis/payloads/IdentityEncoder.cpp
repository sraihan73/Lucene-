using namespace std;

#include "IdentityEncoder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::analysis::payloads
{
using BytesRef = org::apache::lucene::util::BytesRef;

IdentityEncoder::IdentityEncoder() {}

IdentityEncoder::IdentityEncoder(shared_ptr<Charset> charset)
{
  this->charset = charset;
}

shared_ptr<BytesRef> IdentityEncoder::encode(std::deque<wchar_t> &buffer,
                                             int offset, int length)
{
  shared_ptr<ByteBuffer> *const bb =
      charset->encode(CharBuffer::wrap(buffer, offset, length));
  if (bb->hasArray()) {
    return make_shared<BytesRef>(
        bb->array_(), bb->arrayOffset() + bb->position(), bb->remaining());
  } else {
    // normally it should always have an array, but who knows?
    const std::deque<char> b = std::deque<char>(bb->remaining());
    bb->get(b);
    return make_shared<BytesRef>(b);
  }
}
} // namespace org::apache::lucene::analysis::payloads