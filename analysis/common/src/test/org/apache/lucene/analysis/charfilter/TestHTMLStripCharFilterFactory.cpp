using namespace std;

#include "TestHTMLStripCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::charfilter
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestHTMLStripCharFilterFactory::testNothingChanged() 
{
  //                             11111111112
  //                   012345678901234567890
  const wstring text = L"this is only a test.";
  shared_ptr<Reader> cs =
      charFilterFactory(L"HTMLStrip", {L"escapedTags", L"a, Title"})
          ->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"this", L"is", L"only", L"a", L"test."},
      std::deque<int>{0, 5, 8, 13, 15}, std::deque<int>{4, 7, 12, 14, 20});
}

void TestHTMLStripCharFilterFactory::testNoEscapedTags() 
{
  //                             11111111112222222222333333333344
  //                   012345678901234567890123456789012345678901
  const wstring text = L"<u>this</u> is <b>only</b> a <I>test</I>.";
  shared_ptr<Reader> cs =
      charFilterFactory(L"HTMLStrip").create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"this", L"is", L"only", L"a", L"test."},
      std::deque<int>{3, 12, 18, 27, 32},
      std::deque<int>{11, 14, 26, 28, 41});
}

void TestHTMLStripCharFilterFactory::testEscapedTags() 
{
  //                             11111111112222222222333333333344
  //                   012345678901234567890123456789012345678901
  const wstring text = L"<u>this</u> is <b>only</b> a <I>test</I>.";
  shared_ptr<Reader> cs =
      charFilterFactory(L"HTMLStrip", {L"escapedTags", L"U i"})
          ->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"<u>this</u>", L"is", L"only",
                                                 L"a", L"<I>test</I>."},
                            std::deque<int>{0, 12, 18, 27, 29},
                            std::deque<int>{11, 14, 26, 28, 41});
}

void TestHTMLStripCharFilterFactory::testSeparatorOnlyEscapedTags() throw(
    runtime_error)
{
  //                             11111111112222222222333333333344
  //                   012345678901234567890123456789012345678901
  const wstring text = L"<u>this</u> is <b>only</b> a <I>test</I>.";
  shared_ptr<Reader> cs =
      charFilterFactory(L"HTMLStrip", {L"escapedTags", L",, , "})
          ->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"this", L"is", L"only", L"a", L"test."},
      std::deque<int>{3, 12, 18, 27, 32},
      std::deque<int>{11, 14, 26, 28, 41});
}

void TestHTMLStripCharFilterFactory::testEmptyEscapedTags() 
{
  //                             11111111112222222222333333333344
  //                   012345678901234567890123456789012345678901
  const wstring text = L"<u>this</u> is <b>only</b> a <I>test</I>.";
  shared_ptr<Reader> cs = charFilterFactory(L"HTMLStrip", {L"escapedTags", L""})
                              ->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"this", L"is", L"only", L"a", L"test."},
      std::deque<int>{3, 12, 18, 27, 32},
      std::deque<int>{11, 14, 26, 28, 41});
}

void TestHTMLStripCharFilterFactory::testSingleEscapedTag() 
{
  //                             11111111112222222222333333333344
  //                   012345678901234567890123456789012345678901
  const wstring text = L"<u>this</u> is <b>only</b> a <I>test</I>.";
  shared_ptr<Reader> cs =
      charFilterFactory(L"HTMLStrip", {L"escapedTags", L", B\r\n\t"})
          ->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> ts = whitespaceMockTokenizer(cs);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"this", L"is", L"<b>only</b>", L"a", L"test."},
      std::deque<int>{3, 12, 15, 27, 32},
      std::deque<int>{11, 14, 26, 28, 41});
}

void TestHTMLStripCharFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    charFilterFactory(L"HTMLStrip", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::charfilter