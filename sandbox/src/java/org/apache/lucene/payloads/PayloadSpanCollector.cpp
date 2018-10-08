using namespace std;

#include "PayloadSpanCollector.h"

namespace org::apache::lucene::payloads
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using SpanCollector = org::apache::lucene::search::spans::SpanCollector;
using BytesRef = org::apache::lucene::util::BytesRef;

void PayloadSpanCollector::collectLeaf(shared_ptr<PostingsEnum> postings,
                                       int position,
                                       shared_ptr<Term> term) 
{
  shared_ptr<BytesRef> payload = postings->getPayload();
  if (payload == nullptr) {
    return;
  }
  const std::deque<char> bytes = std::deque<char>(payload->length);
  System::arraycopy(payload->bytes, payload->offset, bytes, 0, payload->length);
  payloads->add(bytes);
}

void PayloadSpanCollector::reset() { payloads->clear(); }

shared_ptr<deque<std::deque<char>>> PayloadSpanCollector::getPayloads()
{
  return payloads;
}
} // namespace org::apache::lucene::payloads