using namespace std;

#include "TestNGramFilters.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/PayloadHelper.h"

namespace org::apache::lucene::analysis::ngram
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using PayloadHelper = org::apache::lucene::analysis::payloads::PayloadHelper;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestNGramFilters::testNGramTokenizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = tokenizerFactory(L"NGram").create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"t", L"te", L"e", L"es", L"s", L"st", L"t"});
}

void TestNGramFilters::testNGramTokenizer2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream =
      tokenizerFactory(L"NGram", {L"minGramSize", L"2", L"maxGramSize", L"3"})
          ->create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"te", L"tes", L"es", L"est", L"st"});
}

void TestNGramFilters::testNGramFilter() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"NGram", {L"minGramSize", L"1", L"maxGramSize", L"2"})
          ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"t", L"te", L"e", L"es", L"s", L"st", L"t"});
}

void TestNGramFilters::testNGramFilter2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream =
      tokenFilterFactory(L"NGram", {L"minGramSize", L"2", L"maxGramSize", L"3"})
          ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"te", L"tes", L"es", L"est", L"st"});
}

void TestNGramFilters::testNGramFilterPayload() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test|0.1");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"DelimitedPayload", {L"encoder", L"float"})
               ->create(stream);
  stream =
      tokenFilterFactory(L"NGram", {L"minGramSize", L"1", L"maxGramSize", L"2"})
          ->create(stream);

  stream->reset();
  while (stream->incrementToken()) {
    shared_ptr<PayloadAttribute> payAttr =
        stream->getAttribute(PayloadAttribute::typeid);
    assertNotNull(payAttr);
    shared_ptr<BytesRef> payData = payAttr->getPayload();
    assertNotNull(payData);
    float payFloat = PayloadHelper::decodeFloat(payData->bytes);
    assertEquals(0.1f, payFloat, 0.0f);
  }
  stream->end();
  delete stream;
}

void TestNGramFilters::testEdgeNGramTokenizer() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = tokenizerFactory(L"EdgeNGram").create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  assertTokenStreamContents(stream, std::deque<wstring>{L"t"});
}

void TestNGramFilters::testEdgeNGramTokenizer2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream =
      tokenizerFactory(L"EdgeNGram",
                       {L"minGramSize", L"1", L"maxGramSize", L"2"})
          ->create();
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  assertTokenStreamContents(stream, std::deque<wstring>{L"t", L"te"});
}

void TestNGramFilters::testEdgeNGramFilter() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"EdgeNGram",
                              {L"minGramSize", L"1", L"maxGramSize", L"1"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"t"});
}

void TestNGramFilters::testEdgeNGramFilter2() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"EdgeNGram",
                              {L"minGramSize", L"1", L"maxGramSize", L"2"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"t", L"te"});
}

void TestNGramFilters::testEdgeNGramFilterPayload() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"test|0.1");
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(reader);
  stream = tokenFilterFactory(L"DelimitedPayload", {L"encoder", L"float"})
               ->create(stream);
  stream = tokenFilterFactory(L"EdgeNGram",
                              {L"minGramSize", L"1", L"maxGramSize", L"2"})
               ->create(stream);

  stream->reset();
  while (stream->incrementToken()) {
    shared_ptr<PayloadAttribute> payAttr =
        stream->getAttribute(PayloadAttribute::typeid);
    assertNotNull(payAttr);
    shared_ptr<BytesRef> payData = payAttr->getPayload();
    assertNotNull(payData);
    float payFloat = PayloadHelper::decodeFloat(payData->bytes);
    assertEquals(0.1f, payFloat, 0.0f);
  }
  stream->end();
  delete stream;
}

void TestNGramFilters::testBogusArguments() 
{
  invalid_argument expected = nullptr;
  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"NGram", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"EdgeNGram", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"NGram", {L"minGramSize", L"2", L"maxGramSize", L"5",
                                  L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));

  expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"EdgeNGram", {L"minGramSize", L"2", L"maxGramSize",
                                      L"5", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::ngram