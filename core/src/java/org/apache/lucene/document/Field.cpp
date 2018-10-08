using namespace std;

#include "Field.h"

namespace org::apache::lucene::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

Field::Field(const wstring &name, shared_ptr<IndexableFieldType> type)
    : type(type), name(name)
{
  if (name == L"") {
    throw invalid_argument(L"name must not be null");
  }
  if (type == nullptr) {
    throw invalid_argument(L"type must not be null");
  }
}

Field::Field(const wstring &name, shared_ptr<Reader> reader,
             shared_ptr<IndexableFieldType> type)
    : type(type), name(name)
{
  if (name == L"") {
    throw invalid_argument(L"name must not be null");
  }
  if (type == nullptr) {
    throw invalid_argument(L"type must not be null");
  }
  if (reader == nullptr) {
    throw make_shared<NullPointerException>(L"reader must not be null");
  }
  if (type->stored()) {
    throw invalid_argument(L"fields with a Reader value cannot be stored");
  }
  if (type->indexOptions() != IndexOptions::NONE && !type->tokenized()) {
    throw invalid_argument(L"non-tokenized fields must use std::wstring values");
  }

  this->fieldsData = reader;
}

Field::Field(const wstring &name, shared_ptr<TokenStream> tokenStream,
             shared_ptr<IndexableFieldType> type)
    : type(type), name(name)
{
  if (name == L"") {
    throw invalid_argument(L"name must not be null");
  }
  if (tokenStream == nullptr) {
    throw make_shared<NullPointerException>(L"tokenStream must not be null");
  }
  if (type->indexOptions() == IndexOptions::NONE || !type->tokenized()) {
    throw invalid_argument(L"TokenStream fields must be indexed and tokenized");
  }
  if (type->stored()) {
    throw invalid_argument(L"TokenStream fields cannot be stored");
  }

  this->fieldsData = nullptr;
  this->tokenStream_ = tokenStream;
}

Field::Field(const wstring &name, std::deque<char> &value,
             shared_ptr<IndexableFieldType> type)
    : Field(name, value, 0, value.length, type)
{
}

Field::Field(const wstring &name, std::deque<char> &value, int offset,
             int length, shared_ptr<IndexableFieldType> type)
    : Field(name, new BytesRef(value, offset, length), type)
{
}

Field::Field(const wstring &name, shared_ptr<BytesRef> bytes,
             shared_ptr<IndexableFieldType> type)
    : type(type), name(name)
{
  if (name == L"") {
    throw invalid_argument(L"name must not be null");
  }
  if (bytes == nullptr) {
    throw invalid_argument(L"bytes must not be null");
  }
  this->fieldsData = bytes;
}

Field::Field(const wstring &name, const wstring &value,
             shared_ptr<IndexableFieldType> type)
    : type(type), name(name)
{
  if (name == L"") {
    throw invalid_argument(L"name must not be null");
  }
  if (value == L"") {
    throw invalid_argument(L"value must not be null");
  }
  if (!type->stored() && type->indexOptions() == IndexOptions::NONE) {
    throw invalid_argument(
        wstring(L"it doesn't make sense to have a field that ") +
        L"is neither indexed nor stored");
  }
  this->fieldsData = value;
}

wstring Field::stringValue()
{
  if (dynamic_cast<wstring>(fieldsData) != nullptr ||
      std::dynamic_pointer_cast<Number>(fieldsData) != nullptr) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return fieldsData.toString();
  } else {
    return L"";
  }
}

shared_ptr<Reader> Field::readerValue()
{
  return std::dynamic_pointer_cast<Reader>(fieldsData) != nullptr
             ? any_cast<std::shared_ptr<Reader>>(fieldsData)
             : nullptr;
}

shared_ptr<TokenStream> Field::tokenStreamValue() { return tokenStream_; }

void Field::setStringValue(const wstring &value)
{
  if (!(dynamic_cast<wstring>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to std::wstring");
  }
  if (value == L"") {
    throw invalid_argument(L"value must not be null");
  }
  fieldsData = value;
}

void Field::setReaderValue(shared_ptr<Reader> value)
{
  if (!(std::dynamic_pointer_cast<Reader>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Reader");
  }
  fieldsData = value;
}

void Field::setBytesValue(std::deque<char> &value)
{
  setBytesValue(make_shared<BytesRef>(value));
}

void Field::setBytesValue(shared_ptr<BytesRef> value)
{
  if (!(std::dynamic_pointer_cast<BytesRef>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to BytesRef");
  }
  if (type->indexOptions() != IndexOptions::NONE) {
    throw invalid_argument(L"cannot set a BytesRef value on an indexed field");
  }
  if (value == nullptr) {
    throw invalid_argument(L"value must not be null");
  }
  fieldsData = value;
}

void Field::setByteValue(char value)
{
  if (!(dynamic_cast<optional<char>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Byte");
  }
  fieldsData = static_cast<char>(value);
}

void Field::setShortValue(short value)
{
  if (!(dynamic_cast<optional<short>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Short");
  }
  fieldsData = static_cast<Short>(value);
}

void Field::setIntValue(int value)
{
  if (!(dynamic_cast<optional<int>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Integer");
  }
  fieldsData = static_cast<Integer>(value);
}

void Field::setLongValue(int64_t value)
{
  if (!(dynamic_cast<optional<int64_t>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Long");
  }
  fieldsData = static_cast<int64_t>(value);
}

void Field::setFloatValue(float value)
{
  if (!(dynamic_cast<optional<float>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Float");
  }
  fieldsData = static_cast<Float>(value);
}

void Field::setDoubleValue(double value)
{
  if (!(dynamic_cast<optional<double>>(fieldsData) != nullptr)) {
    throw invalid_argument(L"cannot change value type from " +
                           fieldsData.type().getSimpleName() + L" to Double");
  }
  fieldsData = static_cast<Double>(value);
}

void Field::setTokenStream(shared_ptr<TokenStream> tokenStream)
{
  if (type->indexOptions() == IndexOptions::NONE || !type->tokenized()) {
    throw invalid_argument(L"TokenStream fields must be indexed and tokenized");
  }
  this->tokenStream_ = tokenStream;
}

wstring Field::name() { return name_; }

shared_ptr<Number> Field::numericValue()
{
  if (std::dynamic_pointer_cast<Number>(fieldsData) != nullptr) {
    return any_cast<std::shared_ptr<Number>>(fieldsData);
  } else {
    return nullptr;
  }
}

shared_ptr<BytesRef> Field::binaryValue()
{
  if (std::dynamic_pointer_cast<BytesRef>(fieldsData) != nullptr) {
    return any_cast<std::shared_ptr<BytesRef>>(fieldsData);
  } else {
    return nullptr;
  }
}

wstring Field::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  result->append(type->toString());
  result->append(L'<');
  result->append(name_);
  result->append(L':');

  if (fieldsData != nullptr) {
    result->append(fieldsData);
  }

  result->append(L'>');
  return result->toString();
}

shared_ptr<IndexableFieldType> Field::fieldType() { return type; }

shared_ptr<TokenStream> Field::tokenStream(shared_ptr<Analyzer> analyzer,
                                           shared_ptr<TokenStream> reuse)
{
  if (fieldType()->indexOptions() == IndexOptions::NONE) {
    // Not indexed
    return nullptr;
  }

  if (!fieldType()->tokenized()) {
    if (stringValue() != L"") {
      if (!(std::dynamic_pointer_cast<Analyzer::StringTokenStream>(reuse) !=
            nullptr)) {
        // lazy init the TokenStream as it is heavy to instantiate
        // (attributes,...) if not needed
        reuse = make_shared<Analyzer::StringTokenStream>();
      }
      (std::static_pointer_cast<Analyzer::StringTokenStream>(reuse))
          ->setValue(stringValue());
      return reuse;
    } else if (binaryValue() != nullptr) {
      if (!(std::dynamic_pointer_cast<BinaryTokenStream>(reuse) != nullptr)) {
        // lazy init the TokenStream as it is heavy to instantiate
        // (attributes,...) if not needed
        reuse = make_shared<BinaryTokenStream>();
      }
      (std::static_pointer_cast<BinaryTokenStream>(reuse))
          ->setValue(binaryValue());
      return reuse;
    } else {
      throw invalid_argument(L"Non-Tokenized Fields must have a std::wstring value");
    }
  }

  if (tokenStream_ != nullptr) {
    return tokenStream_;
  } else if (readerValue() != nullptr) {
    return analyzer->tokenStream(name(), readerValue());
  } else if (stringValue() != L"") {
    return analyzer->tokenStream(name(), stringValue());
  }

  throw invalid_argument(L"Field must have either TokenStream, std::wstring, Reader "
                         L"or Number value; got " +
                         shared_from_this());
}

Field::BinaryTokenStream::BinaryTokenStream() {}

void Field::BinaryTokenStream::setValue(shared_ptr<BytesRef> value)
{
  this->value = value;
}

bool Field::BinaryTokenStream::incrementToken()
{
  if (used) {
    return false;
  }
  clearAttributes();
  bytesAtt->setBytesRef(value);
  used = true;
  return true;
}

void Field::BinaryTokenStream::reset() { used = false; }

Field::BinaryTokenStream::~BinaryTokenStream() { value.reset(); }

Field::StringTokenStream::StringTokenStream() {}

void Field::StringTokenStream::setValue(const wstring &value)
{
  this->value = value;
}

bool Field::StringTokenStream::incrementToken()
{
  if (used) {
    return false;
  }
  clearAttributes();
  termAttribute->append(value);
  offsetAttribute->setOffset(0, value.length());
  used = true;
  return true;
}

void Field::StringTokenStream::end() 
{
  TokenStream::end();
  constexpr int finalOffset = value.length();
  offsetAttribute->setOffset(finalOffset, finalOffset);
}

void Field::StringTokenStream::reset() { used = false; }

Field::StringTokenStream::~StringTokenStream() { value = L""; }
} // namespace org::apache::lucene::document