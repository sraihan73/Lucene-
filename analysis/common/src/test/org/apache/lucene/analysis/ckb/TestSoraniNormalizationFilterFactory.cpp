using namespace std;

#include "TestSoraniNormalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::ckb
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestSoraniNormalizationFilterFactory::testNormalization() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"پیــــاوەکان");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"SoraniNormalization").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"پیاوەکان"});
}

void TestSoraniNormalizationFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"SoraniNormalization", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::ckb