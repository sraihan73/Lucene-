using namespace std;

#include "DocIdsWriter.h"

namespace org::apache::lucene::util::bkd
{
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;

DocIdsWriter::DocIdsWriter() {}

void DocIdsWriter::writeDocIds(std::deque<int> &docIds, int start, int count,
                               shared_ptr<DataOutput> out) 
{
  // docs can be sorted either when all docs in a block have the same value
  // or when a segment is sorted
  bool sorted = true;
  for (int i = 1; i < count; ++i) {
    if (docIds[start + i - 1] > docIds[start + i]) {
      sorted = false;
      break;
    }
  }
  if (sorted) {
    out->writeByte(static_cast<char>(0));
    int previous = 0;
    for (int i = 0; i < count; ++i) {
      int doc = docIds[start + i];
      out->writeVInt(doc - previous);
      previous = doc;
    }
  } else {
    int64_t max = 0;
    for (int i = 0; i < count; ++i) {
      max |= Integer::toUnsignedLong(docIds[start + i]);
    }
    if (max <= 0xffffff) {
      out->writeByte(static_cast<char>(24));
      for (int i = 0; i < count; ++i) {
        out->writeShort(static_cast<short>(static_cast<int>(
            static_cast<unsigned int>(docIds[start + i]) >> 8)));
        out->writeByte(static_cast<char>(docIds[start + i]));
      }
    } else {
      out->writeByte(static_cast<char>(32));
      for (int i = 0; i < count; ++i) {
        out->writeInt(docIds[start + i]);
      }
    }
  }
}

void DocIdsWriter::readInts(shared_ptr<IndexInput> in_, int count,
                            std::deque<int> &docIDs) 
{
  constexpr int bpv = in_->readByte();
  switch (bpv) {
  case 0:
    readDeltaVInts(in_, count, docIDs);
    break;
  case 32:
    readInts32(in_, count, docIDs);
    break;
  case 24:
    readInts24(in_, count, docIDs);
    break;
  default:
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Unsupported number of bits per value: " + bpv);
  }
}

void DocIdsWriter::readDeltaVInts(shared_ptr<IndexInput> in_, int count,
                                  std::deque<int> &docIDs) 
{
  int doc = 0;
  for (int i = 0; i < count; i++) {
    doc += in_->readVInt();
    docIDs[i] = doc;
  }
}

template <typename T>
void DocIdsWriter::readInts32(shared_ptr<IndexInput> in_, int count,
                              std::deque<int> &docIDs) 
{
  for (int i = 0; i < count; i++) {
    docIDs[i] = in_->readInt();
  }
}

void DocIdsWriter::readInts24(shared_ptr<IndexInput> in_, int count,
                              std::deque<int> &docIDs) 
{
  int i;
  for (i = 0; i < count - 7; i += 8) {
    int64_t l1 = in_->readLong();
    int64_t l2 = in_->readLong();
    int64_t l3 = in_->readLong();
    docIDs[i] = static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(l1) >> 40));
    docIDs[i + 1] = static_cast<int>(static_cast<int64_t>(
                        static_cast<uint64_t>(l1) >> 16)) &
                    0xffffff;
    docIDs[i + 2] = static_cast<int>(
        ((l1 & 0xffff) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(l2) >> 56)));
    docIDs[i + 3] = static_cast<int>(static_cast<int64_t>(
                        static_cast<uint64_t>(l2) >> 32)) &
                    0xffffff;
    docIDs[i + 4] = static_cast<int>(static_cast<int64_t>(
                        static_cast<uint64_t>(l2) >> 8)) &
                    0xffffff;
    docIDs[i + 5] = static_cast<int>(
        ((l2 & 0xff) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(l3) >> 48)));
    docIDs[i + 6] = static_cast<int>(static_cast<int64_t>(
                        static_cast<uint64_t>(l3) >> 24)) &
                    0xffffff;
    docIDs[i + 7] = static_cast<int>(l3) & 0xffffff;
  }
  for (; i < count; ++i) {
    docIDs[i] = (Short::toUnsignedInt(in_->readShort()) << 8) |
                Byte::toUnsignedInt(in_->readByte());
  }
}

void DocIdsWriter::readInts(
    shared_ptr<IndexInput> in_, int count,
    shared_ptr<IntersectVisitor> visitor) 
{
  constexpr int bpv = in_->readByte();
  switch (bpv) {
  case 0:
    readDeltaVInts(in_, count, visitor);
    break;
  case 32:
    readInts32(in_, count, visitor);
    break;
  case 24:
    readInts24(in_, count, visitor);
    break;
  default:
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Unsupported number of bits per value: " + bpv);
  }
}

void DocIdsWriter::readDeltaVInts(
    shared_ptr<IndexInput> in_, int count,
    shared_ptr<IntersectVisitor> visitor) 
{
  int doc = 0;
  for (int i = 0; i < count; i++) {
    doc += in_->readVInt();
    visitor->visit(doc);
  }
}

void DocIdsWriter::readInts32(
    shared_ptr<IndexInput> in_, int count,
    shared_ptr<IntersectVisitor> visitor) 
{
  for (int i = 0; i < count; i++) {
    visitor->visit(in_->readInt());
  }
}

void DocIdsWriter::readInts24(
    shared_ptr<IndexInput> in_, int count,
    shared_ptr<IntersectVisitor> visitor) 
{
  int i;
  for (i = 0; i < count - 7; i += 8) {
    int64_t l1 = in_->readLong();
    int64_t l2 = in_->readLong();
    int64_t l3 = in_->readLong();
    visitor->visit(static_cast<int>(
        static_cast<int64_t>(static_cast<uint64_t>(l1) >> 40)));
    visitor->visit(static_cast<int>(static_cast<int64_t>(
                       static_cast<uint64_t>(l1) >> 16)) &
                   0xffffff);
    visitor->visit(static_cast<int>(
        ((l1 & 0xffff) << 8) |
        (static_cast<int64_t>(static_cast<uint64_t>(l2) >> 56))));
    visitor->visit(static_cast<int>(static_cast<int64_t>(
                       static_cast<uint64_t>(l2) >> 32)) &
                   0xffffff);
    visitor->visit(static_cast<int>(static_cast<int64_t>(
                       static_cast<uint64_t>(l2) >> 8)) &
                   0xffffff);
    visitor->visit(static_cast<int>(
        ((l2 & 0xff) << 16) |
        (static_cast<int64_t>(static_cast<uint64_t>(l3) >> 48))));
    visitor->visit(static_cast<int>(static_cast<int64_t>(
                       static_cast<uint64_t>(l3) >> 24)) &
                   0xffffff);
    visitor->visit(static_cast<int>(l3) & 0xffffff);
  }
  for (; i < count; ++i) {
    visitor->visit((Short::toUnsignedInt(in_->readShort()) << 8) |
                   Byte::toUnsignedInt(in_->readByte()));
  }
}
} // namespace org::apache::lucene::util::bkd