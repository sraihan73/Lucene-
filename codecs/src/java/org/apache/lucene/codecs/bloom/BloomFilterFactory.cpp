using namespace std;

#include "BloomFilterFactory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "FuzzySet.h"

namespace org::apache::lucene::codecs::bloom
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

shared_ptr<FuzzySet>
BloomFilterFactory::downsize(shared_ptr<FieldInfo> fieldInfo,
                             shared_ptr<FuzzySet> initialSet)
{
  // Aim for a bitset size that would have 10% of bits set (so 90% of searches
  // would fail-fast)
  float targetMaxSaturation = 0.1f;
  return initialSet->downsize(targetMaxSaturation);
}
} // namespace org::apache::lucene::codecs::bloom