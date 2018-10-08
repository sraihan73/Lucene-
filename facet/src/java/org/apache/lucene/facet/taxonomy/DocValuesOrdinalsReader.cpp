using namespace std;

#include "DocValuesOrdinalsReader.h"

namespace org::apache::lucene::facet::taxonomy
{
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

DocValuesOrdinalsReader::DocValuesOrdinalsReader()
    : DocValuesOrdinalsReader(FacetsConfig::DEFAULT_INDEX_FIELD_NAME)
{
}

DocValuesOrdinalsReader::DocValuesOrdinalsReader(const wstring &field)
    : field(field)
{
}

shared_ptr<OrdinalsSegmentReader> DocValuesOrdinalsReader::getReader(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<BinaryDocValues> values0 =
      context->reader()->getBinaryDocValues(field);
  if (values0 == nullptr) {
    values0 = DocValues::emptyBinary();
  }

  shared_ptr<BinaryDocValues> *const values = values0;

  return make_shared<OrdinalsSegmentReaderAnonymousInnerClass>(
      shared_from_this(), values);
}

DocValuesOrdinalsReader::OrdinalsSegmentReaderAnonymousInnerClass::
    OrdinalsSegmentReaderAnonymousInnerClass(
        shared_ptr<DocValuesOrdinalsReader> outerInstance,
        shared_ptr<BinaryDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

void DocValuesOrdinalsReader::OrdinalsSegmentReaderAnonymousInnerClass::get(
    int docID, shared_ptr<IntsRef> ordinals) 
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(L"docs out of order: lastDocID=" +
                                      lastDocID + L" vs docID=" +
                                      to_wstring(docID));
  }
  lastDocID = docID;
  if (docID > values->docID()) {
    values->advance(docID);
  }
  shared_ptr<BytesRef> *const bytes;
  if (values->docID() == docID) {
    bytes = values->binaryValue();
  } else {
    bytes = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
  }
  outerInstance->decode(bytes, ordinals);
}

wstring DocValuesOrdinalsReader::getIndexFieldName() { return field; }

void DocValuesOrdinalsReader::decode(shared_ptr<BytesRef> buf,
                                     shared_ptr<IntsRef> ordinals)
{

  // grow the buffer up front, even if by a large number of values (buf.length)
  // that saves the need to check inside the loop for every decoded value if
  // the buffer needs to grow.
  if (ordinals->ints.size() < buf->length) {
    ordinals->ints = ArrayUtil::grow(ordinals->ints, buf->length);
  }

  ordinals->offset = 0;
  ordinals->length = 0;

  // it is better if the decoding is inlined like so, and not e.g.
  // in a utility method
  int upto = buf->offset + buf->length;
  int value = 0;
  int offset = buf->offset;
  int prev = 0;
  while (offset < upto) {
    char b = buf->bytes[offset++];
    if (b >= 0) {
      ordinals->ints[ordinals->length] = ((value << 7) | b) + prev;
      value = 0;
      prev = ordinals->ints[ordinals->length];
      ordinals->length++;
    } else {
      value = (value << 7) | (b & 0x7F);
    }
  }
}
} // namespace org::apache::lucene::facet::taxonomy