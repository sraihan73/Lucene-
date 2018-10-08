using namespace std;

#include "TestKeywordRepeatFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeywordRepeatFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/RemoveDuplicatesTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/snowball/SnowballFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;

void TestKeywordRepeatFilter::testBasic() 
{
  shared_ptr<TokenStream> ts =
      make_shared<RemoveDuplicatesTokenFilter>(make_shared<SnowballFilter>(
          make_shared<KeywordRepeatFilter>(
              whitespaceMockTokenizer(L"the birds are flying")),
          L"English"));
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"the", L"birds", L"bird",
                                                 L"are", L"flying", L"fli"},
                            std::deque<int>{1, 1, 0, 1, 1, 0});
}

void TestKeywordRepeatFilter::testComposition() 
{
  shared_ptr<TokenStream> ts =
      make_shared<RemoveDuplicatesTokenFilter>(make_shared<SnowballFilter>(
          make_shared<KeywordRepeatFilter>(make_shared<KeywordRepeatFilter>(
              whitespaceMockTokenizer(L"the birds are flying"))),
          L"English"));
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"the", L"birds", L"bird",
                                                 L"are", L"flying", L"fli"},
                            std::deque<int>{1, 1, 0, 1, 1, 0});
}
} // namespace org::apache::lucene::analysis::miscellaneous