using namespace std;

#include "TestDelimitedPayloadTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PayloadAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/payloads/PayloadHelper.h"

namespace org::apache::lucene::analysis::payloads
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using PayloadHelper = org::apache::lucene::analysis::payloads::PayloadHelper;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestDelimitedPayloadTokenFilterFactory::testEncoder() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"the|0.1 quick|0.1 red|0.1");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"DelimitedPayload", {L"encoder", L"float"})
               ->create(stream);

  stream->reset();
  while (stream->incrementToken()) {
    shared_ptr<PayloadAttribute> payAttr =
        stream->getAttribute(PayloadAttribute::typeid);
    assertNotNull(payAttr);
    std::deque<char> payData = payAttr->getPayload()->bytes;
    assertNotNull(payData);
    float payFloat = PayloadHelper::decodeFloat(payData);
    assertEquals(0.1f, payFloat, 0.0f);
  }
  stream->end();
  delete stream;
}

void TestDelimitedPayloadTokenFilterFactory::testDelim() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"the*0.1 quick*0.1 red*0.1");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"DelimitedPayload",
                              {L"encoder", L"float", L"delimiter", L"*"})
               ->create(stream);
  stream->reset();
  while (stream->incrementToken()) {
    shared_ptr<PayloadAttribute> payAttr =
        stream->getAttribute(PayloadAttribute::typeid);
    assertNotNull(payAttr);
    std::deque<char> payData = payAttr->getPayload()->bytes;
    assertNotNull(payData);
    float payFloat = PayloadHelper::decodeFloat(payData);
    assertEquals(0.1f, payFloat, 0.0f);
  }
  stream->end();
  delete stream;
}

void TestDelimitedPayloadTokenFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"DelimitedPayload",
                       {L"encoder", L"float", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::payloads