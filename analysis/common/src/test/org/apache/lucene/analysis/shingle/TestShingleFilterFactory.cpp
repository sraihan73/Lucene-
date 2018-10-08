using namespace std;

#include "TestShingleFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::shingle
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestShingleFilterFactory::testDefaults() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"this", L"this is",
                                                         L"is", L"is a", L"a",
                                                         L"a test", L"test"});
}

void TestShingleFilterFactory::testNoUnigrams() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle", {L"outputUnigrams", L"false"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this is", L"is a", L"a test"});
}

void TestShingleFilterFactory::testMaxShingleSize() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"Shingle", {L"maxShingleSize", L"3"})->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"this", L"this is", L"this is a", L"is", L"is a",
                           L"is a test", L"a", L"a test", L"test"});
}

void TestShingleFilterFactory::testMinShingleSize() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(
               L"Shingle", {L"minShingleSize", L"3", L"maxShingleSize", L"4"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this", L"this is a", L"this is a test",
                                   L"is", L"is a test", L"a", L"test"});
}

void TestShingleFilterFactory::testMinShingleSizeNoUnigrams() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle",
                              {L"minShingleSize", L"3", L"maxShingleSize", L"4",
                               L"outputUnigrams", L"false"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"this is a", L"this is a test", L"is a test"});
}

void TestShingleFilterFactory::testEqualMinAndMaxShingleSize() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(
               L"Shingle", {L"minShingleSize", L"3", L"maxShingleSize", L"3"})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"this", L"this is a", L"is",
                                                 L"is a test", L"a", L"test"});
}

void TestShingleFilterFactory::testEqualMinAndMaxShingleSizeNoUnigrams() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle",
                              {L"minShingleSize", L"3", L"maxShingleSize", L"3",
                               L"outputUnigrams", L"false"})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"this is a", L"is a test"});
}

void TestShingleFilterFactory::testTokenSeparator() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle", {L"tokenSeparator", L"=BLAH="})
               ->create(stream);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"this", L"this=BLAH=is",
                                                 L"is", L"is=BLAH=a", L"a",
                                                 L"a=BLAH=test", L"test"});
}

void TestShingleFilterFactory::testTokenSeparatorNoUnigrams() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle", {L"tokenSeparator", L"=BLAH=",
                                           L"outputUnigrams", L"false"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"this=BLAH=is", L"is=BLAH=a", L"a=BLAH=test"});
}

void TestShingleFilterFactory::testEmptyTokenSeparator() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"Shingle", {L"tokenSeparator", L""})->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"this", L"thisis",
                                                         L"is", L"isa", L"a",
                                                         L"atest", L"test"});
}

void TestShingleFilterFactory::testMinShingleSizeAndTokenSeparator() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle",
                              {L"minShingleSize", L"3", L"maxShingleSize", L"4",
                               L"tokenSeparator", L"=BLAH="})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this", L"this=BLAH=is=BLAH=a",
                                   L"this=BLAH=is=BLAH=a=BLAH=test", L"is",
                                   L"is=BLAH=a=BLAH=test", L"a", L"test"});
}

void TestShingleFilterFactory::
    testMinShingleSizeAndTokenSeparatorNoUnigrams() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"this is a test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"Shingle",
                              {L"minShingleSize", L"3", L"maxShingleSize", L"4",
                               L"tokenSeparator", L"=BLAH=", L"outputUnigrams",
                               L"false"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"this=BLAH=is=BLAH=a",
                                   L"this=BLAH=is=BLAH=a=BLAH=test",
                                   L"is=BLAH=a=BLAH=test"});
}

void TestShingleFilterFactory::testOutputUnigramsIfNoShingles() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"Shingle", {L"outputUnigrams", L"false",
                                      L"outputUnigramsIfNoShingles", L"true"})
          ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"test"});
}

void TestShingleFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Shingle", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::shingle