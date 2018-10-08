using namespace std;

#include "JapaneseReadingFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "tokenattributes/ReadingAttribute.h"
#include "util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using ReadingAttribute =
    org::apache::lucene::analysis::ja::tokenattributes::ReadingAttribute;
using ToStringUtil = org::apache::lucene::analysis::ja::util::ToStringUtil;

JapaneseReadingFormFilter::JapaneseReadingFormFilter(
    shared_ptr<TokenStream> input, bool useRomaji)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  this->useRomaji = useRomaji;
}

JapaneseReadingFormFilter::JapaneseReadingFormFilter(
    shared_ptr<TokenStream> input)
    : JapaneseReadingFormFilter(input, false)
{
}

bool JapaneseReadingFormFilter::incrementToken() 
{
  if (input->incrementToken()) {
    wstring reading = readingAttr->getReading();

    if (useRomaji) {
      if (reading == L"") {
        // if it's an OOV term, just try the term text
        buffer->setLength(0);
        ToStringUtil::getRomanization(buffer, termAttr);
        termAttr->setEmpty()->append(buffer);
      } else {
        ToStringUtil::getRomanization(termAttr->setEmpty(), reading);
      }
    } else {
      // just replace the term text with the reading, if it exists
      if (reading != L"") {
        termAttr->setEmpty()->append(reading);
      }
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::ja