using namespace std;

#include "SegToken.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{
using WordType = org::apache::lucene::analysis::cn::smart::WordType;

SegToken::SegToken(std::deque<wchar_t> &idArray, int start, int end,
                   int wordType, int weight)
{
  this->charArray = idArray;
  this->startOffset = start;
  this->endOffset = end;
  this->wordType = wordType;
  this->weight = weight;
}

int SegToken::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  for (int i = 0; i < charArray.size(); i++) {
    result = prime * result + charArray[i];
  }
  result = prime * result + endOffset;
  result = prime * result + index;
  result = prime * result + startOffset;
  result = prime * result + weight;
  result = prime * result + wordType;
  return result;
}

bool SegToken::equals(any obj)
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
  shared_ptr<SegToken> other = any_cast<std::shared_ptr<SegToken>>(obj);
  if (!Arrays::equals(charArray, other->charArray)) {
    return false;
  }
  if (endOffset != other->endOffset) {
    return false;
  }
  if (index != other->index) {
    return false;
  }
  if (startOffset != other->startOffset) {
    return false;
  }
  if (weight != other->weight) {
    return false;
  }
  if (wordType != other->wordType) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm