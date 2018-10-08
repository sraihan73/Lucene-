using namespace std;

#include "DefaultBloomFilterFactory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "FuzzySet.h"

namespace org::apache::lucene::codecs::bloom
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

shared_ptr<FuzzySet>
DefaultBloomFilterFactory::getSetForField(shared_ptr<SegmentWriteState> state,
                                          shared_ptr<FieldInfo> info)
{
  // Assume all of the docs have a unique term (e.g. a primary key) and we hope
  // to maintain a set with 10% of bits set
  return FuzzySet::createSetBasedOnQuality(state->segmentInfo->maxDoc(), 0.10f);
}

bool DefaultBloomFilterFactory::isSaturated(shared_ptr<FuzzySet> bloomFilter,
                                            shared_ptr<FieldInfo> fieldInfo)
{
  // Don't bother saving bitsets if >90% of bits are set - we don't want to
  // throw any more memory at this problem.
  return bloomFilter->getSaturation() > 0.9f;
}
} // namespace org::apache::lucene::codecs::bloom