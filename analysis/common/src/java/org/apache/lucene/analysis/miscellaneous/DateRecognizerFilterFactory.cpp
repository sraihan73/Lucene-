using namespace std;

#include "DateRecognizerFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "DateRecognizerFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring DateRecognizerFilterFactory::DATE_PATTERN = L"datePattern";
const wstring DateRecognizerFilterFactory::LOCALE = L"locale";

DateRecognizerFilterFactory::DateRecognizerFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dateFormat(getDataFormat(get(args, DATE_PATTERN))),
      locale(getLocale(get(args, LOCALE)))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
DateRecognizerFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<DateRecognizerFilter>(input, dateFormat);
}

shared_ptr<Locale>
DateRecognizerFilterFactory::getLocale(const wstring &localeStr)
{
  if (localeStr == L"") {
    return Locale::ENGLISH;
  } else {
    return (make_shared<Locale::Builder>())->setLanguageTag(localeStr).build();
  }
}

shared_ptr<DateFormat>
DateRecognizerFilterFactory::getDataFormat(const wstring &datePattern)
{
  if (datePattern != L"") {
    return make_shared<SimpleDateFormat>(datePattern, locale);
  } else {
    return DateFormat::getDateInstance(DateFormat::DEFAULT, locale);
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous