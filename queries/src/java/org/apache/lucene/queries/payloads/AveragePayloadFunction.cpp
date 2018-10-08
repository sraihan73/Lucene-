using namespace std;

#include "AveragePayloadFunction.h"

namespace org::apache::lucene::queries::payloads
{

float AveragePayloadFunction::currentScore(int docId, const wstring &field,
                                           int start, int end,
                                           int numPayloadsSeen,
                                           float currentScore,
                                           float currentPayloadScore)
{
  return currentPayloadScore + currentScore;
}

float AveragePayloadFunction::docScore(int docId, const wstring &field,
                                       int numPayloadsSeen, float payloadScore)
{
  return numPayloadsSeen > 0 ? (payloadScore / numPayloadsSeen) : 1;
}

int AveragePayloadFunction::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + this->getClass().hashCode();
  return result;
}

bool AveragePayloadFunction::equals(any obj)
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