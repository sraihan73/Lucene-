using namespace std;

#include "TestApostropheFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/tr/ApostropheFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/tr/TurkishLowerCaseFilter.h"

namespace org::apache::lucene::analysis::tr
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void TestApostropheFilter::testApostropheFilter() 
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"Türkiye'de 2003'te Van Gölü'nü gördüm");
  stream = make_shared<TurkishLowerCaseFilter>(stream);
  stream = make_shared<ApostropheFilter>(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"türkiye", L"2003", L"van", L"gölü", L"gördüm"});
}
} // namespace org::apache::lucene::analysis::tr