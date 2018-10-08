using namespace std;

#include "StringAndPayloadField.h"

namespace org::apache::lucene::codecs::idversion
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<org::apache::lucene::document::FieldType>
    StringAndPayloadField::TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

StringAndPayloadField::StaticConstructor::StaticConstructor()
{
  TYPE->setOmitNorms(true);
  TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE->setTokenized(true);
  TYPE->freeze();
}

StringAndPayloadField::StaticConstructor
    StringAndPayloadField::staticConstructor;

StringAndPayloadField::StringAndPayloadField(const wstring &name,
                                             const wstring &value,
                                             shared_ptr<BytesRef> payload)
    : org::apache::lucene::document::Field(name, value, TYPE), payload(payload)
{
}

shared_ptr<TokenStream>
StringAndPayloadField::tokenStream(shared_ptr<Analyzer> analyzer,
                                   shared_ptr<TokenStream> reuse)
{
  shared_ptr<SingleTokenWithPayloadTokenStream> ts;
  if (std::dynamic_pointer_cast<SingleTokenWithPayloadTokenStream>(reuse) !=
      nullptr) {
    ts = std::static_pointer_cast<SingleTokenWithPayloadTokenStream>(reuse);
  } else {
    ts = make_shared<SingleTokenWithPayloadTokenStream>();
  }
  ts->setValue(any_cast<wstring>(fieldsData), payload);
  return ts;
}

void StringAndPayloadField::SingleTokenWithPayloadTokenStream::setValue(
    const wstring &value, shared_ptr<BytesRef> payload)
{
  this->value = value;
  this->payload = payload;
}

bool StringAndPayloadField::SingleTokenWithPayloadTokenStream::incrementToken()
{
  if (used) {
    return false;
  }
  clearAttributes();
  termAttribute->append(value);
  payloadAttribute->setPayload(payload);
  used = true;
  return true;
}

void StringAndPayloadField::SingleTokenWithPayloadTokenStream::reset()
{
  used = false;
}

StringAndPayloadField::SingleTokenWithPayloadTokenStream::
    ~SingleTokenWithPayloadTokenStream()
{
  value = L"";
  payload.reset();
}
} // namespace org::apache::lucene::codecs::idversion