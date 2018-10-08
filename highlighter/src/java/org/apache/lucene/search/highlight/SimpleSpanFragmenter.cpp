using namespace std;

#include "SimpleSpanFragmenter.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Spans = org::apache::lucene::search::spans::Spans;

SimpleSpanFragmenter::SimpleSpanFragmenter(shared_ptr<QueryScorer> queryScorer)
    : SimpleSpanFragmenter(queryScorer, DEFAULT_FRAGMENT_SIZE)
{
}

SimpleSpanFragmenter::SimpleSpanFragmenter(shared_ptr<QueryScorer> queryScorer,
                                           int fragmentSize)
{
  this->fragmentSize = fragmentSize;
  this->queryScorer = queryScorer;
}

bool SimpleSpanFragmenter::isNewFragment()
{
  position += posIncAtt->getPositionIncrement();

  if (waitForPos <= position) {
    waitForPos = -1;
  } else if (waitForPos != -1) {
    return false;
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<WeightedSpanTerm> wSpanTerm =
      queryScorer->getWeightedSpanTerm(termAtt->toString());

  if (wSpanTerm != nullptr) {
    deque<std::shared_ptr<PositionSpan>> positionSpans =
        wSpanTerm->getPositionSpans();

    for (auto positionSpan : positionSpans) {
      if (positionSpan->start == position) {
        waitForPos = positionSpan->end + 1;
        break;
      }
    }
  }

  bool isNewFrag =
      offsetAtt->endOffset() >= (fragmentSize * currentNumFrags) &&
      (textSize - offsetAtt->endOffset()) >=
          (static_cast<int>(static_cast<unsigned int>(fragmentSize) >> 1));

  if (isNewFrag) {
    currentNumFrags++;
  }

  return isNewFrag;
}

void SimpleSpanFragmenter::start(const wstring &originalText,
                                 shared_ptr<TokenStream> tokenStream)
{
  position = -1;
  currentNumFrags = 1;
  textSize = originalText.length();
  termAtt = tokenStream->addAttribute(CharTermAttribute::typeid);
  posIncAtt = tokenStream->addAttribute(PositionIncrementAttribute::typeid);
  offsetAtt = tokenStream->addAttribute(OffsetAttribute::typeid);
}
} // namespace org::apache::lucene::search::highlight