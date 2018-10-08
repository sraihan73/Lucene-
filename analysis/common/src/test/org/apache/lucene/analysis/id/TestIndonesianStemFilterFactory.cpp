using namespace std;

#include "TestIndonesianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::id
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestIndonesianStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dibukukannya");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"IndonesianStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"buku"});
}

void TestIndonesianStemFilterFactory::testStemmingInflectional() throw(
    runtime_error)
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"dibukukannya");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream =
      tokenFilterFactory(L"IndonesianStem", {L"stemDerivational", L"false"})
          ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"dibukukan"});
}

void TestIndonesianStemFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"IndonesianStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::id