using namespace std;

#include "KeywordRepeatFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

KeywordRepeatFilter::KeywordRepeatFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool KeywordRepeatFilter::incrementToken() 
{
  if (state != nullptr) {
    restoreState(state);
    posIncAttr->setPositionIncrement(0);
    keywordAttribute->setKeyword(false);
    state.reset();
    return true;
  }
  if (input->incrementToken()) {
    state = captureState();
    keywordAttribute->setKeyword(true);
    return true;
  }
  return false;
}

void KeywordRepeatFilter::reset() 
{
  TokenFilter::reset();
  state.reset();
}
} // namespace org::apache::lucene::analysis::miscellaneous