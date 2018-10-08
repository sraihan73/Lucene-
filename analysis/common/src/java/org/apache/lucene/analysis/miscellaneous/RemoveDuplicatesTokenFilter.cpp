using namespace std;

#include "RemoveDuplicatesTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

RemoveDuplicatesTokenFilter::RemoveDuplicatesTokenFilter(
    shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool RemoveDuplicatesTokenFilter::incrementToken() 
{
  while (input->incrementToken()) {
    const std::deque<wchar_t> term = termAttribute->buffer();
    constexpr int length = termAttribute->length();
    constexpr int posIncrement = posIncAttribute->getPositionIncrement();

    if (posIncrement > 0) {
      previous->clear();
    }

    bool duplicate = (posIncrement == 0 && previous->contains(term, 0, length));

    // clone the term, and add to the set of seen terms.
    std::deque<wchar_t> saved(length);
    System::arraycopy(term, 0, saved, 0, length);
    previous->add(saved);

    if (!duplicate) {
      return true;
    }
  }
  return false;
}

void RemoveDuplicatesTokenFilter::reset() 
{
  TokenFilter::reset();
  previous->clear();
}
} // namespace org::apache::lucene::analysis::miscellaneous