using namespace std;

#include "MockPayloadAnalyzer.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<TokenStreamComponents>
MockPayloadAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> result =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  return make_shared<TokenStreamComponents>(
      result, make_shared<MockPayloadFilter>(result, fieldName));
}

MockPayloadFilter::MockPayloadFilter(shared_ptr<TokenStream> input,
                                     const wstring &fieldName)
    : TokenFilter(input),
      posIncrAttr(input->addAttribute(PositionIncrementAttribute::typeid)),
      payloadAttr(input->addAttribute(PayloadAttribute::typeid)),
      termAttr(input->addAttribute(CharTermAttribute::typeid))
{
  this->fieldName = fieldName;
  pos = 0;
  i = 0;
}

bool MockPayloadFilter::incrementToken() 
{
  if (input->incrementToken()) {
    payloadAttr->setPayload(make_shared<BytesRef>(
        (L"pos: " + to_wstring(pos))->getBytes(StandardCharsets::UTF_8)));
    int posIncr;
    if (pos == 0 || i % 2 == 1) {
      posIncr = 1;
    } else {
      posIncr = 0;
    }
    posIncrAttr->setPositionIncrement(posIncr);
    pos += posIncr;
    i++;
    return true;
  } else {
    return false;
  }
}

void MockPayloadFilter::reset() 
{
  TokenFilter::reset();
  i = 0;
  pos = 0;
}
} // namespace org::apache::lucene::analysis