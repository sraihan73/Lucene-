using namespace std;

#include "ElisionFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::util
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

ElisionFilter::ElisionFilter(shared_ptr<TokenStream> input,
                             shared_ptr<CharArraySet> articles)
    : org::apache::lucene::analysis::TokenFilter(input), articles(articles)
{
}

bool ElisionFilter::incrementToken() 
{
  if (input->incrementToken()) {
    std::deque<wchar_t> termBuffer = termAtt->buffer();
    int termLength = termAtt->length();

    int index = -1;
    for (int i = 0; i < termLength; i++) {
      wchar_t ch = termBuffer[i];
      if (ch == L'\'' || ch == L'\u2019') {
        index = i;
        break;
      }
    }

    // An apostrophe has been found. If the prefix is an article strip it off.
    if (index >= 0 && articles->contains(termBuffer, 0, index)) {
      termAtt->copyBuffer(termBuffer, index + 1, termLength - (index + 1));
    }

    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::util