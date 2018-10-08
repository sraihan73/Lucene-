using namespace std;

#include "TestElisionFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::util
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestElisionFilterFactory::testElision() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"l'avion");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"Elision", {L"articles", L"frenchArticles.txt"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"avion"});
}

void TestElisionFilterFactory::testDefaultArticles() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"l'avion");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"Elision").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"avion"});
}

void TestElisionFilterFactory::testCaseInsensitive() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"L'avion");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"Elision", {L"articles", L"frenchArticles.txt",
                                           L"ignoreCase", L"true"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"avion"});
}

void TestElisionFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"Elision", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::util