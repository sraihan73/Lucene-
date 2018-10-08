using namespace std;

#include "WeightedFragListBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{

WeightedFragListBuilder::WeightedFragListBuilder() : BaseFragListBuilder() {}

WeightedFragListBuilder::WeightedFragListBuilder(int margin)
    : BaseFragListBuilder(margin)
{
}

shared_ptr<FieldFragList> WeightedFragListBuilder::createFieldFragList(
    shared_ptr<FieldPhraseList> fieldPhraseList, int fragCharSize)
{
  return createFieldFragList(fieldPhraseList,
                             make_shared<WeightedFieldFragList>(fragCharSize),
                             fragCharSize);
}
} // namespace org::apache::lucene::search::vectorhighlight