using namespace std;

#include "DateRecognizerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
const wstring DateRecognizerFilter::DATE_TYPE = L"date";

DateRecognizerFilter::DateRecognizerFilter(shared_ptr<TokenStream> input)
    : DateRecognizerFilter(input, nullptr)
{
}

DateRecognizerFilter::DateRecognizerFilter(shared_ptr<TokenStream> input,
                                           shared_ptr<DateFormat> dateFormat)
    : org::apache::lucene::analysis::FilteringTokenFilter(input),
      dateFormat(dateFormat != nullptr
                     ? dateFormat
                     : DateFormat::getDateInstance(DateFormat::DEFAULT,
                                                   Locale::ENGLISH))
{
}

bool DateRecognizerFilter::accept()
{
  try {
    // We don't care about the date, just that the term can be parsed to one.
    // C++ TODO: There is no native C++ equivalent to 'toString':
    dateFormat->parse(termAtt->toString());
    return true;
  } catch (const ParseException &e) {
    // This term is not a date.
  }

  return false;
}
} // namespace org::apache::lucene::analysis::miscellaneous