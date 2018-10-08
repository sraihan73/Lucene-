using namespace std;

#include "SingleFragListBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;
using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

shared_ptr<FieldFragList> SingleFragListBuilder::createFieldFragList(
    shared_ptr<FieldPhraseList> fieldPhraseList, int fragCharSize)
{

  shared_ptr<FieldFragList> ffl =
      make_shared<SimpleFieldFragList>(fragCharSize);

  deque<std::shared_ptr<WeightedPhraseInfo>> wpil =
      deque<std::shared_ptr<WeightedPhraseInfo>>();
  deque<std::shared_ptr<WeightedPhraseInfo>>::const_iterator ite =
      fieldPhraseList->phraseList.begin();
  shared_ptr<WeightedPhraseInfo> phraseInfo = nullptr;
  while (true) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    if (!ite.hasNext()) {
      break;
    }
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    phraseInfo = ite.next();
    if (phraseInfo == nullptr) {
      break;
    }

    wpil.push_back(phraseInfo);
  }
  if (wpil.size() > 0) {
    ffl->add(0, numeric_limits<int>::max(), wpil);
  }
  return ffl;
}
} // namespace org::apache::lucene::search::vectorhighlight