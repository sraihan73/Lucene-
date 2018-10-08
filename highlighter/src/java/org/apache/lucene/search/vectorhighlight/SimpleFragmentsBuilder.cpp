using namespace std;

#include "SimpleFragmentsBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;

SimpleFragmentsBuilder::SimpleFragmentsBuilder() : BaseFragmentsBuilder() {}

SimpleFragmentsBuilder::SimpleFragmentsBuilder(std::deque<wstring> &preTags,
                                               std::deque<wstring> &postTags)
    : BaseFragmentsBuilder(preTags, postTags)
{
}

SimpleFragmentsBuilder::SimpleFragmentsBuilder(shared_ptr<BoundaryScanner> bs)
    : BaseFragmentsBuilder(bs)
{
}

SimpleFragmentsBuilder::SimpleFragmentsBuilder(std::deque<wstring> &preTags,
                                               std::deque<wstring> &postTags,
                                               shared_ptr<BoundaryScanner> bs)
    : BaseFragmentsBuilder(preTags, postTags, bs)
{
}

deque<std::shared_ptr<WeightedFragInfo>>
SimpleFragmentsBuilder::getWeightedFragInfoList(
    deque<std::shared_ptr<WeightedFragInfo>> &src)
{
  return src;
}
} // namespace org::apache::lucene::search::vectorhighlight