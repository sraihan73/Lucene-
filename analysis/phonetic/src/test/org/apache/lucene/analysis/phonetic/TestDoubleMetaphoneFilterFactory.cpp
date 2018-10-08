using namespace std;

#include "TestDoubleMetaphoneFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DoubleMetaphoneFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DoubleMetaphoneFilterFactory.h"

namespace org::apache::lucene::analysis::phonetic
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using DoubleMetaphoneFilter =
    org::apache::lucene::analysis::phonetic::DoubleMetaphoneFilter;

void TestDoubleMetaphoneFilterFactory::testDefaults() 
{
  shared_ptr<DoubleMetaphoneFilterFactory> factory =
      make_shared<DoubleMetaphoneFilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<TokenStream> inputStream =
      whitespaceMockTokenizer(L"international");

  shared_ptr<TokenStream> filteredStream = factory->create(inputStream);
  assertEquals(DoubleMetaphoneFilter::typeid, filteredStream->getClass());
  assertTokenStreamContents(filteredStream,
                            std::deque<wstring>{L"international", L"ANTR"});
}

void TestDoubleMetaphoneFilterFactory::testSettingSizeAndInject() throw(
    runtime_error)
{
  unordered_map<wstring, wstring> parameters =
      unordered_map<wstring, wstring>();
  parameters.emplace(L"inject", L"false");
  parameters.emplace(L"maxCodeLength", L"8");
  shared_ptr<DoubleMetaphoneFilterFactory> factory =
      make_shared<DoubleMetaphoneFilterFactory>(parameters);

  shared_ptr<TokenStream> inputStream =
      whitespaceMockTokenizer(L"international");

  shared_ptr<TokenStream> filteredStream = factory->create(inputStream);
  assertEquals(DoubleMetaphoneFilter::typeid, filteredStream->getClass());
  assertTokenStreamContents(filteredStream, std::deque<wstring>{L"ANTRNXNL"});
}

void TestDoubleMetaphoneFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<DoubleMetaphoneFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestDoubleMetaphoneFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestDoubleMetaphoneFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::phonetic