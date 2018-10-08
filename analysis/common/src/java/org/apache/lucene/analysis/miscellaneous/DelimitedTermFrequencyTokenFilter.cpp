using namespace std;

#include "DelimitedTermFrequencyTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TermFrequencyAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

DelimitedTermFrequencyTokenFilter::DelimitedTermFrequencyTokenFilter(
    shared_ptr<TokenStream> input)
    : DelimitedTermFrequencyTokenFilter(input, DEFAULT_DELIMITER)
{
}

DelimitedTermFrequencyTokenFilter::DelimitedTermFrequencyTokenFilter(
    shared_ptr<TokenStream> input, wchar_t delimiter)
    : org::apache::lucene::analysis::TokenFilter(input), delimiter(delimiter)
{
}

bool DelimitedTermFrequencyTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    const std::deque<wchar_t> buffer = termAtt->buffer();
    constexpr int length = termAtt->length();
    for (int i = 0; i < length; i++) {
      if (buffer[i] == delimiter) {
        termAtt->setLength(i); // simply set a new length
        i++;
        tfAtt->setTermFrequency(ArrayUtil::parseInt(buffer, i, length - i));
        return true;
      }
    }
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::miscellaneous