using namespace std;

#include "CompressingStoredFieldsWriter.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/IndexFileNames.h"
#include "../../index/IndexableField.h"
#include "../../index/MergeState.h"
#include "../../index/SegmentInfo.h"
#include "../../store/DataOutput.h"
#include "../../store/Directory.h"
#include "../../store/GrowableByteArrayDataOutput.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../../store/IndexOutput.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BitUtil.h"
#include "../../util/Bits.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../CodecUtil.h"
#include "../StoredFieldsReader.h"
#include "CompressingStoredFieldsIndexReader.h"
#include "CompressingStoredFieldsIndexWriter.h"
#include "CompressingStoredFieldsReader.h"
#include "CompressionMode.h"
#include "Compressor.h"
#include "MatchingReaders.h"

namespace org::apache::lucene::codecs::compressing
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using SerializedDocument = org::apache::lucene::codecs::compressing::
    CompressingStoredFieldsReader::SerializedDocument;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexableField = org::apache::lucene::index::IndexableField;
using MergeState = org::apache::lucene::index::MergeState;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using GrowableByteArrayDataOutput =
    org::apache::lucene::store::GrowableByteArrayDataOutput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BitUtil = org::apache::lucene::util::BitUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
const wstring CompressingStoredFieldsWriter::FIELDS_EXTENSION = L"fdt";
const wstring CompressingStoredFieldsWriter::FIELDS_INDEX_EXTENSION = L"fdx";
const wstring CompressingStoredFieldsWriter::CODEC_SFX_IDX = L"Index";
const wstring CompressingStoredFieldsWriter::CODEC_SFX_DAT = L"Data";

CompressingStoredFieldsWriter::CompressingStoredFieldsWriter(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    const wstring &segmentSuffix, shared_ptr<IOContext> context,
    const wstring &formatName, shared_ptr<CompressionMode> compressionMode,
    int chunkSize, int maxDocsPerChunk, int blockSize) 
    : segment(si->name), compressionMode(compressionMode), chunkSize(chunkSize),
      maxDocsPerChunk(maxDocsPerChunk),
      bufferedDocs(make_shared<GrowableByteArrayDataOutput>(chunkSize))
{
  assert(directory != nullptr);
  this->compressor = compressionMode->newCompressor();
  this->docBase = 0;
  this->numStoredFields = std::deque<int>(16);
  this->endOffsets = std::deque<int>(16);
  this->numBufferedDocs = 0;

  bool success = false;
  shared_ptr<IndexOutput> indexStream = directory->createOutput(
      IndexFileNames::segmentFileName(segment, segmentSuffix,
                                      FIELDS_INDEX_EXTENSION),
      context);
  try {
    fieldsStream =
        directory->createOutput(IndexFileNames::segmentFileName(
                                    segment, segmentSuffix, FIELDS_EXTENSION),
                                context);

    const wstring codecNameIdx = formatName + CODEC_SFX_IDX;
    const wstring codecNameDat = formatName + CODEC_SFX_DAT;
    CodecUtil::writeIndexHeader(indexStream, codecNameIdx, VERSION_CURRENT,
                                si->getId(), segmentSuffix);
    CodecUtil::writeIndexHeader(fieldsStream, codecNameDat, VERSION_CURRENT,
                                si->getId(), segmentSuffix);
    assert((CodecUtil::indexHeaderLength(codecNameDat, segmentSuffix) ==
            fieldsStream->getFilePointer()));
    assert((CodecUtil::indexHeaderLength(codecNameIdx, segmentSuffix) ==
            indexStream->getFilePointer()));

    indexWriter =
        make_shared<CompressingStoredFieldsIndexWriter>(indexStream, blockSize);
    indexStream.reset();

    fieldsStream->writeVInt(chunkSize);
    fieldsStream->writeVInt(PackedInts::VERSION_CURRENT);

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException(
          {fieldsStream, indexStream, indexWriter});
    }
  }
}

CompressingStoredFieldsWriter::~CompressingStoredFieldsWriter()
{
  try {
    IOUtils::close({fieldsStream, indexWriter, compressor});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    fieldsStream.reset();
    indexWriter.reset();
    compressor.reset();
  }
}

void CompressingStoredFieldsWriter::startDocument()  {}

void CompressingStoredFieldsWriter::finishDocument() 
{
  if (numBufferedDocs == this->numStoredFields.size()) {
    constexpr int newLength = ArrayUtil::oversize(numBufferedDocs + 1, 4);
    this->numStoredFields = Arrays::copyOf(this->numStoredFields, newLength);
    endOffsets = Arrays::copyOf(endOffsets, newLength);
  }
  this->numStoredFields[numBufferedDocs] = numStoredFieldsInDoc;
  numStoredFieldsInDoc = 0;
  endOffsets[numBufferedDocs] = bufferedDocs->getPosition();
  ++numBufferedDocs;
  if (triggerFlush()) {
    flush();
  }
}

void CompressingStoredFieldsWriter::saveInts(
    std::deque<int> &values, int length,
    shared_ptr<DataOutput> out) 
{
  assert(length > 0);
  if (length == 1) {
    out->writeVInt(values[0]);
  } else {
    bool allEqual = true;
    for (int i = 1; i < length; ++i) {
      if (values[i] != values[0]) {
        allEqual = false;
        break;
      }
    }
    if (allEqual) {
      out->writeVInt(0);
      out->writeVInt(values[0]);
    } else {
      int64_t max = 0;
      for (int i = 0; i < length; ++i) {
        max |= values[i];
      }
      constexpr int bitsRequired = PackedInts::bitsRequired(max);
      out->writeVInt(bitsRequired);
      shared_ptr<PackedInts::Writer> *const w = PackedInts::getWriterNoHeader(
          out, PackedInts::Format::PACKED, length, bitsRequired, 1);
      for (int i = 0; i < length; ++i) {
        w->add(values[i]);
      }
      w->finish();
    }
  }
}

void CompressingStoredFieldsWriter::writeHeader(
    int docBase, int numBufferedDocs, std::deque<int> &numStoredFields,
    std::deque<int> &lengths, bool sliced) 
{
  constexpr int slicedBit = sliced ? 1 : 0;

  // save docBase and numBufferedDocs
  fieldsStream->writeVInt(docBase);
  fieldsStream->writeVInt((numBufferedDocs) << 1 | slicedBit);

  // save numStoredFields
  saveInts(numStoredFields, numBufferedDocs, fieldsStream);

  // save lengths
  saveInts(lengths, numBufferedDocs, fieldsStream);
}

bool CompressingStoredFieldsWriter::triggerFlush()
{
  return bufferedDocs->getPosition() >= chunkSize ||
         numBufferedDocs >= maxDocsPerChunk;
}

void CompressingStoredFieldsWriter::flush() 
{
  indexWriter->writeIndex(numBufferedDocs, fieldsStream->getFilePointer());

  // transform end offsets into lengths
  const std::deque<int> lengths = endOffsets;
  for (int i = numBufferedDocs - 1; i > 0; --i) {
    lengths[i] = endOffsets[i] - endOffsets[i - 1];
    assert(lengths[i] >= 0);
  }
  constexpr bool sliced = bufferedDocs->getPosition() >= 2 * chunkSize;
  writeHeader(docBase, numBufferedDocs, numStoredFields, lengths, sliced);

  // compress stored fields to fieldsStream
  if (sliced) {
    // big chunk, slice it
    for (int compressed = 0; compressed < bufferedDocs->getPosition();
         compressed += chunkSize) {
      compressor->compress(
          bufferedDocs->getBytes(), compressed,
          min(chunkSize, bufferedDocs->getPosition() - compressed),
          fieldsStream);
    }
  } else {
    compressor->compress(bufferedDocs->getBytes(), 0,
                         bufferedDocs->getPosition(), fieldsStream);
  }

  // reset
  docBase += numBufferedDocs;
  numBufferedDocs = 0;
  bufferedDocs->reset();
  numChunks++;
}

void CompressingStoredFieldsWriter::writeField(
    shared_ptr<FieldInfo> info,
    shared_ptr<IndexableField> field) 
{

  ++numStoredFieldsInDoc;

  int bits = 0;
  shared_ptr<BytesRef> *const bytes;
  const wstring string;

  shared_ptr<Number> number = field->numericValue();
  if (number != nullptr) {
    if (dynamic_cast<optional<char>>(number) != nullptr ||
        dynamic_cast<optional<short>>(number) != nullptr ||
        dynamic_cast<optional<int>>(number) != nullptr) {
      bits = NUMERIC_INT;
    } else if (dynamic_cast<optional<int64_t>>(number) != nullptr) {
      bits = NUMERIC_LONG;
    } else if (dynamic_cast<optional<float>>(number) != nullptr) {
      bits = NUMERIC_FLOAT;
    } else if (dynamic_cast<optional<double>>(number) != nullptr) {
      bits = NUMERIC_DOUBLE;
    } else {
      throw invalid_argument(L"cannot store numeric type " +
                             number->getClass());
    }
    string = L"";
    bytes.reset();
  } else {
    bytes = field->binaryValue();
    if (bytes != nullptr) {
      bits = BYTE_ARR;
      string = L"";
    } else {
      bits = STRING;
      string = field->stringValue();
      if (string == L"") {
        throw invalid_argument(L"field " + field->name() +
                               L" is stored but does not have binaryValue, "
                               L"stringValue nor numericValue");
      }
    }
  }

  constexpr int64_t infoAndBits =
      ((static_cast<int64_t>(info->number)) << TYPE_BITS) | bits;
  bufferedDocs->writeVLong(infoAndBits);

  if (bytes != nullptr) {
    bufferedDocs->writeVInt(bytes->length);
    bufferedDocs->writeBytes(bytes->bytes, bytes->offset, bytes->length);
  } else if (string != L"") {
    bufferedDocs->writeString(string);
  } else {
    if (dynamic_cast<optional<char>>(number) != nullptr ||
        dynamic_cast<optional<short>>(number) != nullptr ||
        dynamic_cast<optional<int>>(number) != nullptr) {
      bufferedDocs->writeZInt(number->intValue());
    } else if (dynamic_cast<optional<int64_t>>(number) != nullptr) {
      writeTLong(bufferedDocs, number->longValue());
    } else if (dynamic_cast<optional<float>>(number) != nullptr) {
      writeZFloat(bufferedDocs, number->floatValue());
    } else if (dynamic_cast<optional<double>>(number) != nullptr) {
      writeZDouble(bufferedDocs, number->doubleValue());
    } else {
      throw make_shared<AssertionError>(L"Cannot get here");
    }
  }
}

void CompressingStoredFieldsWriter::writeZFloat(shared_ptr<DataOutput> out,
                                                float f) 
{
  int intVal = static_cast<int>(f);
  constexpr int floatBits = Float::floatToIntBits(f);

  if (f == intVal && intVal >= -1 && intVal <= 0x7D &&
      floatBits != NEGATIVE_ZERO_FLOAT) {
    // small integer value [-1..125]: single byte
    out->writeByte(static_cast<char>(0x80 | (1 + intVal)));
  } else if ((static_cast<int>(static_cast<unsigned int>(floatBits) >> 31)) ==
             0) {
    // other positive floats: 4 bytes
    out->writeInt(floatBits);
  } else {
    // other negative float: 5 bytes
    out->writeByte(static_cast<char>(0xFF));
    out->writeInt(floatBits);
  }
}

void CompressingStoredFieldsWriter::writeZDouble(shared_ptr<DataOutput> out,
                                                 double d) 
{
  int intVal = static_cast<int>(d);
  constexpr int64_t doubleBits = Double::doubleToLongBits(d);

  if (d == intVal && intVal >= -1 && intVal <= 0x7C &&
      doubleBits != NEGATIVE_ZERO_DOUBLE) {
    // small integer value [-1..124]: single byte
    out->writeByte(static_cast<char>(0x80 | (intVal + 1)));
    return;
  } else if (d == static_cast<float>(d)) {
    // d has an accurate float representation: 5 bytes
    out->writeByte(static_cast<char>(0xFE));
    out->writeInt(Float::floatToIntBits(static_cast<float>(d)));
  } else if ((static_cast<int64_t>(
                 static_cast<uint64_t>(doubleBits) >> 63)) == 0) {
    // other positive doubles: 8 bytes
    out->writeLong(doubleBits);
  } else {
    // other negative doubles: 9 bytes
    out->writeByte(static_cast<char>(0xFF));
    out->writeLong(doubleBits);
  }
}

void CompressingStoredFieldsWriter::writeTLong(shared_ptr<DataOutput> out,
                                               int64_t l) 
{
  int header;
  if (l % SECOND != 0) {
    header = 0;
  } else if (l % DAY == 0) {
    // timestamp with day precision
    header = DAY_ENCODING;
    l /= DAY;
  } else if (l % HOUR == 0) {
    // timestamp with hour precision, or day precision with a timezone
    header = HOUR_ENCODING;
    l /= HOUR;
  } else {
    // timestamp with second precision
    header = SECOND_ENCODING;
    l /= SECOND;
  }

  constexpr int64_t zigZagL = BitUtil::zigZagEncode(l);
  header |= (zigZagL & 0x1F); // last 5 bits
  constexpr int64_t upperBits =
      static_cast<int64_t>(static_cast<uint64_t>(zigZagL) >> 5);
  if (upperBits != 0) {
    header |= 0x20;
  }
  out->writeByte(static_cast<char>(header));
  if (upperBits != 0) {
    out->writeVLong(upperBits);
  }
}

void CompressingStoredFieldsWriter::finish(shared_ptr<FieldInfos> fis,
                                           int numDocs) 
{
  if (numBufferedDocs > 0) {
    flush();
    numDirtyChunks++; // incomplete: we had to force this flush
  } else {
    assert(bufferedDocs->getPosition() == 0);
  }
  if (docBase != numDocs) {
    throw runtime_error(L"Wrote " + to_wstring(docBase) +
                        L" docs, finish called with numDocs=" +
                        to_wstring(numDocs));
  }
  indexWriter->finish(numDocs, fieldsStream->getFilePointer());
  fieldsStream->writeVLong(numChunks);
  fieldsStream->writeVLong(numDirtyChunks);
  CodecUtil::writeFooter(fieldsStream);
  assert(bufferedDocs->getPosition() == 0);
}

const wstring CompressingStoredFieldsWriter::BULK_MERGE_ENABLED_SYSPROP =
    CompressingStoredFieldsWriter::typeid->getName() + L".enableBulkMerge";

CompressingStoredFieldsWriter::StaticConstructor::StaticConstructor()
{
  bool v = true;
  try {
    v = static_cast<Boolean>(
        System::getProperty(BULK_MERGE_ENABLED_SYSPROP, L"true"));
  } catch (const SecurityException &ignored) {
  }
  BULK_MERGE_ENABLED = v;
}

CompressingStoredFieldsWriter::StaticConstructor
    CompressingStoredFieldsWriter::staticConstructor;

int CompressingStoredFieldsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  int docCount = 0;
  int numReaders = mergeState->maxDocs.size();

  shared_ptr<MatchingReaders> matching =
      make_shared<MatchingReaders>(mergeState);
  if (mergeState->needsIndexSort) {
    /**
     * If all readers are compressed and they have the same fieldinfos then we
     * can merge the serialized document directly.
     */
    deque<std::shared_ptr<CompressingStoredFieldsMergeSub>> subs =
        deque<std::shared_ptr<CompressingStoredFieldsMergeSub>>();
    for (int i = 0; i < mergeState->storedFieldsReaders.size(); i++) {
      if (matching->matchingReaders[i] &&
          std::dynamic_pointer_cast<CompressingStoredFieldsReader>(
              mergeState->storedFieldsReaders[i]) != nullptr) {
        shared_ptr<CompressingStoredFieldsReader> storedFieldsReader =
            std::static_pointer_cast<CompressingStoredFieldsReader>(
                mergeState->storedFieldsReaders[i]);
        storedFieldsReader->checkIntegrity();
        subs.push_back(make_shared<CompressingStoredFieldsMergeSub>(
            storedFieldsReader, mergeState->docMaps[i],
            mergeState->maxDocs[i]));
      } else {
        return StoredFieldsWriter::merge(mergeState);
      }
    }

    shared_ptr<DocIDMerger<std::shared_ptr<CompressingStoredFieldsMergeSub>>>
        *const docIDMerger = DocIDMerger::of(subs, true);
    while (true) {
      shared_ptr<CompressingStoredFieldsMergeSub> sub = docIDMerger->next();
      if (sub == nullptr) {
        break;
      }
      assert(sub->mappedDocID == docCount);
      shared_ptr<SerializedDocument> doc = sub->reader.document(sub->docID);
      startDocument();
      bufferedDocs->copyBytes(doc->in_, doc->length);
      numStoredFieldsInDoc = doc->numStoredFields;
      finishDocument();
      ++docCount;
    }
    finish(mergeState->mergeFieldInfos, docCount);
    return docCount;
  }

  for (int readerIndex = 0; readerIndex < numReaders; readerIndex++) {
    shared_ptr<StoredFieldsWriter::MergeVisitor> visitor =
        make_shared<StoredFieldsWriter::MergeVisitor>(shared_from_this(),
                                                      mergeState, readerIndex);
    shared_ptr<CompressingStoredFieldsReader> matchingFieldsReader = nullptr;
    if (matching->matchingReaders[readerIndex]) {
      shared_ptr<StoredFieldsReader> *const fieldsReader =
          mergeState->storedFieldsReaders[readerIndex];
      // we can only bulk-copy if the matching reader is also a
      // CompressingStoredFieldsReader
      if (fieldsReader != nullptr &&
          std::dynamic_pointer_cast<CompressingStoredFieldsReader>(
              fieldsReader) != nullptr) {
        matchingFieldsReader =
            std::static_pointer_cast<CompressingStoredFieldsReader>(
                fieldsReader);
      }
    }

    constexpr int maxDoc = mergeState->maxDocs[readerIndex];
    shared_ptr<Bits> *const liveDocs = mergeState->liveDocs[readerIndex];

    // if its some other format, or an older version of this format, or safety
    // switch:
    if (matchingFieldsReader == nullptr ||
        matchingFieldsReader->getVersion() != VERSION_CURRENT ||
        BULK_MERGE_ENABLED == false) {
      // naive merge...
      shared_ptr<StoredFieldsReader> storedFieldsReader =
          mergeState->storedFieldsReaders[readerIndex];
      if (storedFieldsReader != nullptr) {
        storedFieldsReader->checkIntegrity();
      }
      for (int docID = 0; docID < maxDoc; docID++) {
        if (liveDocs != nullptr && liveDocs->get(docID) == false) {
          continue;
        }
        startDocument();
        storedFieldsReader->visitDocument(docID, visitor);
        finishDocument();
        ++docCount;
      }
    } else if (matchingFieldsReader->getCompressionMode() == compressionMode &&
               matchingFieldsReader->getChunkSize() == chunkSize &&
               matchingFieldsReader->getPackedIntsVersion() ==
                   PackedInts::VERSION_CURRENT &&
               liveDocs == nullptr && !tooDirty(matchingFieldsReader)) {
      // optimized merge, raw byte copy
      // its not worth fine-graining this if there are deletions.

      // if the format is older, its always handled by the naive merge case
      // above
      assert(matchingFieldsReader->getVersion() == VERSION_CURRENT);
      matchingFieldsReader->checkIntegrity();

      // flush any pending chunks
      if (numBufferedDocs > 0) {
        flush();
        numDirtyChunks++; // incomplete: we had to force this flush
      }

      // iterate over each chunk. we use the stored fields index to find chunk
      // boundaries, read the docstart + doccount from the chunk header (we
      // write a new header, since doc numbers will change), and just copy the
      // bytes directly.
      shared_ptr<IndexInput> rawDocs = matchingFieldsReader->getFieldsStream();
      shared_ptr<CompressingStoredFieldsIndexReader> index =
          matchingFieldsReader->getIndexReader();
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
        int code = rawDocs->readVInt();

        // write a new index entry and new header for this chunk.
        int bufferedDocs =
            static_cast<int>(static_cast<unsigned int>(code) >> 1);
        indexWriter->writeIndex(bufferedDocs, fieldsStream->getFilePointer());
        fieldsStream->writeVInt(docBase); // rebase
        fieldsStream->writeVInt(code);
        docID += bufferedDocs;
        docBase += bufferedDocs;
        docCount += bufferedDocs;

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
          end = matchingFieldsReader->getMaxPointer();
        } else {
          end = index->getStartPointer(docID);
        }
        fieldsStream->copyBytes(rawDocs, end - rawDocs->getFilePointer());
      }

      if (rawDocs->getFilePointer() != matchingFieldsReader->getMaxPointer()) {
        throw make_shared<CorruptIndexException>(
            L"invalid state: pos=" + to_wstring(rawDocs->getFilePointer()) +
                L", max=" + to_wstring(matchingFieldsReader->getMaxPointer()),
            rawDocs);
      }

      // since we bulk merged all chunks, we inherit any dirty ones from this
      // segment.
      numChunks += matchingFieldsReader->getNumChunks();
      numDirtyChunks += matchingFieldsReader->getNumDirtyChunks();
    } else {
      // optimized merge, we copy serialized (but decompressed) bytes directly
      // even on simple docs (1 stored field), it seems to help by about 20%

      // if the format is older, its always handled by the naive merge case
      // above
      assert(matchingFieldsReader->getVersion() == VERSION_CURRENT);
      matchingFieldsReader->checkIntegrity();

      for (int docID = 0; docID < maxDoc; docID++) {
        if (liveDocs != nullptr && liveDocs->get(docID) == false) {
          continue;
        }
        shared_ptr<SerializedDocument> doc =
            matchingFieldsReader->document(docID);
        startDocument();
        bufferedDocs->copyBytes(doc->in_, doc->length);
        numStoredFieldsInDoc = doc->numStoredFields;
        finishDocument();
        ++docCount;
      }
    }
  }
  finish(mergeState->mergeFieldInfos, docCount);
  return docCount;
}

bool CompressingStoredFieldsWriter::tooDirty(
    shared_ptr<CompressingStoredFieldsReader> candidate)
{
  // more than 1% dirty, or more than hard limit of 1024 dirty chunks
  return candidate->getNumDirtyChunks() > 1024 ||
         candidate->getNumDirtyChunks() * 100 > candidate->getNumChunks();
}

CompressingStoredFieldsWriter::CompressingStoredFieldsMergeSub::
    CompressingStoredFieldsMergeSub(
        shared_ptr<CompressingStoredFieldsReader> reader,
        shared_ptr<MergeState::DocMap> docMap, int maxDoc)
    : org::apache::lucene::index::DocIDMerger::Sub(docMap), reader(reader),
      maxDoc(maxDoc)
{
}

int CompressingStoredFieldsWriter::CompressingStoredFieldsMergeSub::nextDoc()
{
  docID++;
  if (docID == maxDoc) {
    return NO_MORE_DOCS;
  } else {
    return docID;
  }
}
} // namespace org::apache::lucene::codecs::compressing