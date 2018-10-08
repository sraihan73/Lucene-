using namespace std;

#include "SimpleFragListBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{

SimpleFragListBuilder::SimpleFragListBuilder() : BaseFragListBuilder() {}

SimpleFragListBuilder::SimpleFragListBuilder(int margin)
    : BaseFragListBuilder(margin)
{
}

shared_ptr<FieldFragList> SimpleFragListBuilder::createFieldFragList(
    shared_ptr<FieldPhraseList> fieldPhraseList, int fragCharSize)
{
  return createFieldFragList(fieldPhraseList,
                             make_shared<SimpleFieldFragList>(fragCharSize),
                             fragCharSize);
}
} // namespace org::apache::lucene::search::vectorhighlight