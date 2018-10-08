using namespace std;

#include "CompressingTermVectorsReader.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../store/AlreadyClosedException.h"
#include "../../store/ByteArrayDataInput.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountable.h"
#include "../../util/Accountables.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../../util/LongsRef.h"
#include "../../util/packed/BlockPackedReaderIterator.h"
#include "../../util/packed/PackedInts.h"
#include "../CodecUtil.h"
#include "CompressingStoredFieldsIndexReader.h"
#include "CompressionMode.h"
#include "Decompressor.h"

namespace org::apache::lucene::codecs::compressing
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LongsRef = org::apache::lucene::util::LongsRef;
using BlockPackedReaderIterator =
    org::apache::lucene::util::packed::BlockPackedReaderIterator;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.CODEC_SFX_DAT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.CODEC_SFX_IDX;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.FLAGS_BITS;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.OFFSETS;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.PACKED_BLOCK_SIZE;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.PAYLOADS;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.POSITIONS;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VECTORS_EXTENSION;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VECTORS_INDEX_EXTENSION;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VERSION_CURRENT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingTermVectorsWriter.VERSION_START;

CompressingTermVectorsReader::CompressingTermVectorsReader(
    shared_ptr<CompressingTermVectorsReader> reader)
    : fieldInfos(reader->fieldInfos), indexReader(reader->indexReader->clone()),
      vectorsStream(reader->vectorsStream->clone()), version(reader->version),
      packedIntsVersion(reader->packedIntsVersion),
      compressionMode(reader->compressionMode),
      decompressor(reader->decompressor->clone()), chunkSize(reader->chunkSize),
      numDocs(reader->numDocs),
      reader(make_shared<BlockPackedReaderIterator>(
          vectorsStream, packedIntsVersion, PACKED_BLOCK_SIZE, 0)),
      numChunks(reader->numChunks), numDirtyChunks(reader->numDirtyChunks),
      maxPointer(reader->maxPointer)
{
  this->closed = false;
}

CompressingTermVectorsReader::CompressingTermVectorsReader(
    shared_ptr<Directory> d, shared_ptr<SegmentInfo> si,
    const wstring &segmentSuffix, shared_ptr<FieldInfos> fn,
    shared_ptr<IOContext> context, const wstring &formatName,
    shared_ptr<CompressionMode> compressionMode) 
    : fieldInfos(fn), compressionMode(compressionMode), numDocs(si->maxDoc())
{
  const wstring segment = si->name;
  bool success = false;
  int version = -1;
  shared_ptr<CompressingStoredFieldsIndexReader> indexReader = nullptr;

  int64_t maxPointer = -1;

  // Load the index into memory
  const wstring indexName = IndexFileNames::segmentFileName(
      segment, segmentSuffix, VECTORS_INDEX_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = d.openChecksumInput(indexName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        d->openChecksumInput(indexName, context);
    runtime_error priorE = nullptr;
    try {
      const wstring codecNameIdx = formatName + CODEC_SFX_IDX;
      version = CodecUtil::checkIndexHeader(
          input, codecNameIdx, PackedInts::VERSION_START,
          PackedInts::VERSION_CURRENT, si->getId(), segmentSuffix);
      assert((CodecUtil::indexHeaderLength(codecNameIdx, segmentSuffix) ==
              input->getFilePointer()));
      indexReader = make_shared<CompressingStoredFieldsIndexReader>(input, si);
      maxPointer = input->readVLong(); // the end of the data section
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(input, priorE);
    }
  }

  this->version = version;
  this->indexReader = indexReader;
  this->maxPointer = maxPointer;

  try {
    // Open the data file and read metadata
    const wstring vectorsStreamFN = IndexFileNames::segmentFileName(
        segment, segmentSuffix, VECTORS_EXTENSION);
    vectorsStream = d->openInput(vectorsStreamFN, context);
    const wstring codecNameDat = formatName + CODEC_SFX_DAT;
    int version2 = CodecUtil::checkIndexHeader(
        vectorsStream, codecNameDat, PackedInts::VERSION_START,
        PackedInts::VERSION_CURRENT, si->getId(), segmentSuffix);
    if (version != version2) {
      throw make_shared<CorruptIndexException>(
          L"Version mismatch between stored fields index and data: " +
              to_wstring(version) + L" != " + to_wstring(version2),
          vectorsStream);
    }
    assert((CodecUtil::indexHeaderLength(codecNameDat, segmentSuffix) ==
            vectorsStream->getFilePointer()));

    int64_t pos = vectorsStream->getFilePointer();

    vectorsStream->seek(maxPointer);
    numChunks = vectorsStream->readVLong();
    numDirtyChunks = vectorsStream->readVLong();
    if (numDirtyChunks > numChunks) {
      throw make_shared<CorruptIndexException>(
          L"invalid chunk counts: dirty=" + to_wstring(numDirtyChunks) +
              L", total=" + to_wstring(numChunks),
          vectorsStream);
    }

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(vectorsStream);
    vectorsStream->seek(pos);

    packedIntsVersion = vectorsStream->readVInt();
    chunkSize = vectorsStream->readVInt();
    decompressor = compressionMode->newDecompressor();
    this->reader = make_shared<BlockPackedReaderIterator>(
        vectorsStream, packedIntsVersion, PACKED_BLOCK_SIZE, 0);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

shared_ptr<CompressionMode> CompressingTermVectorsReader::getCompressionMode()
{
  return compressionMode;
}

int CompressingTermVectorsReader::getChunkSize() { return chunkSize; }

int CompressingTermVectorsReader::getPackedIntsVersion()
{
  return packedIntsVersion;
}

int CompressingTermVectorsReader::getVersion() { return version; }

shared_ptr<CompressingStoredFieldsIndexReader>
CompressingTermVectorsReader::getIndexReader()
{
  return indexReader;
}

shared_ptr<IndexInput> CompressingTermVectorsReader::getVectorsStream()
{
  return vectorsStream;
}

int64_t CompressingTermVectorsReader::getMaxPointer() { return maxPointer; }

int64_t CompressingTermVectorsReader::getNumChunks() { return numChunks; }

int64_t CompressingTermVectorsReader::getNumDirtyChunks()
{
  return numDirtyChunks;
}

void CompressingTermVectorsReader::ensureOpen() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"this FieldsReader is closed");
  }
}

CompressingTermVectorsReader::~CompressingTermVectorsReader()
{
  if (!closed) {
    IOUtils::close({vectorsStream});
    closed = true;
  }
}

shared_ptr<TermVectorsReader> CompressingTermVectorsReader::clone()
{
  return make_shared<CompressingTermVectorsReader>(shared_from_this());
}

shared_ptr<Fields> CompressingTermVectorsReader::get(int doc) 
{
  ensureOpen();

  {
    // seek to the right place
    constexpr int64_t startPointer = indexReader->getStartPointer(doc);
    vectorsStream->seek(startPointer);
  }

  // decode
  // - docBase: first doc ID of the chunk
  // - chunkDocs: number of docs of the chunk
  constexpr int docBase = vectorsStream->readVInt();
  constexpr int chunkDocs = vectorsStream->readVInt();
  if (doc < docBase || doc >= docBase + chunkDocs ||
      docBase + chunkDocs > numDocs) {
    throw make_shared<CorruptIndexException>(
        L"docBase=" + to_wstring(docBase) + L",chunkDocs=" +
            to_wstring(chunkDocs) + L",doc=" + to_wstring(doc),
        vectorsStream);
  }

  constexpr int skip;      // number of fields to skip
  constexpr int numFields; // number of fields of the document we're looking for
  constexpr int
      totalFields; // total number of fields of the chunk (sum for all docs)
  if (chunkDocs == 1) {
    skip = 0;
    numFields = totalFields = vectorsStream->readVInt();
  } else {
    reader->reset(vectorsStream, chunkDocs);
    int sum = 0;
    for (int i = docBase; i < doc; ++i) {
      sum += reader->next();
    }
    skip = sum;
    numFields = static_cast<int>(reader->next());
    sum += numFields;
    for (int i = doc + 1; i < docBase + chunkDocs; ++i) {
      sum += reader->next();
    }
    totalFields = sum;
  }

  if (numFields == 0) {
    // no vectors
    return nullptr;
  }

  // read field numbers that have term vectors
  const std::deque<int> fieldNums;
  {
    constexpr int token = vectorsStream->readByte() & 0xFF;
    assert(token != 0); // means no term vectors, cannot happen since we checked
                        // for numFields == 0
    constexpr int bitsPerFieldNum = token & 0x1F;
    int totalDistinctFields =
        static_cast<int>(static_cast<unsigned int>(token) >> 5);
    if (totalDistinctFields == 0x07) {
      totalDistinctFields += vectorsStream->readVInt();
    }
    ++totalDistinctFields;
    shared_ptr<PackedInts::ReaderIterator> *const it =
        PackedInts::getReaderIteratorNoHeader(
            vectorsStream, PackedInts::Format::PACKED, packedIntsVersion,
            totalDistinctFields, bitsPerFieldNum, 1);
    fieldNums = std::deque<int>(totalDistinctFields);
    for (int i = 0; i < totalDistinctFields; ++i) {
      fieldNums[i] = static_cast<int>(it->next());
    }
  }

  // read field numbers and flags
  const std::deque<int> fieldNumOffs = std::deque<int>(numFields);
  shared_ptr<PackedInts::Reader> *const flags;
  {
    constexpr int bitsPerOff = PackedInts::bitsRequired(fieldNums.size() - 1);
    shared_ptr<PackedInts::Reader> *const allFieldNumOffs =
        PackedInts::getReaderNoHeader(vectorsStream, PackedInts::Format::PACKED,
                                      packedIntsVersion, totalFields,
                                      bitsPerOff);
    switch (vectorsStream->readVInt()) {
    case 0: {
      shared_ptr<PackedInts::Reader> *const fieldFlags =
          PackedInts::getReaderNoHeader(
              vectorsStream, PackedInts::Format::PACKED, packedIntsVersion,
              fieldNums.size(), FLAGS_BITS);
      shared_ptr<PackedInts::Mutable> f =
          PackedInts::getMutable(totalFields, FLAGS_BITS, PackedInts::COMPACT);
      for (int i = 0; i < totalFields; ++i) {
        constexpr int fieldNumOff = static_cast<int>(allFieldNumOffs->get(i));
        assert(fieldNumOff >= 0 && fieldNumOff < fieldNums.size());
        constexpr int fgs = static_cast<int>(fieldFlags->get(fieldNumOff));
        f->set(i, fgs);
      }
      flags = f;
      break;
    }
    case 1:
      flags = PackedInts::getReaderNoHeader(
          vectorsStream, PackedInts::Format::PACKED, packedIntsVersion,
          totalFields, FLAGS_BITS);
      break;
    default:
      throw make_shared<AssertionError>();
    }
    for (int i = 0; i < numFields; ++i) {
      fieldNumOffs[i] = static_cast<int>(allFieldNumOffs->get(skip + i));
    }
  }

  // number of terms per field for all fields
  shared_ptr<PackedInts::Reader> *const numTerms;
  constexpr int totalTerms;
  {
    constexpr int bitsRequired = vectorsStream->readVInt();
    numTerms = PackedInts::getReaderNoHeader(
        vectorsStream, PackedInts::Format::PACKED, packedIntsVersion,
        totalFields, bitsRequired);
    int sum = 0;
    for (int i = 0; i < totalFields; ++i) {
      sum += numTerms->get(i);
    }
    totalTerms = sum;
  }

  // term lengths
  int docOff = 0, docLen = 0, totalLen;
  const std::deque<int> fieldLengths = std::deque<int>(numFields);
  const std::deque<std::deque<int>> prefixLengths =
      std::deque<std::deque<int>>(numFields);
  const std::deque<std::deque<int>> suffixLengths =
      std::deque<std::deque<int>>(numFields);
  {
    reader->reset(vectorsStream, totalTerms);
    // skip
    int toSkip = 0;
    for (int i = 0; i < skip; ++i) {
      toSkip += numTerms->get(i);
    }
    reader->skip(toSkip);
    // read prefix lengths
    for (int i = 0; i < numFields; ++i) {
      constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
      const std::deque<int> fieldPrefixLengths = std::deque<int>(termCount);
      prefixLengths[i] = fieldPrefixLengths;
      for (int j = 0; j < termCount;) {
        shared_ptr<LongsRef> *const next = reader->next(termCount - j);
        for (int k = 0; k < next->length; ++k) {
          fieldPrefixLengths[j++] =
              static_cast<int>(next->longs[next->offset + k]);
        }
      }
    }
    reader->skip(totalTerms - reader->ord());

    reader->reset(vectorsStream, totalTerms);
    // skip
    toSkip = 0;
    for (int i = 0; i < skip; ++i) {
      for (int j = 0; j < numTerms->get(i); ++j) {
        docOff += reader->next();
      }
    }
    for (int i = 0; i < numFields; ++i) {
      constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
      const std::deque<int> fieldSuffixLengths = std::deque<int>(termCount);
      suffixLengths[i] = fieldSuffixLengths;
      for (int j = 0; j < termCount;) {
        shared_ptr<LongsRef> *const next = reader->next(termCount - j);
        for (int k = 0; k < next->length; ++k) {
          fieldSuffixLengths[j++] =
              static_cast<int>(next->longs[next->offset + k]);
        }
      }
      fieldLengths[i] = sum(suffixLengths[i]);
      docLen += fieldLengths[i];
    }
    totalLen = docOff + docLen;
    for (int i = skip + numFields; i < totalFields; ++i) {
      for (int j = 0; j < numTerms->get(i); ++j) {
        totalLen += reader->next();
      }
    }
  }

  // term freqs
  const std::deque<int> termFreqs = std::deque<int>(totalTerms);
  {
    reader->reset(vectorsStream, totalTerms);
    for (int i = 0; i < totalTerms;) {
      shared_ptr<LongsRef> *const next = reader->next(totalTerms - i);
      for (int k = 0; k < next->length; ++k) {
        termFreqs[i++] = 1 + static_cast<int>(next->longs[next->offset + k]);
      }
    }
  }

  // total number of positions, offsets and payloads
  int totalPositions = 0, totalOffsets = 0, totalPayloads = 0;
  for (int i = 0, termIndex = 0; i < totalFields; ++i) {
    constexpr int f = static_cast<int>(flags->get(i));
    constexpr int termCount = static_cast<int>(numTerms->get(i));
    for (int j = 0; j < termCount; ++j) {
      constexpr int freq = termFreqs[termIndex++];
      if ((f & PostingsEnum::POSITIONS) != 0) {
        totalPositions += freq;
      }
      if ((f & PostingsEnum::OFFSETS) != 0) {
        totalOffsets += freq;
      }
      if ((f & PostingsEnum::PAYLOADS) != 0) {
        totalPayloads += freq;
      }
    }
    assert((i != totalFields - 1 || termIndex == totalTerms,
            termIndex + L" " + to_wstring(totalTerms)));
  }

  const std::deque<std::deque<int>> positionIndex =
      this->positionIndex(skip, numFields, numTerms, termFreqs);
  const std::deque<std::deque<int>> positions, startOffsets, lengths;
  if (totalPositions > 0) {
    positions =
        readPositions(skip, numFields, flags, numTerms, termFreqs,
                      PostingsEnum::POSITIONS, totalPositions, positionIndex);
  } else {
    positions = std::deque<std::deque<int>>(numFields);
  }

  if (totalOffsets > 0) {
    // average number of chars per term
    const std::deque<float> charsPerTerm =
        std::deque<float>(fieldNums.size());
    for (int i = 0; i < charsPerTerm.size(); ++i) {
      charsPerTerm[i] = Float::intBitsToFloat(vectorsStream->readInt());
    }
    startOffsets =
        readPositions(skip, numFields, flags, numTerms, termFreqs,
                      PostingsEnum::OFFSETS, totalOffsets, positionIndex);
    lengths = readPositions(skip, numFields, flags, numTerms, termFreqs,
                            PostingsEnum::OFFSETS, totalOffsets, positionIndex);

    for (int i = 0; i < numFields; ++i) {
      const std::deque<int> fStartOffsets = startOffsets[i];
      const std::deque<int> fPositions = positions[i];
      // patch offsets from positions
      if (fStartOffsets.size() > 0 && fPositions.size() > 0) {
        constexpr float fieldCharsPerTerm = charsPerTerm[fieldNumOffs[i]];
        for (int j = 0; j < startOffsets[i].size(); ++j) {
          fStartOffsets[j] +=
              static_cast<int>(fieldCharsPerTerm * fPositions[j]);
        }
      }
      if (fStartOffsets.size() > 0) {
        const std::deque<int> fPrefixLengths = prefixLengths[i];
        const std::deque<int> fSuffixLengths = suffixLengths[i];
        const std::deque<int> fLengths = lengths[i];
        for (int j = 0, end = static_cast<int>(numTerms->get(skip + i));
             j < end; ++j) {
          // delta-decode start offsets and  patch lengths using term lengths
          constexpr int termLength = fPrefixLengths[j] + fSuffixLengths[j];
          lengths[i][positionIndex[i][j]] += termLength;
          for (int k = positionIndex[i][j] + 1; k < positionIndex[i][j + 1];
               ++k) {
            fStartOffsets[k] += fStartOffsets[k - 1];
            fLengths[k] += termLength;
          }
        }
      }
    }
  } else {
    startOffsets = lengths = std::deque<std::deque<int>>(numFields);
  }
  if (totalPositions > 0) {
    // delta-decode positions
    for (int i = 0; i < numFields; ++i) {
      const std::deque<int> fPositions = positions[i];
      const std::deque<int> fpositionIndex = positionIndex[i];
      if (fPositions.size() > 0) {
        for (int j = 0, end = static_cast<int>(numTerms->get(skip + i));
             j < end; ++j) {
          // delta-decode start offsets
          for (int k = fpositionIndex[j] + 1; k < fpositionIndex[j + 1]; ++k) {
            fPositions[k] += fPositions[k - 1];
          }
        }
      }
    }
  }

  // payload lengths
  const std::deque<std::deque<int>> payloadIndex =
      std::deque<std::deque<int>>(numFields);
  int totalPayloadLength = 0;
  int payloadOff = 0;
  int payloadLen = 0;
  if (totalPayloads > 0) {
    reader->reset(vectorsStream, totalPayloads);
    // skip
    int termIndex = 0;
    for (int i = 0; i < skip; ++i) {
      constexpr int f = static_cast<int>(flags->get(i));
      constexpr int termCount = static_cast<int>(numTerms->get(i));
      if ((f & PostingsEnum::PAYLOADS) != 0) {
        for (int j = 0; j < termCount; ++j) {
          constexpr int freq = termFreqs[termIndex + j];
          for (int k = 0; k < freq; ++k) {
            constexpr int l = static_cast<int>(reader->next());
            payloadOff += l;
          }
        }
      }
      termIndex += termCount;
    }
    totalPayloadLength = payloadOff;
    // read doc payload lengths
    for (int i = 0; i < numFields; ++i) {
      constexpr int f = static_cast<int>(flags->get(skip + i));
      constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
      if ((f & PostingsEnum::PAYLOADS) != 0) {
        constexpr int totalFreq = positionIndex[i][termCount];
        payloadIndex[i] = std::deque<int>(totalFreq + 1);
        int posIdx = 0;
        payloadIndex[i][posIdx] = payloadLen;
        for (int j = 0; j < termCount; ++j) {
          constexpr int freq = termFreqs[termIndex + j];
          for (int k = 0; k < freq; ++k) {
            constexpr int payloadLength = static_cast<int>(reader->next());
            payloadLen += payloadLength;
            payloadIndex[i][posIdx + 1] = payloadLen;
            ++posIdx;
          }
        }
        assert(posIdx == totalFreq);
      }
      termIndex += termCount;
    }
    totalPayloadLength += payloadLen;
    for (int i = skip + numFields; i < totalFields; ++i) {
      constexpr int f = static_cast<int>(flags->get(i));
      constexpr int termCount = static_cast<int>(numTerms->get(i));
      if ((f & PostingsEnum::PAYLOADS) != 0) {
        for (int j = 0; j < termCount; ++j) {
          constexpr int freq = termFreqs[termIndex + j];
          for (int k = 0; k < freq; ++k) {
            totalPayloadLength += reader->next();
          }
        }
      }
      termIndex += termCount;
    }
    assert((termIndex == totalTerms,
            to_wstring(termIndex) + L" " + to_wstring(totalTerms)));
  }

  // decompress data
  shared_ptr<BytesRef> *const suffixBytes = make_shared<BytesRef>();
  decompressor->decompress(vectorsStream, totalLen + totalPayloadLength,
                           docOff + payloadOff, docLen + payloadLen,
                           suffixBytes);
  suffixBytes->length = docLen;
  shared_ptr<BytesRef> *const payloadBytes = make_shared<BytesRef>(
      suffixBytes->bytes, suffixBytes->offset + docLen, payloadLen);

  const std::deque<int> fieldFlags = std::deque<int>(numFields);
  for (int i = 0; i < numFields; ++i) {
    fieldFlags[i] = static_cast<int>(flags->get(skip + i));
  }

  const std::deque<int> fieldNumTerms = std::deque<int>(numFields);
  for (int i = 0; i < numFields; ++i) {
    fieldNumTerms[i] = static_cast<int>(numTerms->get(skip + i));
  }

  const std::deque<std::deque<int>> fieldTermFreqs =
      std::deque<std::deque<int>>(numFields);
  {
    int termIdx = 0;
    for (int i = 0; i < skip; ++i) {
      termIdx += numTerms->get(i);
    }
    for (int i = 0; i < numFields; ++i) {
      constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
      fieldTermFreqs[i] = std::deque<int>(termCount);
      for (int j = 0; j < termCount; ++j) {
        fieldTermFreqs[i][j] = termFreqs[termIdx++];
      }
    }
  }

  assert((sum(fieldLengths) == docLen,
          to_wstring(sum(fieldLengths)) + L" != " + to_wstring(docLen)));

  return make_shared<TVFields>(shared_from_this(), fieldNums, fieldFlags,
                               fieldNumOffs, fieldNumTerms, fieldLengths,
                               prefixLengths, suffixLengths, fieldTermFreqs,
                               positionIndex, positions, startOffsets, lengths,
                               payloadBytes, payloadIndex, suffixBytes);
}

std::deque<std::deque<int>> CompressingTermVectorsReader::positionIndex(
    int skip, int numFields, shared_ptr<PackedInts::Reader> numTerms,
    std::deque<int> &termFreqs)
{
  const std::deque<std::deque<int>> positionIndex =
      std::deque<std::deque<int>>(numFields);
  int termIndex = 0;
  for (int i = 0; i < skip; ++i) {
    constexpr int termCount = static_cast<int>(numTerms->get(i));
    termIndex += termCount;
  }
  for (int i = 0; i < numFields; ++i) {
    constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
    positionIndex[i] = std::deque<int>(termCount + 1);
    for (int j = 0; j < termCount; ++j) {
      constexpr int freq = termFreqs[termIndex + j];
      positionIndex[i][j + 1] = positionIndex[i][j] + freq;
    }
    termIndex += termCount;
  }
  return positionIndex;
}

std::deque<std::deque<int>> CompressingTermVectorsReader::readPositions(
    int skip, int numFields, shared_ptr<PackedInts::Reader> flags,
    shared_ptr<PackedInts::Reader> numTerms, std::deque<int> &termFreqs,
    int flag, int const totalPositions,
    std::deque<std::deque<int>> &positionIndex) 
{
  const std::deque<std::deque<int>> positions =
      std::deque<std::deque<int>>(numFields);
  reader->reset(vectorsStream, totalPositions);
  // skip
  int toSkip = 0;
  int termIndex = 0;
  for (int i = 0; i < skip; ++i) {
    constexpr int f = static_cast<int>(flags->get(i));
    constexpr int termCount = static_cast<int>(numTerms->get(i));
    if ((f & flag) != 0) {
      for (int j = 0; j < termCount; ++j) {
        constexpr int freq = termFreqs[termIndex + j];
        toSkip += freq;
      }
    }
    termIndex += termCount;
  }
  reader->skip(toSkip);
  // read doc positions
  for (int i = 0; i < numFields; ++i) {
    constexpr int f = static_cast<int>(flags->get(skip + i));
    constexpr int termCount = static_cast<int>(numTerms->get(skip + i));
    if ((f & flag) != 0) {
      constexpr int totalFreq = positionIndex[i][termCount];
      const std::deque<int> fieldPositions = std::deque<int>(totalFreq);
      positions[i] = fieldPositions;
      for (int j = 0; j < totalFreq;) {
        shared_ptr<LongsRef> *const nextPositions = reader->next(totalFreq - j);
        for (int k = 0; k < nextPositions->length; ++k) {
          fieldPositions[j++] =
              static_cast<int>(nextPositions->longs[nextPositions->offset + k]);
        }
      }
    }
    termIndex += termCount;
  }
  reader->skip(totalPositions - reader->ord());
  return positions;
}

CompressingTermVectorsReader::TVFields::TVFields(
    shared_ptr<CompressingTermVectorsReader> outerInstance,
    std::deque<int> &fieldNums, std::deque<int> &fieldFlags,
    std::deque<int> &fieldNumOffs, std::deque<int> &numTerms,
    std::deque<int> &fieldLengths,
    std::deque<std::deque<int>> &prefixLengths,
    std::deque<std::deque<int>> &suffixLengths,
    std::deque<std::deque<int>> &termFreqs,
    std::deque<std::deque<int>> &positionIndex,
    std::deque<std::deque<int>> &positions,
    std::deque<std::deque<int>> &startOffsets,
    std::deque<std::deque<int>> &lengths, shared_ptr<BytesRef> payloadBytes,
    std::deque<std::deque<int>> &payloadIndex,
    shared_ptr<BytesRef> suffixBytes)
    : fieldNums(fieldNums), fieldFlags(fieldFlags), fieldNumOffs(fieldNumOffs),
      numTerms(numTerms), fieldLengths(fieldLengths),
      prefixLengths(prefixLengths), suffixLengths(suffixLengths),
      termFreqs(termFreqs), positionIndex(positionIndex), positions(positions),
      startOffsets(startOffsets), lengths(lengths), payloadIndex(payloadIndex),
      suffixBytes(suffixBytes), payloadBytes(payloadBytes),
      outerInstance(outerInstance)
{
}

shared_ptr<Iterator<wstring>> CompressingTermVectorsReader::TVFields::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

CompressingTermVectorsReader::TVFields::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(shared_ptr<TVFields> outerInstance)
{
  this->outerInstance = outerInstance;
  i = 0;
}

bool CompressingTermVectorsReader::TVFields::IteratorAnonymousInnerClass::
    hasNext()
{
  return i < outerInstance->fieldNumOffs.size();
}

wstring
CompressingTermVectorsReader::TVFields::IteratorAnonymousInnerClass::next()
{
  if (!hasNext()) {
    // C++ TODO: The following line could not be converted:
    throw java.util.NoSuchElementException();
  }
  constexpr int fieldNum =
      outerInstance->fieldNums[outerInstance->fieldNumOffs[i++]];
  return outerInstance->outerInstance->fieldInfos.fieldInfo(fieldNum).name;
}

void CompressingTermVectorsReader::TVFields::IteratorAnonymousInnerClass::
    remove()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Terms> CompressingTermVectorsReader::TVFields::terms(
    const wstring &field) 
{
  shared_ptr<FieldInfo> *const fieldInfo =
      outerInstance->fieldInfos->fieldInfo(field);
  if (fieldInfo == nullptr) {
    return nullptr;
  }
  int idx = -1;
  for (int i = 0; i < fieldNumOffs.size(); ++i) {
    if (fieldNums[fieldNumOffs[i]] == fieldInfo->number) {
      idx = i;
      break;
    }
  }

  if (idx == -1 || numTerms[idx] == 0) {
    // no term
    return nullptr;
  }
  int fieldOff = 0, fieldLen = -1;
  for (int i = 0; i < fieldNumOffs.size(); ++i) {
    if (i < idx) {
      fieldOff += fieldLengths[i];
    } else {
      fieldLen = fieldLengths[i];
      break;
    }
  }
  assert(fieldLen >= 0);
  return make_shared<TVTerms>(
      numTerms[idx], fieldFlags[idx], prefixLengths[idx], suffixLengths[idx],
      termFreqs[idx], positionIndex[idx], positions[idx], startOffsets[idx],
      lengths[idx], payloadIndex[idx], payloadBytes,
      make_shared<BytesRef>(suffixBytes->bytes, suffixBytes->offset + fieldOff,
                            fieldLen));
}

int CompressingTermVectorsReader::TVFields::size()
{
  return fieldNumOffs.size();
}

CompressingTermVectorsReader::TVTerms::TVTerms(
    int numTerms, int flags, std::deque<int> &prefixLengths,
    std::deque<int> &suffixLengths, std::deque<int> &termFreqs,
    std::deque<int> &positionIndex, std::deque<int> &positions,
    std::deque<int> &startOffsets, std::deque<int> &lengths,
    std::deque<int> &payloadIndex, shared_ptr<BytesRef> payloadBytes,
    shared_ptr<BytesRef> termBytes)
    : numTerms(numTerms), flags(flags), prefixLengths(prefixLengths),
      suffixLengths(suffixLengths), termFreqs(termFreqs),
      positionIndex(positionIndex), positions(positions),
      startOffsets(startOffsets), lengths(lengths), payloadIndex(payloadIndex),
      termBytes(termBytes), payloadBytes(payloadBytes)
{
}

shared_ptr<TermsEnum>
CompressingTermVectorsReader::TVTerms::iterator() 
{
  shared_ptr<TVTermsEnum> termsEnum = make_shared<TVTermsEnum>();
  termsEnum->reset(numTerms, flags, prefixLengths, suffixLengths, termFreqs,
                   positionIndex, positions, startOffsets, lengths,
                   payloadIndex, payloadBytes,
                   make_shared<ByteArrayDataInput>(
                       termBytes->bytes, termBytes->offset, termBytes->length));
  return termsEnum;
}

int64_t CompressingTermVectorsReader::TVTerms::size() 
{
  return numTerms;
}

int64_t
CompressingTermVectorsReader::TVTerms::getSumTotalTermFreq() 
{
  return -1LL;
}

int64_t
CompressingTermVectorsReader::TVTerms::getSumDocFreq() 
{
  return numTerms;
}

int CompressingTermVectorsReader::TVTerms::getDocCount() 
{
  return 1;
}

bool CompressingTermVectorsReader::TVTerms::hasFreqs() { return true; }

bool CompressingTermVectorsReader::TVTerms::hasOffsets()
{
  return (flags & PostingsEnum::OFFSETS) != 0;
}

bool CompressingTermVectorsReader::TVTerms::hasPositions()
{
  return (flags & PostingsEnum::POSITIONS) != 0;
}

bool CompressingTermVectorsReader::TVTerms::hasPayloads()
{
  return (flags & PostingsEnum::PAYLOADS) != 0;
}

CompressingTermVectorsReader::TVTermsEnum::TVTermsEnum()
    : term(make_shared<BytesRef>(16))
{
}

void CompressingTermVectorsReader::TVTermsEnum::reset(
    int numTerms, int flags, std::deque<int> &prefixLengths,
    std::deque<int> &suffixLengths, std::deque<int> &termFreqs,
    std::deque<int> &positionIndex, std::deque<int> &positions,
    std::deque<int> &startOffsets, std::deque<int> &lengths,
    std::deque<int> &payloadIndex, shared_ptr<BytesRef> payloads,
    shared_ptr<ByteArrayDataInput> in_)
{
  this->numTerms = numTerms;
  this->prefixLengths = prefixLengths;
  this->suffixLengths = suffixLengths;
  this->termFreqs = termFreqs;
  this->positionIndex = positionIndex;
  this->positions = positions;
  this->startOffsets = startOffsets;
  this->lengths = lengths;
  this->payloadIndex = payloadIndex;
  this->payloads = payloads;
  this->in_ = in_;
  startPos = in_->getPosition();
  reset();
}

void CompressingTermVectorsReader::TVTermsEnum::reset()
{
  term_->length = 0;
  in_->setPosition(startPos);
  ord_ = -1;
}

shared_ptr<BytesRef>
CompressingTermVectorsReader::TVTermsEnum::next() 
{
  if (ord_ == numTerms - 1) {
    return nullptr;
  } else {
    assert(ord_ < numTerms);
    ++ord_;
  }

  // read term
  term_->offset = 0;
  term_->length = prefixLengths[ord_] + suffixLengths[ord_];
  if (term_->length > term_->bytes.size()) {
    term_->bytes = ArrayUtil::grow(term_->bytes, term_->length);
  }
  in_->readBytes(term_->bytes, prefixLengths[ord_], suffixLengths[ord_]);

  return term_;
}

SeekStatus CompressingTermVectorsReader::TVTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{
  if (ord_ < numTerms && ord_ >= 0) {
    constexpr int cmp = term()->compareTo(text);
    if (cmp == 0) {
      return SeekStatus::FOUND;
    } else if (cmp > 0) {
      reset();
    }
  }
  // linear scan
  while (true) {
    shared_ptr<BytesRef> *const term = next();
    if (term == nullptr) {
      return SeekStatus::END;
    }
    constexpr int cmp = term->compareTo(text);
    if (cmp > 0) {
      return SeekStatus::NOT_FOUND;
    } else if (cmp == 0) {
      return SeekStatus::FOUND;
    }
  }
}

void CompressingTermVectorsReader::TVTermsEnum::seekExact(int64_t ord) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef>
CompressingTermVectorsReader::TVTermsEnum::term() 
{
  return term_;
}

int64_t CompressingTermVectorsReader::TVTermsEnum::ord() 
{
  throw make_shared<UnsupportedOperationException>();
}

int CompressingTermVectorsReader::TVTermsEnum::docFreq() 
{
  return 1;
}

int64_t
CompressingTermVectorsReader::TVTermsEnum::totalTermFreq() 
{
  return termFreqs[ord_];
}

shared_ptr<PostingsEnum> CompressingTermVectorsReader::TVTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  shared_ptr<TVPostingsEnum> *const docsEnum;
  if (reuse != nullptr &&
      std::dynamic_pointer_cast<TVPostingsEnum>(reuse) != nullptr) {
    docsEnum = std::static_pointer_cast<TVPostingsEnum>(reuse);
  } else {
    docsEnum = make_shared<TVPostingsEnum>();
  }

  docsEnum->reset(termFreqs[ord_], positionIndex[ord_], positions, startOffsets,
                  lengths, payloads, payloadIndex);
  return docsEnum;
}

CompressingTermVectorsReader::TVPostingsEnum::TVPostingsEnum()
    : payload(make_shared<BytesRef>())
{
}

void CompressingTermVectorsReader::TVPostingsEnum::reset(
    int freq, int positionIndex, std::deque<int> &positions,
    std::deque<int> &startOffsets, std::deque<int> &lengths,
    shared_ptr<BytesRef> payloads, std::deque<int> &payloadIndex)
{
  this->termFreq = freq;
  this->positionIndex = positionIndex;
  this->positions = positions;
  this->startOffsets = startOffsets;
  this->lengths = lengths;
  this->basePayloadOffset = payloads->offset;
  this->payload->bytes = payloads->bytes;
  payload->offset = payload->length = 0;
  this->payloadIndex = payloadIndex;

  doc = i = -1;
}

void CompressingTermVectorsReader::TVPostingsEnum::checkDoc()
{
  if (doc == NO_MORE_DOCS) {
    throw make_shared<IllegalStateException>(L"DocsEnum exhausted");
  } else if (doc == -1) {
    throw make_shared<IllegalStateException>(L"DocsEnum not started");
  }
}

void CompressingTermVectorsReader::TVPostingsEnum::checkPosition()
{
  checkDoc();
  if (i < 0) {
    throw make_shared<IllegalStateException>(L"Position enum not started");
  } else if (i >= termFreq) {
    throw make_shared<IllegalStateException>(L"Read past last position");
  }
}

int CompressingTermVectorsReader::TVPostingsEnum::nextPosition() throw(
    IOException)
{
  if (doc != 0) {
    throw make_shared<IllegalStateException>();
  } else if (i >= termFreq - 1) {
    throw make_shared<IllegalStateException>(L"Read past last position");
  }

  ++i;

  if (payloadIndex.size() > 0) {
    payload->offset = basePayloadOffset + payloadIndex[positionIndex + i];
    payload->length =
        payloadIndex[positionIndex + i + 1] - payloadIndex[positionIndex + i];
  }

  if (positions.empty()) {
    return -1;
  } else {
    return positions[positionIndex + i];
  }
}

int CompressingTermVectorsReader::TVPostingsEnum::startOffset() throw(
    IOException)
{
  checkPosition();
  if (startOffsets.empty()) {
    return -1;
  } else {
    return startOffsets[positionIndex + i];
  }
}

int CompressingTermVectorsReader::TVPostingsEnum::endOffset() 
{
  checkPosition();
  if (startOffsets.empty()) {
    return -1;
  } else {
    return startOffsets[positionIndex + i] + lengths[positionIndex + i];
  }
}

shared_ptr<BytesRef>
CompressingTermVectorsReader::TVPostingsEnum::getPayload() 
{
  checkPosition();
  if (payloadIndex.empty() || payload->length == 0) {
    return nullptr;
  } else {
    return payload;
  }
}

int CompressingTermVectorsReader::TVPostingsEnum::freq() 
{
  checkDoc();
  return termFreq;
}

int CompressingTermVectorsReader::TVPostingsEnum::docID() { return doc; }

int CompressingTermVectorsReader::TVPostingsEnum::nextDoc() 
{
  if (doc == -1) {
    return (doc = 0);
  } else {
    return (doc = NO_MORE_DOCS);
  }
}

int CompressingTermVectorsReader::TVPostingsEnum::advance(int target) throw(
    IOException)
{
  return slowAdvance(target);
}

int64_t CompressingTermVectorsReader::TVPostingsEnum::cost() { return 1; }

int CompressingTermVectorsReader::sum(std::deque<int> &arr)
{
  int sum = 0;
  for (auto el : arr) {
    sum += el;
  }
  return sum;
}

int64_t CompressingTermVectorsReader::ramBytesUsed()
{
  return indexReader->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CompressingTermVectorsReader::getChildResources()
{
  return Collections::singleton(
      Accountables::namedAccountable(L"term deque index", indexReader));
}

void CompressingTermVectorsReader::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(vectorsStream);
}

wstring CompressingTermVectorsReader::toString()
{
  return getClass().getSimpleName() + L"(mode=" + compressionMode +
         L",chunksize=" + to_wstring(chunkSize) + L")";
}
} // namespace org::apache::lucene::codecs::compressing