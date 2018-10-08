using namespace std;

#include "BinaryDocValuesFieldUpdates.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PagedGrowableWriter =
    org::apache::lucene::util::packed::PagedGrowableWriter;
using PagedMutable = org::apache::lucene::util::packed::PagedMutable;

BinaryDocValuesFieldUpdates::Iterator::Iterator(
    int size, shared_ptr<PagedGrowableWriter> offsets,
    shared_ptr<PagedGrowableWriter> lengths, shared_ptr<PagedMutable> docs,
    shared_ptr<BytesRef> values, int64_t delGen)
    : DocValuesFieldUpdates::AbstractIterator(size, docs, delGen),
      offsets(offsets), lengths(lengths), value(values->clone())
{
}

shared_ptr<BytesRef> BinaryDocValuesFieldUpdates::Iterator::binaryValue()
{
  value->offset = offset;
  value->length = length;
  return value;
}

void BinaryDocValuesFieldUpdates::Iterator::set(int64_t idx)
{
  offset = static_cast<int>(offsets->get(idx));
  length = static_cast<int>(lengths->get(idx));
}

int64_t BinaryDocValuesFieldUpdates::Iterator::longValue()
{
  throw make_shared<UnsupportedOperationException>();
}

BinaryDocValuesFieldUpdates::BinaryDocValuesFieldUpdates(int64_t delGen,
                                                         const wstring &field,
                                                         int maxDoc)
    : DocValuesFieldUpdates(maxDoc, delGen, field, DocValuesType::BINARY)
{
  offsets = make_shared<PagedGrowableWriter>(1, PAGE_SIZE, 1, PackedInts::FAST);
  lengths = make_shared<PagedGrowableWriter>(1, PAGE_SIZE, 1, PackedInts::FAST);
  values = make_shared<BytesRefBuilder>();
}

void BinaryDocValuesFieldUpdates::add(int doc, int64_t value)
{
  throw make_shared<UnsupportedOperationException>();
}

void BinaryDocValuesFieldUpdates::add(
    int docId, shared_ptr<DocValuesFieldUpdates::Iterator> iterator)
{
  add(docId, iterator->binaryValue());
}

// C++ WARNING: The following method was originally marked 'synchronized':
void BinaryDocValuesFieldUpdates::add(int doc, shared_ptr<BytesRef> value)
{
  int index = add(doc);
  offsets->set(index, values->length());
  lengths->set(index, value->length);
  values->append(value);
}

void BinaryDocValuesFieldUpdates::swap(int i, int j)
{
  DocValuesFieldUpdates::swap(i, j);

  int64_t tmpOffset = offsets->get(j);
  offsets->set(j, offsets->get(i));
  offsets->set(i, tmpOffset);

  int64_t tmpLength = lengths->get(j);
  lengths->set(j, lengths->get(i));
  lengths->set(i, tmpLength);
}

void BinaryDocValuesFieldUpdates::grow(int size)
{
  DocValuesFieldUpdates::grow(size);
  offsets = offsets->grow(size);
  lengths = lengths->grow(size);
}

void BinaryDocValuesFieldUpdates::resize(int size)
{
  DocValuesFieldUpdates::resize(size);
  offsets = offsets->resize(size);
  lengths = lengths->resize(size);
}

shared_ptr<Iterator> BinaryDocValuesFieldUpdates::iterator()
{
  ensureFinished();
  return make_shared<Iterator>(size_, offsets, lengths, docs, values->get(),
                               delGen);
}

int64_t BinaryDocValuesFieldUpdates::ramBytesUsed()
{
  return DocValuesFieldUpdates::ramBytesUsed() + offsets->ramBytesUsed() +
         lengths->ramBytesUsed() + RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
         2 * Integer::BYTES + 3 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
         values->bytes().size();
}
} // namespace org::apache::lucene::index