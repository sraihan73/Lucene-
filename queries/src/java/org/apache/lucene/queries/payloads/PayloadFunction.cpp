using namespace std;

#include "PayloadFunction.h"

namespace org::apache::lucene::queries::payloads
{
using Explanation = org::apache::lucene::search::Explanation;
using Spans = org::apache::lucene::search::spans::Spans;

shared_ptr<Explanation> PayloadFunction::explain(int docId,
                                                 const wstring &field,
                                                 int numPayloadsSeen,
                                                 float payloadScore)
{
  return Explanation::match(
      docScore(docId, field, numPayloadsSeen, payloadScore),
      getClass().getSimpleName() + L".docScore()");
};
} // namespace org::apache::lucene::queries::payloads