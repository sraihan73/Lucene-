using namespace std;

#include "TestPatternReplaceCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestPatternReplaceCharFilterFactory::testNothingChange() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is test.");
  reader =
      charFilterFactory(L"PatternReplace", {L"pattern", L"(aa)\\s+(bb)\\s+(cc)",
                                            L"replacement", L"$1$2$3"})
          ->create(reader);
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(reader);
  assertTokenStreamContents(ts, std::deque<wstring>{L"this", L"is", L"test."},
                            std::deque<int>{0, 5, 8},
                            std::deque<int>{4, 7, 13});
}

void TestPatternReplaceCharFilterFactory::testReplaceByEmpty() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"aa bb cc");
  reader = charFilterFactory(L"PatternReplace",
                             {L"pattern", L"(aa)\\s+(bb)\\s+(cc)"})
               ->create(reader);
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(reader);
  assertTokenStreamContents(ts, std::deque<wstring>());
}

void TestPatternReplaceCharFilterFactory::test1block1matchSameLength() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"aa bb cc");
  reader =
      charFilterFactory(L"PatternReplace", {L"pattern", L"(aa)\\s+(bb)\\s+(cc)",
                                            L"replacement", L"$1#$2#$3"})
          ->create(reader);
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(reader);
  assertTokenStreamContents(ts, std::deque<wstring>{L"aa#bb#cc"},
                            std::deque<int>{0}, std::deque<int>{8});
}

void TestPatternReplaceCharFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    charFilterFactory(L"PatternReplace",
                      {L"pattern", L"something", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::pattern