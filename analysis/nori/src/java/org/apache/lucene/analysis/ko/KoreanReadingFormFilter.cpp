using namespace std;

#include "KoreanReadingFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "tokenattributes/ReadingAttribute.h"

namespace org::apache::lucene::analysis::ko
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ReadingAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::ReadingAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

KoreanReadingFormFilter::KoreanReadingFormFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool KoreanReadingFormFilter::incrementToken() 
{
  if (input->incrementToken()) {
    wstring reading = readingAtt->getReading();
    if (reading != L"") {
      termAtt->setEmpty()->append(reading);
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::ko