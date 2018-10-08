using namespace std;

#include "CompressingStoredFieldsIndexWriter.h"
#include "../../store/IndexOutput.h"
#include "../../util/packed/PackedInts.h"
#include "../CodecUtil.h"

namespace org::apache::lucene::codecs::compressing
{
//    import static org.apache.lucene.util.BitUtil.zigZagEncode;
using Codec = org::apache::lucene::codecs::Codec;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

CompressingStoredFieldsIndexWriter::CompressingStoredFieldsIndexWriter(
    shared_ptr<IndexOutput> indexOutput, int blockSize) 
    : fieldsIndexOut(indexOutput), blockSize(blockSize),
      docBaseDeltas(std::deque<int>(blockSize)),
      startPointerDeltas(std::deque<int64_t>(blockSize))
{
  if (blockSize <= 0) {
    throw invalid_argument(L"blockSize must be positive");
  }
  reset();
  totalDocs = 0;
  fieldsIndexOut->writeVInt(PackedInts::VERSION_CURRENT);
}

void CompressingStoredFieldsIndexWriter::reset()
{
  blockChunks = 0;
  blockDocs = 0;
  firstStartPointer = -1; // means unset
}

void CompressingStoredFieldsIndexWriter::writeBlock() 
{
  assert(blockChunks > 0);
  fieldsIndexOut->writeVInt(blockChunks);

  // The trick here is that we only store the difference from the average start
  // pointer or doc base, this helps save bits per value.
  // And in order to prevent a few chunks that would be far from the average to
  // raise the number of bits per value for all of them, we only encode blocks
  // of 1024 chunks at once
  // See LUCENE-4512

  // doc bases
  constexpr int avgChunkDocs;
  if (blockChunks == 1) {
    avgChunkDocs = 0;
  } else {
    avgChunkDocs =
        round(static_cast<float>(blockDocs - docBaseDeltas[blockChunks - 1]) /
              (blockChunks - 1));
  }
  fieldsIndexOut->writeVInt(totalDocs - blockDocs); // docBase
  fieldsIndexOut->writeVInt(avgChunkDocs);
  int docBase = 0;
  int64_t maxDelta = 0;
  for (int i = 0; i < blockChunks; ++i) {
    constexpr int delta = docBase - avgChunkDocs * i;
    maxDelta |= zigZagEncode(delta);
    docBase += docBaseDeltas[i];
  }

  constexpr int bitsPerDocBase = PackedInts::bitsRequired(maxDelta);
  fieldsIndexOut->writeVInt(bitsPerDocBase);
  shared_ptr<PackedInts::Writer> writer =
      PackedInts::getWriterNoHeader(fieldsIndexOut, PackedInts::Format::PACKED,
                                    blockChunks, bitsPerDocBase, 1);
  docBase = 0;
  for (int i = 0; i < blockChunks; ++i) {
    constexpr int64_t delta = docBase - avgChunkDocs * i;
    assert(PackedInts::bitsRequired(zigZagEncode(delta)) <=
           writer->bitsPerValue());
    writer->add(zigZagEncode(delta));
    docBase += docBaseDeltas[i];
  }
  writer->finish();

  // start pointers
  fieldsIndexOut->writeVLong(firstStartPointer);
  constexpr int64_t avgChunkSize;
  if (blockChunks == 1) {
    avgChunkSize = 0;
  } else {
    avgChunkSize = (maxStartPointer - firstStartPointer) / (blockChunks - 1);
  }
  fieldsIndexOut->writeVLong(avgChunkSize);
  int64_t startPointer = 0;
  maxDelta = 0;
  for (int i = 0; i < blockChunks; ++i) {
    startPointer += startPointerDeltas[i];
    constexpr int64_t delta = startPointer - avgChunkSize * i;
    maxDelta |= zigZagEncode(delta);
  }

  constexpr int bitsPerStartPointer = PackedInts::bitsRequired(maxDelta);
  fieldsIndexOut->writeVInt(bitsPerStartPointer);
  writer =
      PackedInts::getWriterNoHeader(fieldsIndexOut, PackedInts::Format::PACKED,
                                    blockChunks, bitsPerStartPointer, 1);
  startPointer = 0;
  for (int i = 0; i < blockChunks; ++i) {
    startPointer += startPointerDeltas[i];
    constexpr int64_t delta = startPointer - avgChunkSize * i;
    assert(PackedInts::bitsRequired(zigZagEncode(delta)) <=
           writer->bitsPerValue());
    writer->add(zigZagEncode(delta));
  }
  writer->finish();
}

void CompressingStoredFieldsIndexWriter::writeIndex(
    int numDocs, int64_t startPointer) 
{
  if (blockChunks == blockSize) {
    writeBlock();
    reset();
  }

  if (firstStartPointer == -1) {
    firstStartPointer = maxStartPointer = startPointer;
  }
  assert(firstStartPointer > 0 && startPointer >= firstStartPointer);

  docBaseDeltas[blockChunks] = numDocs;
  startPointerDeltas[blockChunks] = startPointer - maxStartPointer;

  ++blockChunks;
  blockDocs += numDocs;
  totalDocs += numDocs;
  maxStartPointer = startPointer;
}

void CompressingStoredFieldsIndexWriter::finish(
    int numDocs, int64_t maxPointer) 
{
  if (numDocs != totalDocs) {
    throw make_shared<IllegalStateException>(
        L"Expected " + to_wstring(numDocs) + L" docs, but got " +
        to_wstring(totalDocs));
  }
  if (blockChunks > 0) {
    writeBlock();
  }
  fieldsIndexOut->writeVInt(0); // end marker
  fieldsIndexOut->writeVLong(maxPointer);
  CodecUtil::writeFooter(fieldsIndexOut);
}

CompressingStoredFieldsIndexWriter::~CompressingStoredFieldsIndexWriter()
{
  delete fieldsIndexOut;
}
} // namespace org::apache::lucene::codecs::compressing