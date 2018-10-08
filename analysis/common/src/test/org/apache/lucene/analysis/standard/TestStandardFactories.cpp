using namespace std;

#include "TestStandardFactories.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestStandardFactories::testStandardTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"Wha\u0301t's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Standard").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Wha\u0301t's", L"this", L"thing", L"do"});
}

void TestStandardFactories::testStandardTokenizerMaxTokenLength() throw(
    runtime_error)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int i = 0; i < 100; ++i) {
    builder->append(L"abcdefg"); // 7 * 100 = 700 char "word"
  }
  wstring longWord = builder->toString();
  wstring content = L"one two three " + longWord + L" four five six";
  shared_ptr<Reader> reader = make_shared<StringReader>(content);
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Standard", {L"maxTokenLength", L"1000"})
          ->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"one", L"two", L"three", longWord, L"four",
                                   L"five", L"six"});
}

void TestStandardFactories::testClassicTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Classic").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"What's", L"this", L"thing", L"do"});
}

void TestStandardFactories::testClassicTokenizerMaxTokenLength() throw(
    runtime_error)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  for (int i = 0; i < 100; ++i) {
    builder->append(L"abcdefg"); // 7 * 100 = 700 char "word"
  }
  wstring longWord = builder->toString();
  wstring content = L"one two three " + longWord + L" four five six";
  shared_ptr<Reader> reader = make_shared<StringReader>(content);
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Classic", {L"maxTokenLength", L"1000"})
          ->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"one", L"two", L"three", longWord, L"four",
                                   L"five", L"six"});
}

void TestStandardFactories::testStandardFilter() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> tokenizer =
      tokenizerFactory(L"Classic").create(newAttributeFactory());
  tokenizer->setReader(reader);
  shared_ptr<TokenStream> stream =
      tokenFilterFactory(L"Classic").create(tokenizer);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"What", L"this", L"thing", L"do"});
}

void TestStandardFactories::testKeywordTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> stream = tokenizerFactory(L"Keyword").create();
  stream->setReader(reader);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"What's this thing do?"});
}

void TestStandardFactories::testWhitespaceTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Whitespace").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"What's", L"this", L"thing", L"do?"});
}

void TestStandardFactories::testLetterTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Letter").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"What", L"s", L"this", L"thing", L"do"});
}

void TestStandardFactories::testLowerCaseTokenizer() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"What's this thing do?");
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"LowerCase").create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"what", L"s", L"this", L"thing", L"do"});
}

void TestStandardFactories::testASCIIFolding() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"Česká");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"ASCIIFolding").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"Ceska"});
}

void TestStandardFactories::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Standard", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Classic", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Whitespace", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Letter", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"LowerCase", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ASCIIFolding", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Standard", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Classic", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::standard