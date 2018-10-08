using namespace std;

#include "DocValuesUpdate.h"

namespace org::apache::lucene::index
{
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_ARRAY_HEADER; import
//    static org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_HEADER;
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

DocValuesUpdate::DocValuesUpdate(DocValuesType type, shared_ptr<Term> term,
                                 const wstring &field, int docIDUpto,
                                 bool hasValue)
    : type(type), term(term), field(field), docIDUpto(docIDUpto),
      hasValue(hasValue)
{
  assert((docIDUpto >= 0, to_wstring(docIDUpto) + L"must be >= 0"));
}

int DocValuesUpdate::sizeInBytes()
{
  int sizeInBytes = RAW_SIZE_IN_BYTES;
  sizeInBytes += term->field_->length() * Character::BYTES;
  sizeInBytes += term->bytes_.bytes->length;
  sizeInBytes += field.length() * Character::BYTES;
  sizeInBytes += valueSizeInBytes();
  sizeInBytes += 1; // hasValue
  return sizeInBytes;
}

bool DocValuesUpdate::hasValue() { return hasValue_; }

wstring DocValuesUpdate::toString()
{
  return L"term=" + term + L",field=" + field + L",value=" + valueToString() +
         L",docIDUpto=" + to_wstring(docIDUpto);
}

DocValuesUpdate::BinaryDocValuesUpdate::BinaryDocValuesUpdate(
    shared_ptr<Term> term, const wstring &field, shared_ptr<BytesRef> value)
    : BinaryDocValuesUpdate(term, field, value, BufferedUpdates::MAX_INT)
{
}

DocValuesUpdate::BinaryDocValuesUpdate::BinaryDocValuesUpdate(
    shared_ptr<Term> term, const wstring &field, shared_ptr<BytesRef> value,
    int docIDUpTo)
    : DocValuesUpdate(DocValuesType::BINARY, term, field, docIDUpTo,
                      value != nullptr),
      value(value)
{
}

shared_ptr<BinaryDocValuesUpdate>
DocValuesUpdate::BinaryDocValuesUpdate::prepareForApply(int docIDUpto)
{
  if (docIDUpto == this->docIDUpto) {
    return shared_from_this(); // it's a final value so we can safely reuse this
                               // instance
  }
  return make_shared<BinaryDocValuesUpdate>(term, field, value, docIDUpto);
}

int64_t DocValuesUpdate::BinaryDocValuesUpdate::valueSizeInBytes()
{
  return RAW_VALUE_SIZE_IN_BYTES + (value == nullptr ? 0 : value->bytes.size());
}

wstring DocValuesUpdate::BinaryDocValuesUpdate::valueToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return value->toString();
}

shared_ptr<BytesRef> DocValuesUpdate::BinaryDocValuesUpdate::getValue()
{
  assert((hasValue_,
          L"getValue should only be called if this update has a value"));
  return value;
}

void DocValuesUpdate::BinaryDocValuesUpdate::writeTo(
    shared_ptr<DataOutput> out) 
{
  assert(hasValue_);
  out->writeVInt(value->length);
  out->writeBytes(value->bytes, value->offset, value->length);
}

shared_ptr<BytesRef> DocValuesUpdate::BinaryDocValuesUpdate::readFrom(
    shared_ptr<DataInput> in_, shared_ptr<BytesRef> scratch) 
{
  scratch->length = in_->readVInt();
  if (scratch->bytes.size() < scratch->length) {
    scratch->bytes = ArrayUtil::grow(scratch->bytes, scratch->length);
  }
  in_->readBytes(scratch->bytes, 0, scratch->length);
  return scratch;
}

DocValuesUpdate::NumericDocValuesUpdate::NumericDocValuesUpdate(
    shared_ptr<Term> term, const wstring &field, int64_t value)
    : NumericDocValuesUpdate(term, field, value, BufferedUpdates::MAX_INT, true)
{
}

DocValuesUpdate::NumericDocValuesUpdate::NumericDocValuesUpdate(
    shared_ptr<Term> term, const wstring &field, optional<int64_t> &value)
    : NumericDocValuesUpdate(term, field,
                             value != nullptr ? value.longValue() : -1,
                             BufferedUpdates::MAX_INT, value != nullptr)
{
}

DocValuesUpdate::NumericDocValuesUpdate::NumericDocValuesUpdate(
    shared_ptr<Term> term, const wstring &field, int64_t value, int docIDUpTo,
    bool hasValue)
    : DocValuesUpdate(DocValuesType::NUMERIC, term, field, docIDUpTo, hasValue),
      value(value)
{
}

shared_ptr<NumericDocValuesUpdate>
DocValuesUpdate::NumericDocValuesUpdate::prepareForApply(int docIDUpto)
{
  if (docIDUpto == this->docIDUpto) {
    return shared_from_this();
  }
  return make_shared<NumericDocValuesUpdate>(term, field, value, docIDUpto,
                                             hasValue_);
}

int64_t DocValuesUpdate::NumericDocValuesUpdate::valueSizeInBytes()
{
  return Long::BYTES;
}

wstring DocValuesUpdate::NumericDocValuesUpdate::valueToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return hasValue_ ? Long::toString(value) : L"null";
}

void DocValuesUpdate::NumericDocValuesUpdate::writeTo(
    shared_ptr<DataOutput> out) 
{
  assert(hasValue_);
  out->writeZLong(value);
}

int64_t DocValuesUpdate::NumericDocValuesUpdate::readFrom(
    shared_ptr<DataInput> in_) 
{
  return in_->readZLong();
}

int64_t DocValuesUpdate::NumericDocValuesUpdate::getValue()
{
  assert((hasValue_,
          L"getValue should only be called if this update has a value"));
  return value;
}
} // namespace org::apache::lucene::index