using namespace std;

#include "PayloadDecoder.h"

namespace org::apache::lucene::queries::payloads
{
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<PayloadDecoder> PayloadDecoder::FLOAT_DECODER =
    [&](any bytes) {
      return bytes == nullptr ? 1 : bytes::bytes[bytes::offset];
    };
} // namespace org::apache::lucene::queries::payloads