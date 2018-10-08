using namespace std;

#include "DateRecognizerFilterTest.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void DateRecognizerFilterTest::test() 
{
  const wstring test = L"The red fox jumped over the lazy dogs on 7/11/2006 "
                       L"The dogs finally reacted on 7/12/2006";
  shared_ptr<DateFormat> *const dateFormat =
      make_shared<SimpleDateFormat>(L"MM/dd/yyyy", Locale::ENGLISH);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // new DateRecognizerFilter(whitespaceMockTokenizer(test), dateFormat);)
  {
    org::apache::lucene::analysis::TokenStream ts =
        DateRecognizerFilter(whitespaceMockTokenizer(test), dateFormat);
    assertStreamHasNumberOfTokens(ts, 2);
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous