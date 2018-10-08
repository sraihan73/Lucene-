using namespace std;

#include "LegacyStringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::codecs::lucene54
{
using BytesRef = org::apache::lucene::util::BytesRef;

int LegacyStringHelper::bytesDifference(shared_ptr<BytesRef> left,
                                        shared_ptr<BytesRef> right)
{
  int len = left->length < right->length ? left->length : right->length;
  const std::deque<char> bytesLeft = left->bytes;
  constexpr int offLeft = left->offset;
  std::deque<char> bytesRight = right->bytes;
  constexpr int offRight = right->offset;
  for (int i = 0; i < len; i++) {
    if (bytesLeft[i + offLeft] != bytesRight[i + offRight]) {
      return i;
    }
  }
  return len;
}

int LegacyStringHelper::sortKeyLength(shared_ptr<BytesRef> priorTerm,
                                      shared_ptr<BytesRef> currentTerm)
{
  constexpr int currentTermOffset = currentTerm->offset;
  constexpr int priorTermOffset = priorTerm->offset;
  constexpr int limit = min(priorTerm->length, currentTerm->length);
  for (int i = 0; i < limit; i++) {
    if (priorTerm->bytes[priorTermOffset + i] !=
        currentTerm->bytes[currentTermOffset + i]) {
      return i + 1;
    }
  }
  return min(1 + priorTerm->length, currentTerm->length);
}

LegacyStringHelper::LegacyStringHelper() {}
} // namespace org::apache::lucene::codecs::lucene54