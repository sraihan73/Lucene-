using namespace std;

#include "TestProtectedTermFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LowerCaseFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ProtectedTermFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using LowerCaseFilter = org::apache::lucene::analysis::core::LowerCaseFilter;

void TestProtectedTermFilter::testBasic() 
{

  shared_ptr<CannedTokenStream> cts =
      make_shared<CannedTokenStream>(make_shared<Token>(L"Alice", 1, 0, 5),
                                     make_shared<Token>(L"Bob", 1, 6, 9),
                                     make_shared<Token>(L"Clara", 1, 10, 15),
                                     make_shared<Token>(L"David", 1, 16, 21));

  shared_ptr<CharArraySet> protectedTerms = make_shared<CharArraySet>(5, true);
  protectedTerms->add(L"bob");

  shared_ptr<TokenStream> ts = make_shared<ProtectedTermFilter>(
      protectedTerms, cts, LowerCaseFilter::new);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"alice", L"Bob", L"clara", L"david"});
}
} // namespace org::apache::lucene::analysis::miscellaneous