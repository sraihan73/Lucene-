using namespace std;

#include "ScoreOrderFragmentsBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;

ScoreOrderFragmentsBuilder::ScoreOrderFragmentsBuilder()
    : BaseFragmentsBuilder()
{
}

ScoreOrderFragmentsBuilder::ScoreOrderFragmentsBuilder(
    std::deque<wstring> &preTags, std::deque<wstring> &postTags)
    : BaseFragmentsBuilder(preTags, postTags)
{
}

ScoreOrderFragmentsBuilder::ScoreOrderFragmentsBuilder(
    shared_ptr<BoundaryScanner> bs)
    : BaseFragmentsBuilder(bs)
{
}

ScoreOrderFragmentsBuilder::ScoreOrderFragmentsBuilder(
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<BoundaryScanner> bs)
    : BaseFragmentsBuilder(preTags, postTags, bs)
{
}

deque<std::shared_ptr<WeightedFragInfo>>
ScoreOrderFragmentsBuilder::getWeightedFragInfoList(
    deque<std::shared_ptr<WeightedFragInfo>> &src)
{
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(src, new ScoreComparator());
  sort(src.begin(), src.end(), make_shared<ScoreComparator>());
  return src;
}

int ScoreOrderFragmentsBuilder::ScoreComparator::compare(
    shared_ptr<WeightedFragInfo> o1, shared_ptr<WeightedFragInfo> o2)
{
  if (o1->getTotalBoost() > o2->getTotalBoost()) {
    return -1;
  } else if (o1->getTotalBoost() < o2->getTotalBoost()) {
    return 1;
  }
  // if same score then check startOffset
  else {
    if (o1->getStartOffset() < o2->getStartOffset()) {
      return -1;
    } else if (o1->getStartOffset() > o2->getStartOffset()) {
      return 1;
    }
  }
  return 0;
}
} // namespace org::apache::lucene::search::vectorhighlight