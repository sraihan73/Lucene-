using namespace std;

#include "MaxPayloadFunction.h"

namespace org::apache::lucene::queries::payloads
{

float MaxPayloadFunction::currentScore(int docId, const wstring &field,
                                       int start, int end, int numPayloadsSeen,
                                       float currentScore,
                                       float currentPayloadScore)
{
  if (numPayloadsSeen == 0) {
    return currentPayloadScore;
  } else {
    return max(currentPayloadScore, currentScore);
  }
}

float MaxPayloadFunction::docScore(int docId, const wstring &field,
                                   int numPayloadsSeen, float payloadScore)
{
  return numPayloadsSeen > 0 ? payloadScore : 1;
}

int MaxPayloadFunction::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + this->getClass().hashCode();
  return result;
}

bool MaxPayloadFunction::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::queries::payloads