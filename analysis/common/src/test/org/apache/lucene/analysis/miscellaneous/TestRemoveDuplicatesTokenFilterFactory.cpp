using namespace std;

#include "TestRemoveDuplicatesTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

shared_ptr<Token> TestRemoveDuplicatesTokenFilterFactory::tok(int pos,
                                                              const wstring &t,
                                                              int start,
                                                              int end)
{
  shared_ptr<Token> tok = make_shared<Token>(t, start, end);
  tok->setPositionIncrement(pos);
  return tok;
}

void TestRemoveDuplicatesTokenFilterFactory::testDups(
    const wstring &expected, deque<Token> &tokens) 
{
  shared_ptr<TokenStream> stream = make_shared<CannedTokenStream>(tokens);
  stream = tokenFilterFactory(L"RemoveDuplicates").create(stream);
  assertTokenStreamContents(stream, expected.split(L"\\s"));
}

void TestRemoveDuplicatesTokenFilterFactory::testSimpleDups() throw(
    runtime_error)
{
  testDups(L"A B C D E",
           {tok(1, L"A", 0, 4), tok(1, L"B", 5, 10), tok(0, L"B", 11, 15),
            tok(1, L"C", 16, 20), tok(0, L"D", 16, 20), tok(1, L"E", 21, 25)});
}

void TestRemoveDuplicatesTokenFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"RemoveDuplicates", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous