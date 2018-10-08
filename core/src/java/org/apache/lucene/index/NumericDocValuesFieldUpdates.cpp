using namespace std;

#include "NumericDocValuesFieldUpdates.h"

namespace org::apache::lucene::index
{
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PagedGrowableWriter =
    org::apache::lucene::util::packed::PagedGrowableWriter;
using PagedMutable = org::apache::lucene::util::packed::PagedMutable;

NumericDocValuesFieldUpdates::Iterator::Iterator(
    int size, shared_ptr<PagedGrowableWriter> values,
    shared_ptr<PagedMutable> docs, int64_t delGen)
    : DocValuesFieldUpdates::AbstractIterator(size, docs, delGen),
      values(values)
{
}

int64_t NumericDocValuesFieldUpdates::Iterator::longValue() { return value; }

shared_ptr<BytesRef> NumericDocValuesFieldUpdates::Iterator::binaryValue()
{
  throw make_shared<UnsupportedOperationException>();
}

void NumericDocValuesFieldUpdates::Iterator::set(int64_t idx)
{
  value = values->get(idx);
}

NumericDocValuesFieldUpdates::NumericDocValuesFieldUpdates(int64_t delGen,
                                                           const wstring &field,
                                                           int maxDoc)
    : DocValuesFieldUpdates(maxDoc, delGen, field, DocValuesType::NUMERIC)
{
  values = make_shared<PagedGrowableWriter>(1, PAGE_SIZE, 1, PackedInts::FAST);
}

void NumericDocValuesFieldUpdates::add(int doc, shared_ptr<BytesRef> value)
{
  throw make_shared<UnsupportedOperationException>();
}

void NumericDocValuesFieldUpdates::add(
    int docId, shared_ptr<DocValuesFieldUpdates::Iterator> iterator)
{
  add(docId, iterator->longValue());
}

// C++ WARNING: The following method was originally marked 'synchronized':
void NumericDocValuesFieldUpdates::add(int doc, int64_t value)
{
  int add = this->add(doc);
  values->set(add, value);
}

void NumericDocValuesFieldUpdates::swap(int i, int j)
{
  DocValuesFieldUpdates::swap(i, j);
  int64_t tmpVal = values->get(j);
  values->set(j, values->get(i));
  values->set(i, tmpVal);
}

void NumericDocValuesFieldUpdates::grow(int size)
{
  DocValuesFieldUpdates::grow(size);
  values = values->grow(size);
}

void NumericDocValuesFieldUpdates::resize(int size)
{
  DocValuesFieldUpdates::resize(size);
  values = values->resize(size);
}

shared_ptr<Iterator> NumericDocValuesFieldUpdates::iterator()
{
  ensureFinished();
  return make_shared<Iterator>(size_, values, docs, delGen);
}

int64_t NumericDocValuesFieldUpdates::ramBytesUsed()
{
  return values->ramBytesUsed() + DocValuesFieldUpdates::ramBytesUsed() +
         Long::BYTES + RamUsageEstimator::NUM_BYTES_OBJECT_REF;
}
} // namespace org::apache::lucene::index