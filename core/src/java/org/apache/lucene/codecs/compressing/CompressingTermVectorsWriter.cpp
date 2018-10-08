using namespace std;

#include "CompressingTermVectorsWriter.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/Fields.h"
#include "../../index/IndexFileNames.h"
#include "../../index/MergeState.h"
#include "../../index/SegmentInfo.h"
#include "../../store/DataInput.h"
#include "../../store/Directory.h"
#include "../../store/GrowableByteArrayDataOutput.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../../store/IndexOutput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/Bits.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../../util/StringHelper.h"
#include "../../util/packed/BlockPackedWriter.h"
#include "../CodecUtil.h"
#include "../TermVectorsReader.h"
#include "CompressingStoredFieldsIndexReader.h"
#include "CompressingStoredFieldsIndexWriter.h"
#include "CompressingTermVectorsReader.h"
#include "CompressionMode.h"
#include "Compressor.h"
#include "MatchingReaders.h"

namespace org::apache::lucene::codecs::compressing
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
using BlockPackedWriter = org::apache::lucene::util::packed::BlockPackedWriter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
const wstring CompressingTermVectorsWriter::VECTORS_EXTENSION = L"tvd";
const wstring CompressingTermVectorsWriter::VECTORS_INDEX_EXTENSION = L"tvx";
const wstring CompressingTermVectorsWriter::CODEC_SFX_IDX = L"Index";
const wstring CompressingTermVectorsWriter::CODEC_SFX_DAT = L"Data";

CompressingTermVectorsWriter::DocData::DocData(
    shared_ptr<CompressingTermVectorsWriter> outerInstance, int numFields,
    int posStart, int offStart, int payStart)
    : numFields(numFields), fields(make_shared<ArrayDeque<>>(numFields)),
      posStart(posStart), offStart(offStart), payStart(payStart),
      outerInstance(outerInstance)
{
}

shared_ptr<FieldData> CompressingTermVectorsWriter::DocData::addField(
    int fieldNum, int numTerms, bool positions, bool offsets, bool payloads)
{
  shared_ptr<FieldData> *const field;
  if (fields->isEmpty()) {
    field =
        make_shared<FieldData>(outerInstance, fieldNum, numTerms, positions,
                               offsets, payloads, posStart, offStart, payStart);
  } else {
    shared_ptr<FieldData> *const last = fields->getLast();
    constexpr int posStart =
        last->posStart + (last->hasPositions ? last->totalPositions : 0);
    constexpr int offStart =
        last->offStart + (last->hasOffsets ? last->totalPositions : 0);
    constexpr int payStart =
        last->payStart + (last->hasPayloads ? last->totalPositions : 0);
    field =
        make_shared<FieldData>(outerInstance, fieldNum, numTerms, positions,
                               offsets, payloads, posStart, offStart, payStart);
  }
  fields->add(field);
  return field;
}

shared_ptr<DocData>
CompressingTermVectorsWriter::addDocData(int numVectorFields)
{
  shared_ptr<FieldData> last = nullptr;
  for (shared_ptr<Iterator<std::shared_ptr<DocData>>> it =
           pendingDocs->descendingIterator();
       it->hasNext();) {
    shared_ptr<DocData> *const doc = it->next();
    if (!doc->fields->isEmpty()) {
      last = doc->fields->getLast();
      break;
    }
  }
  shared_ptr<DocData> *const doc;
  if (last == nullptr) {
    doc = make_shared<DocData>(shared_from_this(), numVectorFields, 0, 0, 0);
  } else {
    constexpr int posStart =
        last->posStart + (last->hasPositions ? last->totalPositions : 0);
    constexpr int offStart =
        last->offStart + (last->hasOffsets ? last->totalPositions : 0);
    constexpr int payStart =
        last->payStart + (last->hasPayloads ? last->totalPositions : 0);
    doc = make_shared<DocData>(shared_from_this(), numVectorFields, posStart,
                               offStart, payStart);
  }
  pendingDocs->add(doc);
  return doc;
}

CompressingTermVectorsWriter::FieldData::FieldData(
    shared_ptr<CompressingTermVectorsWriter> outerInstance, int fieldNum,
    int numTerms, bool positions, bool offsets, bool payloads, int posStart,
    int offStart, int payStart)
    : hasPositions(positions), hasOffsets(offsets), hasPayloads(payloads),
      fieldNum(fieldNum),
      flags((positions ? POSITIONS : 0) | (offsets ? OFFSETS : 0) |
            (payloads ? PAYLOADS : 0)),
      numTerms(numTerms), freqs(std::deque<int>(numTerms)),
      prefixLengths(std::deque<int>(numTerms)),
      suffixLengths(std::deque<int>(numTerms)), posStart(posStart),
      offStart(offStart), payStart(payStart), outerInstance(outerInstance)
{
  totalPositions = 0;
  ord = 0;
}

void CompressingTermVectorsWriter::FieldData::addTerm(int freq,
                                                      int prefixLength,
                                                      int suffixLength)
{
  freqs[ord] = freq;
  prefixLengths[ord] = prefixLength;
  suffixLengths[ord] = suffixLength;
  ++ord;
}

void CompressingTermVectorsWriter::FieldData::addPosition(int position,
                                                          int startOffset,
                                                          int length,
                                                          int payloadLength)
{
  if (hasPositions) {
    if (posStart + totalPositions == outerInstance->positionsBuf.size()) {
      outerInstance->positionsBuf =
          ArrayUtil::grow(outerInstance->positionsBuf);
    }
    outerInstance->positionsBuf[posStart + totalPositions] = position;
  }
  if (hasOffsets) {
    if (offStart + totalPositions == outerInstance->startOffsetsBuf.size()) {
      constexpr int newLength =
          ArrayUtil::oversize(offStart + totalPositions, 4);
      outerInstance->startOffsetsBuf =
          Arrays::copyOf(outerInstance->startOffsetsBuf, newLength);
      outerInstance->lengthsBuf =
          Arrays::copyOf(outerInstance->lengthsBuf, newLength);
    }
    outerInstance->startOffsetsBuf[offStart + totalPositions] = startOffset;
    outerInstance->lengthsBuf[offStart + totalPositions] = length;
  }
  if (hasPayloads) {
    if (payStart + totalPositions == outerInstance->payloadLengthsBuf.size()) {
      outerInstance->payloadLengthsBuf =
          ArrayUtil::grow(outerInstance->payloadLengthsBuf);
    }
    outerInstance->payloadLengthsBuf[payStart + totalPositions] = payloadLength;
  }
  ++totalPositions;
}

CompressingTermVectorsWriter::CompressingTermVectorsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    const wstring &segmentSuffix, shared_ptr<IOContext> context,
    const wstring &formatName, shared_ptr<CompressionMode> compressionMode,
    int chunkSize, int blockSize) 
    : segment(si->name), compressionMode(compressionMode),
      compressor(compressionMode->newCompressor()), chunkSize(chunkSize),
      pendingDocs(make_shared<ArrayDeque<>>()),
      lastTerm(make_shared<BytesRef>(ArrayUtil::oversize(30, 1))),
      termSuffixes(make_shared<GrowableByteArrayDataOutput>(
          ArrayUtil::oversize(chunkSize, 1))),
      payloadBytes(
          make_shared<GrowableByteArrayDataOutput>(ArrayUtil::oversize(1, 1)))
{
  assert(directory != nullptr);

  numDocs = 0;

  bool success = false;
  shared_ptr<IndexOutput> indexStream = directory->createOutput(
      IndexFileNames::segmentFileName(segment, segmentSuffix,
                                      VECTORS_INDEX_EXTENSION),
      context);
  try {
    vectorsStream =
        directory->createOutput(IndexFileNames::segmentFileName(
                                    segment, segmentSuffix, VECTORS_EXTENSION),
                                context);

    const wstring codecNameIdx = formatName + CODEC_SFX_IDX;
    const wstring codecNameDat = formatName + CODEC_SFX_DAT;
    CodecUtil::writeIndexHeader(indexStream, codecNameIdx, VERSION_CURRENT,
                                si->getId(), segmentSuffix);
    CodecUtil::writeIndexHeader(vectorsStream, codecNameDat, VERSION_CURRENT,
                                si->getId(), segmentSuffix);
    assert((CodecUtil::indexHeaderLength(codecNameDat, segmentSuffix) ==
            vectorsStream->getFilePointer()));
    assert((CodecUtil::indexHeaderLength(codecNameIdx, segmentSuffix) ==
            indexStream->getFilePointer()));

    indexWriter =
        make_shared<CompressingStoredFieldsIndexWriter>(indexStream, blockSize);
    indexStream.reset();

    vectorsStream->writeVInt(PackedInts::VERSION_CURRENT);
    vectorsStream->writeVInt(chunkSize);
    writer = make_shared<BlockPackedWriter>(vectorsStream, PACKED_BLOCK_SIZE);

    positionsBuf = std::deque<int>(1024);
    startOffsetsBuf = std::deque<int>(1024);
    lengthsBuf = std::deque<int>(1024);
    payloadLengthsBuf = std::deque<int>(1024);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException(
          {vectorsStream, indexStream, indexWriter});
    }
  }
}

CompressingTermVectorsWriter::~CompressingTermVectorsWriter()
{
  try {
    IOUtils::close({vectorsStream, indexWriter});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    vectorsStream.reset();
    indexWriter.reset();
  }
}

void CompressingTermVectorsWriter::startDocument(int numVectorFields) throw(
    IOException)
{
  curDoc = addDocData(numVectorFields);
}

void CompressingTermVectorsWriter::finishDocument() 
{
  // append the payload bytes of the doc after its terms
  termSuffixes->writeBytes(payloadBytes->getBytes(),
                           payloadBytes->getPosition());
  payloadBytes->reset();
  ++numDocs;
  if (triggerFlush()) {
    flush();
  }
  curDoc.reset();
}

void CompressingTermVectorsWriter::startField(shared_ptr<FieldInfo> info,
                                              int numTerms, bool positions,
                                              bool offsets,
                                              bool payloads) 
{
  curField =
      curDoc->addField(info->number, numTerms, positions, offsets, payloads);
  lastTerm->length = 0;
}

void CompressingTermVectorsWriter::finishField() 
{
  curField.reset();
}

void CompressingTermVectorsWriter::startTerm(shared_ptr<BytesRef> term,
                                             int freq) 
{
  assert(freq >= 1);
  constexpr int prefix;
  if (lastTerm->length == 0) {
    // no previous term: no bytes to write
    prefix = 0;
  } else {
    prefix = StringHelper::bytesDifference(lastTerm, term);
  }
  curField->addTerm(freq, prefix, term->length - prefix);
  termSuffixes->writeBytes(term->bytes, term->offset + prefix,
                           term->length - prefix);
  // copy last term
  if (lastTerm->bytes.size() < term->length) {
    lastTerm->bytes = std::deque<char>(ArrayUtil::oversize(term->length, 1));
  }
  lastTerm->offset = 0;
  lastTerm->length = term->length;
  System::arraycopy(term->bytes, term->offset, lastTerm->bytes, 0,
                    term->length);
}

void CompressingTermVectorsWriter::addPosition(
    int position, int startOffset, int endOffset,
    shared_ptr<BytesRef> payload) 
{
  assert(curField->flags != 0);
  curField->addPosition(position, startOffset, endOffset - startOffset,
                        payload == nullptr ? 0 : payload->length);
  if (curField->hasPayloads && payload != nullptr) {
    payloadBytes->writeBytes(payload->bytes, payload->offset, payload->length);
  }
}

bool CompressingTermVectorsWriter::triggerFlush()
{
  return termSuffixes->getPosition() >= chunkSize ||
         pendingDocs->size() >= MAX_DOCUMENTS_PER_CHUNK;
}

void CompressingTermVectorsWriter::flush() 
{
  constexpr int chunkDocs = pendingDocs->size();
  assert((chunkDocs > 0, chunkDocs));

  // write the index file
  indexWriter->writeIndex(chunkDocs, vectorsStream->getFilePointer());

  constexpr int docBase = numDocs - chunkDocs;
  vectorsStream->writeVInt(docBase);
  vectorsStream->writeVInt(chunkDocs);

  // total number of fields of the chunk
  constexpr int totalFields = flushNumFields(chunkDocs);

  if (totalFields > 0) {
    // unique field numbers (sorted)
    const std::deque<int> fieldNums = flushFieldNums();
    // offsets in the array of unique field numbers
    flushFields(totalFields, fieldNums);
    // flags (does the field have positions, offsets, payloads?)
    flushFlags(totalFields, fieldNums);
    // number of terms of each field
    flushNumTerms(totalFields);
    // prefix and suffix lengths for each field
    flushTermLengths();
    // term freqs - 1 (because termFreq is always >=1) for each term
    flushTermFreqs();
    // positions for all terms, when enabled
    flushPositions();
    // offsets for all terms, when enabled
    flushOffsets(fieldNums);
    // payload lengths for all terms, when enabled
    flushPayloadLengths();

    // compress terms and payloads and write them to the output
    compressor->compress(termSuffixes->getBytes(), 0,
                         termSuffixes->getPosition(), vectorsStream);
  }

  // reset
  pendingDocs->clear();
  curDoc.reset();
  curField.reset();
  termSuffixes->reset();
  numChunks++;
}

int CompressingTermVectorsWriter::flushNumFields(int chunkDocs) throw(
    IOException)
{
  if (chunkDocs == 1) {
    constexpr int numFields = pendingDocs->getFirst().numFields;
    vectorsStream->writeVInt(numFields);
    return numFields;
  } else {
    writer->reset(vectorsStream);
    int totalFields = 0;
    for (auto dd : pendingDocs) {
      writer->add(dd->numFields);
      totalFields += dd->numFields;
    }
    writer->finish();
    return totalFields;
  }
}

std::deque<int>
CompressingTermVectorsWriter::flushFieldNums() 
{
  shared_ptr<SortedSet<int>> fieldNums = set<int>();
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      fieldNums->add(fd->fieldNum);
    }
  }

  constexpr int numDistinctFields = fieldNums->size();
  assert(numDistinctFields > 0);
  constexpr int bitsRequired = PackedInts::bitsRequired(fieldNums->last());
  constexpr int token = (min(numDistinctFields - 1, 0x07) << 5) | bitsRequired;
  vectorsStream->writeByte(static_cast<char>(token));
  if (numDistinctFields - 1 >= 0x07) {
    vectorsStream->writeVInt(numDistinctFields - 1 - 0x07);
  }
  shared_ptr<PackedInts::Writer> *const writer =
      PackedInts::getWriterNoHeader(vectorsStream, PackedInts::Format::PACKED,
                                    fieldNums->size(), bitsRequired, 1);
  for (shared_ptr<> : : optional<int> fieldNum : fieldNums) {
    writer->add(fieldNum);
  }
  writer->finish();

  std::deque<int> fns(fieldNums->size());
  int i = 0;
  for (shared_ptr<> : : optional<int> key : fieldNums) {
    fns[i++] = key;
  }
  return fns;
}

void CompressingTermVectorsWriter::flushFields(
    int totalFields, std::deque<int> &fieldNums) 
{
  shared_ptr<PackedInts::Writer> *const writer = PackedInts::getWriterNoHeader(
      vectorsStream, PackedInts::Format::PACKED, totalFields,
      PackedInts::bitsRequired(fieldNums.size() - 1), 1);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      constexpr int fieldNumIndex =
          Arrays::binarySearch(fieldNums, fd->fieldNum);
      assert(fieldNumIndex >= 0);
      writer->add(fieldNumIndex);
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::flushFlags(
    int totalFields, std::deque<int> &fieldNums) 
{
  // check if fields always have the same flags
  bool nonChangingFlags = true;
  std::deque<int> fieldFlags(fieldNums.size());
  Arrays::fill(fieldFlags, -1);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      constexpr int fieldNumOff = Arrays::binarySearch(fieldNums, fd->fieldNum);
      assert(fieldNumOff >= 0);
      if (fieldFlags[fieldNumOff] == -1) {
        fieldFlags[fieldNumOff] = fd->flags;
      } else if (fieldFlags[fieldNumOff] != fd->flags) {
        nonChangingFlags = false;
        goto outerBreak;
      }
    }
  outerContinue:;
  }
outerBreak:

  if (nonChangingFlags) {
    // write one flag per field num
    vectorsStream->writeVInt(0);
    shared_ptr<PackedInts::Writer> *const writer =
        PackedInts::getWriterNoHeader(vectorsStream, PackedInts::Format::PACKED,
                                      fieldFlags.size(), FLAGS_BITS, 1);
    for (auto flags : fieldFlags) {
      assert(flags >= 0);
      writer->add(flags);
    }
    assert(writer->ord() == fieldFlags.size() - 1);
    writer->finish();
  } else {
    // write one flag for every field instance
    vectorsStream->writeVInt(1);
    shared_ptr<PackedInts::Writer> *const writer =
        PackedInts::getWriterNoHeader(vectorsStream, PackedInts::Format::PACKED,
                                      totalFields, FLAGS_BITS, 1);
    for (auto dd : pendingDocs) {
      for (auto fd : dd->fields) {
        writer->add(fd->flags);
      }
    }
    assert(writer->ord() == totalFields - 1);
    writer->finish();
  }
}

void CompressingTermVectorsWriter::flushNumTerms(int totalFields) throw(
    IOException)
{
  int maxNumTerms = 0;
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      maxNumTerms |= fd->numTerms;
    }
  }
  constexpr int bitsRequired = PackedInts::bitsRequired(maxNumTerms);
  vectorsStream->writeVInt(bitsRequired);
  shared_ptr<PackedInts::Writer> *const writer = PackedInts::getWriterNoHeader(
      vectorsStream, PackedInts::Format::PACKED, totalFields, bitsRequired, 1);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      writer->add(fd->numTerms);
    }
  }
  assert(writer->ord() == totalFields - 1);
  writer->finish();
}

void CompressingTermVectorsWriter::flushTermLengths() 
{
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      for (int i = 0; i < fd->numTerms; ++i) {
        writer->add(fd->prefixLengths[i]);
      }
    }
  }
  writer->finish();
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      for (int i = 0; i < fd->numTerms; ++i) {
        writer->add(fd->suffixLengths[i]);
      }
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::flushTermFreqs() 
{
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      for (int i = 0; i < fd->numTerms; ++i) {
        writer->add(fd->freqs[i] - 1);
      }
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::flushPositions() 
{
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      if (fd->hasPositions) {
        int pos = 0;
        for (int i = 0; i < fd->numTerms; ++i) {
          int previousPosition = 0;
          for (int j = 0; j < fd->freqs[i]; ++j) {
            constexpr int position = positionsBuf[fd->posStart + pos++];
            writer->add(position - previousPosition);
            previousPosition = position;
          }
        }
        assert(pos == fd->totalPositions);
      }
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::flushOffsets(
    std::deque<int> &fieldNums) 
{
  bool hasOffsets = false;
  std::deque<int64_t> sumPos(fieldNums.size());
  std::deque<int64_t> sumOffsets(fieldNums.size());
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      hasOffsets |= fd->hasOffsets;
      if (fd->hasOffsets && fd->hasPositions) {
        constexpr int fieldNumOff =
            Arrays::binarySearch(fieldNums, fd->fieldNum);
        int pos = 0;
        for (int i = 0; i < fd->numTerms; ++i) {
          int previousPos = 0;
          int previousOff = 0;
          for (int j = 0; j < fd->freqs[i]; ++j) {
            constexpr int position = positionsBuf[fd->posStart + pos];
            constexpr int startOffset = startOffsetsBuf[fd->offStart + pos];
            sumPos[fieldNumOff] += position - previousPos;
            sumOffsets[fieldNumOff] += startOffset - previousOff;
            previousPos = position;
            previousOff = startOffset;
            ++pos;
          }
        }
        assert(pos == fd->totalPositions);
      }
    }
  }

  if (!hasOffsets) {
    // nothing to do
    return;
  }

  const std::deque<float> charsPerTerm = std::deque<float>(fieldNums.size());
  for (int i = 0; i < fieldNums.size(); ++i) {
    charsPerTerm[i] = (sumPos[i] <= 0 || sumOffsets[i] <= 0)
                          ? 0
                          : static_cast<float>(
                                static_cast<double>(sumOffsets[i]) / sumPos[i]);
  }

  // start offsets
  for (int i = 0; i < fieldNums.size(); ++i) {
    vectorsStream->writeInt(Float::floatToRawIntBits(charsPerTerm[i]));
  }

  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      if ((fd->flags & OFFSETS) != 0) {
        constexpr int fieldNumOff =
            Arrays::binarySearch(fieldNums, fd->fieldNum);
        constexpr float cpt = charsPerTerm[fieldNumOff];
        int pos = 0;
        for (int i = 0; i < fd->numTerms; ++i) {
          int previousPos = 0;
          int previousOff = 0;
          for (int j = 0; j < fd->freqs[i]; ++j) {
            constexpr int position =
                fd->hasPositions ? positionsBuf[fd->posStart + pos] : 0;
            constexpr int startOffset = startOffsetsBuf[fd->offStart + pos];
            writer->add(startOffset - previousOff -
                        static_cast<int>(cpt * (position - previousPos)));
            previousPos = position;
            previousOff = startOffset;
            ++pos;
          }
        }
      }
    }
  }
  writer->finish();

  // lengths
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      if ((fd->flags & OFFSETS) != 0) {
        int pos = 0;
        for (int i = 0; i < fd->numTerms; ++i) {
          for (int j = 0; j < fd->freqs[i]; ++j) {
            writer->add(lengthsBuf[fd->offStart + pos++] -
                        fd->prefixLengths[i] - fd->suffixLengths[i]);
          }
        }
        assert(pos == fd->totalPositions);
      }
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::flushPayloadLengths() 
{
  writer->reset(vectorsStream);
  for (auto dd : pendingDocs) {
    for (auto fd : dd->fields) {
      if (fd->hasPayloads) {
        for (int i = 0; i < fd->totalPositions; ++i) {
          writer->add(payloadLengthsBuf[fd->payStart + i]);
        }
      }
    }
  }
  writer->finish();
}

void CompressingTermVectorsWriter::finish(shared_ptr<FieldInfos> fis,
                                          int numDocs) 
{
  if (!pendingDocs->isEmpty()) {
    flush();
    numDirtyChunks++; // incomplete: we had to force this flush
  }
  if (numDocs != this->numDocs) {
    throw runtime_error(L"Wrote " + to_wstring(this->numDocs) +
                        L" docs, finish called with numDocs=" +
                        to_wstring(numDocs));
  }
  indexWriter->finish(numDocs, vectorsStream->getFilePointer());
  vectorsStream->writeVLong(numChunks);
  vectorsStream->writeVLong(numDirtyChunks);
  CodecUtil::writeFooter(vectorsStream);
}

void CompressingTermVectorsWriter::addProx(
    int numProx, shared_ptr<DataInput> positions,
    shared_ptr<DataInput> offsets) 
{
  assert(curField->hasPositions) == (positions != nullptr);
  assert(curField->hasOffsets) == (offsets != nullptr);

  if (curField->hasPositions) {
    constexpr int posStart = curField->posStart + curField->totalPositions;
    if (posStart + numProx > positionsBuf.size()) {
      positionsBuf = ArrayUtil::grow(positionsBuf, posStart + numProx);
    }
    int position = 0;
    if (curField->hasPayloads) {
      constexpr int payStart = curField->payStart + curField->totalPositions;
      if (payStart + numProx > payloadLengthsBuf.size()) {
        payloadLengthsBuf =
            ArrayUtil::grow(payloadLengthsBuf, payStart + numProx);
      }
      for (int i = 0; i < numProx; ++i) {
        constexpr int code = positions->readVInt();
        if ((code & 1) != 0) {
          // This position has a payload
          constexpr int payloadLength = positions->readVInt();
          payloadLengthsBuf[payStart + i] = payloadLength;
          payloadBytes->copyBytes(positions, payloadLength);
        } else {
          payloadLengthsBuf[payStart + i] = 0;
        }
        position += static_cast<int>(static_cast<unsigned int>(code) >> 1);
        positionsBuf[posStart + i] = position;
      }
    } else {
      for (int i = 0; i < numProx; ++i) {
        position += (static_cast<int>(
            static_cast<unsigned int>(positions->readVInt()) >> 1));
        positionsBuf[posStart + i] = position;
      }
    }
  }

  if (curField->hasOffsets) {
    constexpr int offStart = curField->offStart + curField->totalPositions;
    if (offStart + numProx > startOffsetsBuf.size()) {
      constexpr int newLength = ArrayUtil::oversize(offStart + numProx, 4);
      startOffsetsBuf = Arrays::copyOf(startOffsetsBuf, newLength);
      lengthsBuf = Arrays::copyOf(lengthsBuf, newLength);
    }
    int lastOffset = 0, startOffset, endOffset;
    for (int i = 0; i < numProx; ++i) {
      startOffset = lastOffset + offsets->readVInt();
      endOffset = startOffset + offsets->readVInt();
      lastOffset = endOffset;
      startOffsetsBuf[offStart + i] = startOffset;
      lengthsBuf[offStart + i] = endOffset - startOffset;
    }
  }

  curField->totalPositions += numProx;
}

const wstring CompressingTermVectorsWriter::BULK_MERGE_ENABLED_SYSPROP =
    CompressingTermVectorsWriter::typeid->getName() + L".enableBulkMerge";

CompressingTermVectorsWriter::StaticConstructor::StaticConstructor()
{
  bool v = true;
  try {
    v = static_cast<Boolean>(
        System::getProperty(BULK_MERGE_ENABLED_SYSPROP, L"true"));
  } catch (const SecurityException &ignored) {
  }
  BULK_MERGE_ENABLED = v;
}

CompressingTermVectorsWriter::StaticConstructor
    CompressingTermVectorsWriter::staticConstructor;

int CompressingTermVectorsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  if (mergeState->needsIndexSort) {
    // TODO: can we gain back some optos even if index is sorted?  E.g. if sort
    // results in large chunks of contiguous docs from one sub being copied
    // over...?
    return TermVectorsWriter::merge(mergeState);
  }
  int docCount = 0;
  int numReaders = mergeState->maxDocs.size();

  shared_ptr<MatchingReaders> matching =
      make_shared<MatchingReaders>(mergeState);

  for (int readerIndex = 0; readerIndex < numReaders; readerIndex++) {
    shared_ptr<CompressingTermVectorsReader> matchingVectorsReader = nullptr;
    shared_ptr<TermVectorsReader> *const vectorsReader =
        mergeState->termVectorsReaders[readerIndex];
    if (matching->matchingReaders[readerIndex]) {
      // we can only bulk-copy if the matching reader is also a
      // CompressingTermVectorsReader
      if (vectorsReader != nullptr &&
          std::dynamic_pointer_cast<CompressingTermVectorsReader>(
              vectorsReader) != nullptr) {
        matchingVectorsReader =
            std::static_pointer_cast<CompressingTermVectorsReader>(
                vectorsReader);
      }
    }

    constexpr int maxDoc = mergeState->maxDocs[readerIndex];
    shared_ptr<Bits> *const liveDocs = mergeState->liveDocs[readerIndex];

    if (matchingVectorsReader != nullptr &&
        matchingVectorsReader->getCompressionMode() == compressionMode &&
        matchingVectorsReader->getChunkSize() == chunkSize &&
        matchingVectorsReader->getVersion() == VERSION_CURRENT &&
        matchingVectorsReader->getPackedIntsVersion() ==
            PackedInts::VERSION_CURRENT &&
        BULK_MERGE_ENABLED && liveDocs == nullptr &&
        !tooDirty(matchingVectorsReader)) {
      // optimized merge, raw byte copy
      // its not worth fine-graining this if there are deletions.

      matchingVectorsReader->checkIntegrity();

      // flush any pending chunks
      if (!pendingDocs->isEmpty()) {
        flush();
        numDirtyChunks++; // incomplete: we had to force this flush
      }

      // iterate over each chunk. we use the vectors index to find chunk
      // boundaries, read the docstart + doccount from the chunk header (we
      // write a new header, since doc numbers will change), and just copy the
      // bytes directly.
      shared_ptr<IndexInput> rawDocs =
          matchingVectorsReader->getVectorsStream();
      shared_ptr<CompressingStoredFieldsIndexReader> index =
          matchingVectorsReader->getIndexReader();
      rawDocs->seek(index->getStartPointer(0));
      int docID = 0;
      while (docID < maxDoc) {
        // read header
        int base = rawDocs->readVInt();
        if (base != docID) {
          throw make_shared<CorruptIndexException>(
              L"invalid state: base=" + to_wstring(base) + L", docID=" +
                  to_wstring(docID),
              rawDocs);
        }
        int bufferedDocs = rawDocs->readVInt();

        // write a new index entry and new header for this chunk.
        indexWriter->writeIndex(bufferedDocs, vectorsStream->getFilePointer());
        vectorsStream->writeVInt(docCount); // rebase
        vectorsStream->writeVInt(bufferedDocs);
        docID += bufferedDocs;
        docCount += bufferedDocs;
        numDocs += bufferedDocs;

        if (docID > maxDoc) {
          throw make_shared<CorruptIndexException>(
              L"invalid state: base=" + to_wstring(base) + L", count=" +
                  to_wstring(bufferedDocs) + L", maxDoc=" + to_wstring(maxDoc),
              rawDocs);
        }

        // copy bytes until the next chunk boundary (or end of chunk data).
        // using the stored fields index for this isn't the most efficient, but
        // fast enough and is a source of redundancy for detecting bad things.
        constexpr int64_t end;
        if (docID == maxDoc) {
          end = matchingVectorsReader->getMaxPointer();
        } else {
          end = index->getStartPointer(docID);
        }
        vectorsStream->copyBytes(rawDocs, end - rawDocs->getFilePointer());
      }

      if (rawDocs->getFilePointer() != matchingVectorsReader->getMaxPointer()) {
        throw make_shared<CorruptIndexException>(
            L"invalid state: pos=" + to_wstring(rawDocs->getFilePointer()) +
                L", max=" + to_wstring(matchingVectorsReader->getMaxPointer()),
            rawDocs);
      }

      // since we bulk merged all chunks, we inherit any dirty ones from this
      // segment.
      numChunks += matchingVectorsReader->getNumChunks();
      numDirtyChunks += matchingVectorsReader->getNumDirtyChunks();
    } else {
      // naive merge...
      if (vectorsReader != nullptr) {
        vectorsReader->checkIntegrity();
      }
      for (int i = 0; i < maxDoc; i++) {
        if (liveDocs != nullptr && liveDocs->get(i) == false) {
          continue;
        }
        shared_ptr<Fields> vectors;
        if (vectorsReader == nullptr) {
          vectors.reset();
        } else {
          vectors = vectorsReader->get(i);
        }
        addAllDocVectors(vectors, mergeState);
        ++docCount;
      }
    }
  }
  finish(mergeState->mergeFieldInfos, docCount);
  return docCount;
}

bool CompressingTermVectorsWriter::tooDirty(
    shared_ptr<CompressingTermVectorsReader> candidate)
{
  // more than 1% dirty, or more than hard limit of 1024 dirty chunks
  return candidate->getNumDirtyChunks() > 1024 ||
         candidate->getNumDirtyChunks() * 100 > candidate->getNumChunks();
}
} // namespace org::apache::lucene::codecs::compressing