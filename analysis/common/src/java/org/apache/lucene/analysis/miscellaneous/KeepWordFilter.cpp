using namespace std;

#include "KeepWordFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

KeepWordFilter::KeepWordFilter(shared_ptr<TokenStream> in_,
                               shared_ptr<CharArraySet> words)
    : org::apache::lucene::analysis::FilteringTokenFilter(in_), words(words)
{
}

bool KeepWordFilter::accept()
{
  return words->contains(termAtt->buffer(), 0, termAtt->length());
}
} // namespace org::apache::lucene::analysis::miscellaneous