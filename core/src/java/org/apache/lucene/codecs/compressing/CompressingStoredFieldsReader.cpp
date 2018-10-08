using namespace std;

#include "CompressingStoredFieldsReader.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/FieldInfo.h"
#include "../../index/FieldInfos.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../index/StoredFieldVisitor.h"
#include "../../store/AlreadyClosedException.h"
#include "../../store/ByteArrayDataInput.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountable.h"
#include "../../util/Accountables.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BitUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "../../util/packed/PackedInts.h"
#include "../CodecUtil.h"
#include "CompressingStoredFieldsIndexReader.h"
#include "CompressionMode.h"
#include "Decompressor.h"

namespace org::apache::lucene::codecs::compressing
{
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.BYTE_ARR;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.CODEC_SFX_DAT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.CODEC_SFX_IDX;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.DAY;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.DAY_ENCODING;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.FIELDS_EXTENSION;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.FIELDS_INDEX_EXTENSION;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.HOUR;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.HOUR_ENCODING;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_DOUBLE;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_FLOAT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_INT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.NUMERIC_LONG;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.SECOND;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.SECOND_ENCODING;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.STRING;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.TYPE_BITS;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.TYPE_MASK;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.VERSION_CURRENT;
//    import static
//    org.apache.lucene.codecs.compressing.CompressingStoredFieldsWriter.VERSION_START;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using Document = org::apache::lucene::document::Document;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BitUtil = org::apache::lucene::util::BitUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

CompressingStoredFieldsReader::CompressingStoredFieldsReader(
    shared_ptr<CompressingStoredFieldsReader> reader, bool merging)
    : version(reader->version), fieldInfos(reader->fieldInfos),
      indexReader(reader->indexReader->clone()), maxPointer(reader->maxPointer),
      fieldsStream(reader->fieldsStream->clone()), chunkSize(reader->chunkSize),
      packedIntsVersion(reader->packedIntsVersion),
      compressionMode(reader->compressionMode),
      decompressor(reader->decompressor->clone()), numDocs(reader->numDocs),
      merging(merging), state(make_shared<BlockState>(shared_from_this())),
      numChunks(reader->numChunks), numDirtyChunks(reader->numDirtyChunks)
{
  this->closed = false;
}

CompressingStoredFieldsReader::CompressingStoredFieldsReader(
    shared_ptr<Directory> d, shared_ptr<SegmentInfo> si,
    const wstring &segmentSuffix, shared_ptr<FieldInfos> fn,
    shared_ptr<IOContext> context, const wstring &formatName,
    shared_ptr<CompressionMode> compressionMode) 
    : fieldInfos(fn), compressionMode(compressionMode), numDocs(si->maxDoc())
{
  const wstring segment = si->name;
  bool success = false;

  int version = -1;
  int64_t maxPointer = -1;
  shared_ptr<CompressingStoredFieldsIndexReader> indexReader = nullptr;

  // Load the index into memory
  const wstring indexName = IndexFileNames::segmentFileName(
      segment, segmentSuffix, FIELDS_INDEX_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // indexStream = d.openChecksumInput(indexName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput indexStream =
        d->openChecksumInput(indexName, context);
    runtime_error priorE = nullptr;
    try {
      const wstring codecNameIdx = formatName + CODEC_SFX_IDX;
      version = CodecUtil::checkIndexHeader(
          indexStream, codecNameIdx, PackedInts::VERSION_START,
          PackedInts::VERSION_CURRENT, si->getId(), segmentSuffix);
      assert((CodecUtil::indexHeaderLength(codecNameIdx, segmentSuffix) ==
              indexStream->getFilePointer()));
      indexReader =
          make_shared<CompressingStoredFieldsIndexReader>(indexStream, si);
      maxPointer = indexStream->readVLong();
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(indexStream, priorE);
    }
  }

  this->version = version;
  this->maxPointer = maxPointer;
  this->indexReader = indexReader;

  const wstring fieldsStreamFN =
      IndexFileNames::segmentFileName(segment, segmentSuffix, FIELDS_EXTENSION);
  try {
    // Open the data file and read metadata
    fieldsStream = d->openInput(fieldsStreamFN, context);
    const wstring codecNameDat = formatName + CODEC_SFX_DAT;
    constexpr int fieldsVersion = CodecUtil::checkIndexHeader(
        fieldsStream, codecNameDat, PackedInts::VERSION_START,
        PackedInts::VERSION_CURRENT, si->getId(), segmentSuffix);
    if (version != fieldsVersion) {
      throw make_shared<CorruptIndexException>(
          L"Version mismatch between stored fields index and data: " +
              to_wstring(version) + L" != " + to_wstring(fieldsVersion),
          fieldsStream);
    }
    assert((CodecUtil::indexHeaderLength(codecNameDat, segmentSuffix) ==
            fieldsStream->getFilePointer()));

    chunkSize = fieldsStream->readVInt();
    packedIntsVersion = fieldsStream->readVInt();
    decompressor = compressionMode->newDecompressor();
    this->merging = false;
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    this->state = make_shared<BlockState>(shared_from_this());

    fieldsStream->seek(maxPointer);
    numChunks = fieldsStream->readVLong();
    numDirtyChunks = fieldsStream->readVLong();
    if (numDirtyChunks > numChunks) {
      throw make_shared<CorruptIndexException>(
          L"invalid chunk counts: dirty=" + to_wstring(numDirtyChunks) +
              L", total=" + to_wstring(numChunks),
          fieldsStream);
    }

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(fieldsStream);

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

void CompressingStoredFieldsReader::ensureOpen() 
{
  if (closed) {
    throw make_shared<AlreadyClosedException>(L"this FieldsReader is closed");
  }
}

CompressingStoredFieldsReader::~CompressingStoredFieldsReader()
{
  if (!closed) {
    IOUtils::close({fieldsStream});
    closed = true;
  }
}

void CompressingStoredFieldsReader::readField(
    shared_ptr<DataInput> in_, shared_ptr<StoredFieldVisitor> visitor,
    shared_ptr<FieldInfo> info, int bits) 
{
  switch (bits & TYPE_MASK) {
  case BYTE_ARR: {
    int length = in_->readVInt();
    std::deque<char> data(length);
    in_->readBytes(data, 0, length);
    visitor->binaryField(info, data);
    break;
  }
  case STRING:
    length = in_->readVInt();
    data = std::deque<char>(length);
    in_->readBytes(data, 0, length);
    visitor->stringField(info, data);
    break;
  case NUMERIC_INT:
    visitor->intField(info, in_->readZInt());
    break;
  case NUMERIC_FLOAT:
    visitor->floatField(info, readZFloat(in_));
    break;
  case NUMERIC_LONG:
    visitor->longField(info, readTLong(in_));
    break;
  case NUMERIC_DOUBLE:
    visitor->doubleField(info, readZDouble(in_));
    break;
  default:
    throw make_shared<AssertionError>(L"Unknown type flag: " +
                                      Integer::toHexString(bits));
  }
}

void CompressingStoredFieldsReader::skipField(shared_ptr<DataInput> in_,
                                              int bits) 
{
  switch (bits & TYPE_MASK) {
  case BYTE_ARR:
  case STRING: {
    constexpr int length = in_->readVInt();
    in_->skipBytes(length);
    break;
  }
  case NUMERIC_INT:
    in_->readZInt();
    break;
  case NUMERIC_FLOAT:
    readZFloat(in_);
    break;
  case NUMERIC_LONG:
    readTLong(in_);
    break;
  case NUMERIC_DOUBLE:
    readZDouble(in_);
    break;
  default:
    throw make_shared<AssertionError>(L"Unknown type flag: " +
                                      Integer::toHexString(bits));
  }
}

float CompressingStoredFieldsReader::readZFloat(
    shared_ptr<DataInput> in_) 
{
  int b = in_->readByte() & 0xFF;
  if (b == 0xFF) {
    // negative value
    return Float::intBitsToFloat(in_->readInt());
  } else if ((b & 0x80) != 0) {
    // small integer [-1..125]
    return (b & 0x7f) - 1;
  } else {
    // positive float
    int bits =
        b << 24 | ((in_->readShort() & 0xFFFF) << 8) | (in_->readByte() & 0xFF);
    return Float::intBitsToFloat(bits);
  }
}

double CompressingStoredFieldsReader::readZDouble(
    shared_ptr<DataInput> in_) 
{
  int b = in_->readByte() & 0xFF;
  if (b == 0xFF) {
    // negative value
    return Double::longBitsToDouble(in_->readLong());
  } else if (b == 0xFE) {
    // float
    return Float::intBitsToFloat(in_->readInt());
  } else if ((b & 0x80) != 0) {
    // small integer [-1..124]
    return (b & 0x7f) - 1;
  } else {
    // positive double
    int64_t bits = (static_cast<int64_t>(b)) << 56 |
                     ((in_->readInt() & 0xFFFFFFFFLL) << 24) |
                     ((in_->readShort() & 0xFFFFLL) << 8) |
                     (in_->readByte() & 0xFFLL);
    return Double::longBitsToDouble(bits);
  }
}

int64_t CompressingStoredFieldsReader::readTLong(
    shared_ptr<DataInput> in_) 
{
  int header = in_->readByte() & 0xFF;

  int64_t bits = header & 0x1F;
  if ((header & 0x20) != 0) {
    // continuation bit
    bits |= in_->readVLong() << 5;
  }

  int64_t l = BitUtil::zigZagDecode(bits);

  switch (header & DAY_ENCODING) {
  case SECOND_ENCODING:
    l *= SECOND;
    break;
  case HOUR_ENCODING:
    l *= HOUR;
    break;
  case DAY_ENCODING:
    l *= DAY;
    break;
  case 0:
    // uncompressed
    break;
  default:
    throw make_shared<AssertionError>();
  }

  return l;
}

CompressingStoredFieldsReader::SerializedDocument::SerializedDocument(
    shared_ptr<DataInput> in_, int length, int numStoredFields)
    : in_(in_), length(length), numStoredFields(numStoredFields)
{
}

CompressingStoredFieldsReader::BlockState::BlockState(
    shared_ptr<CompressingStoredFieldsReader> outerInstance)
    : outerInstance(outerInstance)
{
}

bool CompressingStoredFieldsReader::BlockState::contains(int docID)
{
  return docID >= docBase && docID < docBase + chunkDocs;
}

void CompressingStoredFieldsReader::BlockState::reset(int docID) throw(
    IOException)
{
  bool success = false;
  try {
    doReset(docID);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false) {
      // if the read failed, set chunkDocs to 0 so that it does not
      // contain any docs anymore and is not reused. This should help
      // get consistent exceptions when trying to get several
      // documents which are in the same corrupted block since it will
      // force the header to be decoded again
      chunkDocs = 0;
    }
  }
}

void CompressingStoredFieldsReader::BlockState::doReset(int docID) throw(
    IOException)
{
  docBase = outerInstance->fieldsStream->readVInt();
  constexpr int token = outerInstance->fieldsStream->readVInt();
  chunkDocs = static_cast<int>(static_cast<unsigned int>(token) >> 1);
  if (contains(docID) == false ||
      docBase + chunkDocs > outerInstance->numDocs) {
    throw make_shared<CorruptIndexException>(
        L"Corrupted: docID=" + to_wstring(docID) + L", docBase=" +
            to_wstring(docBase) + L", chunkDocs=" + to_wstring(chunkDocs) +
            L", numDocs=" + to_wstring(outerInstance->numDocs),
        outerInstance->fieldsStream);
  }

  sliced = (token & 1) != 0;

  offsets = ArrayUtil::grow(offsets, chunkDocs + 1);
  numStoredFields = ArrayUtil::grow(numStoredFields, chunkDocs);

  if (chunkDocs == 1) {
    numStoredFields[0] = outerInstance->fieldsStream->readVInt();
    offsets[1] = outerInstance->fieldsStream->readVInt();
  } else {
    // Number of stored fields per document
    constexpr int bitsPerStoredFields = outerInstance->fieldsStream->readVInt();
    if (bitsPerStoredFields == 0) {
      Arrays::fill(numStoredFields, 0, chunkDocs,
                   outerInstance->fieldsStream->readVInt());
    } else if (bitsPerStoredFields > 31) {
      throw make_shared<CorruptIndexException>(
          L"bitsPerStoredFields=" + to_wstring(bitsPerStoredFields),
          outerInstance->fieldsStream);
    } else {
      shared_ptr<PackedInts::ReaderIterator> *const it =
          PackedInts::getReaderIteratorNoHeader(
              outerInstance->fieldsStream, PackedInts::Format::PACKED,
              outerInstance->packedIntsVersion, chunkDocs, bitsPerStoredFields,
              1);
      for (int i = 0; i < chunkDocs; ++i) {
        numStoredFields[i] = static_cast<int>(it->next());
      }
    }

    // The stream encodes the length of each document and we decode
    // it into a deque of monotonically increasing offsets
    constexpr int bitsPerLength = outerInstance->fieldsStream->readVInt();
    if (bitsPerLength == 0) {
      constexpr int length = outerInstance->fieldsStream->readVInt();
      for (int i = 0; i < chunkDocs; ++i) {
        offsets[1 + i] = (1 + i) * length;
      }
    } else if (bitsPerStoredFields > 31) {
      throw make_shared<CorruptIndexException>(L"bitsPerLength=" +
                                                   to_wstring(bitsPerLength),
                                               outerInstance->fieldsStream);
    } else {
      shared_ptr<PackedInts::ReaderIterator> *const it =
          PackedInts::getReaderIteratorNoHeader(
              outerInstance->fieldsStream, PackedInts::Format::PACKED,
              outerInstance->packedIntsVersion, chunkDocs, bitsPerLength, 1);
      for (int i = 0; i < chunkDocs; ++i) {
        offsets[i + 1] = static_cast<int>(it->next());
      }
      for (int i = 0; i < chunkDocs; ++i) {
        offsets[i + 1] += offsets[i];
      }
    }

    // Additional validation: only the empty document has a serialized length of
    // 0
    for (int i = 0; i < chunkDocs; ++i) {
      constexpr int len = offsets[i + 1] - offsets[i];
      constexpr int storedFields = numStoredFields[i];
      if ((len == 0) != (storedFields == 0)) {
        throw make_shared<CorruptIndexException>(L"length=" + to_wstring(len) +
                                                     L", numStoredFields=" +
                                                     to_wstring(storedFields),
                                                 outerInstance->fieldsStream);
      }
    }
  }

  startPointer = outerInstance->fieldsStream->getFilePointer();

  if (outerInstance->merging) {
    constexpr int totalLength = offsets[chunkDocs];
    // decompress eagerly
    if (sliced) {
      bytes->offset = bytes->length = 0;
      for (int decompressed = 0; decompressed < totalLength;) {
        constexpr int toDecompress =
            min(totalLength - decompressed, outerInstance->chunkSize);
        outerInstance->decompressor->decompress(
            outerInstance->fieldsStream, toDecompress, 0, toDecompress, spare);
        bytes->bytes =
            ArrayUtil::grow(bytes->bytes, bytes->length + spare->length);
        System::arraycopy(spare->bytes, spare->offset, bytes->bytes,
                          bytes->length, spare->length);
        bytes->length += spare->length;
        decompressed += toDecompress;
      }
    } else {
      outerInstance->decompressor->decompress(
          outerInstance->fieldsStream, totalLength, 0, totalLength, bytes);
    }
    if (bytes->length != totalLength) {
      throw make_shared<CorruptIndexException>(
          L"Corrupted: expected chunk size = " + to_wstring(totalLength) +
              L", got " + to_wstring(bytes->length),
          outerInstance->fieldsStream);
    }
  }
}

shared_ptr<SerializedDocument>
CompressingStoredFieldsReader::BlockState::document(int docID) throw(
    IOException)
{
  if (contains(docID) == false) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }

  constexpr int index = docID - docBase;
  constexpr int offset = offsets[index];
  constexpr int length = offsets[index + 1] - offset;
  constexpr int totalLength = offsets[chunkDocs];
  constexpr int numStoredFields = this->numStoredFields[index];

  shared_ptr<DataInput> *const documentInput;
  if (length == 0) {
    // empty
    documentInput = make_shared<ByteArrayDataInput>();
  } else if (outerInstance->merging) {
    // already decompressed
    documentInput = make_shared<ByteArrayDataInput>(
        bytes->bytes, bytes->offset + offset, length);
  } else if (sliced) {
    outerInstance->fieldsStream->seek(startPointer);
    outerInstance->decompressor->decompress(
        outerInstance->fieldsStream, outerInstance->chunkSize, offset,
        min(length, outerInstance->chunkSize - offset), bytes);
    documentInput = make_shared<DataInputAnonymousInnerClass>(
        shared_from_this(), offset, length);
  } else {
    outerInstance->fieldsStream->seek(startPointer);
    outerInstance->decompressor->decompress(outerInstance->fieldsStream,
                                            totalLength, offset, length, bytes);
    assert(bytes->length == length);
    documentInput = make_shared<ByteArrayDataInput>(bytes->bytes, bytes->offset,
                                                    bytes->length);
  }

  return make_shared<SerializedDocument>(documentInput, length,
                                         numStoredFields);
}

CompressingStoredFieldsReader::BlockState::DataInputAnonymousInnerClass::
    DataInputAnonymousInnerClass(shared_ptr<BlockState> outerInstance,
                                 int offset, int length)
{
  this->outerInstance = outerInstance;
  this->offset = offset;
  this->length = length;
  decompressed = outerInstance->bytes->length;
}

void CompressingStoredFieldsReader::BlockState::DataInputAnonymousInnerClass::
    fillBuffer() 
{
  assert(decompressed <= length);
  if (decompressed == length) {
    // C++ TODO: The following line could not be converted:
    throw java.io.EOFException();
  }
  constexpr int toDecompress =
      min(length - decompressed, outerInstance->outerInstance->chunkSize);
  outerInstance->outerInstance->decompressor.decompress(
      outerInstance->outerInstance->fieldsStream, toDecompress, 0, toDecompress,
      outerInstance->bytes);
  decompressed += toDecompress;
}

char CompressingStoredFieldsReader::BlockState::DataInputAnonymousInnerClass::
    readByte() 
{
  if (outerInstance->bytes->length == 0) {
    fillBuffer();
  }
  --outerInstance->bytes->length;
  return outerInstance->bytes->bytes[outerInstance->bytes->offset++];
}

void CompressingStoredFieldsReader::BlockState::DataInputAnonymousInnerClass::
    readBytes(std::deque<char> &b, int offset, int len) 
{
  while (len > outerInstance->bytes->length) {
    System::arraycopy(outerInstance->bytes->bytes, outerInstance->bytes->offset,
                      b, offset, outerInstance->bytes->length);
    len -= outerInstance->bytes->length;
    offset += outerInstance->bytes->length;
    fillBuffer();
  }
  System::arraycopy(outerInstance->bytes->bytes, outerInstance->bytes->offset,
                    b, offset, len);
  outerInstance->bytes->offset += len;
  outerInstance->bytes->length -= len;
}

shared_ptr<SerializedDocument>
CompressingStoredFieldsReader::document(int docID) 
{
  if (state->contains(docID) == false) {
    fieldsStream->seek(indexReader->getStartPointer(docID));
    state->reset(docID);
  }
  assert(state->contains(docID));
  return state->document(docID);
}

void CompressingStoredFieldsReader::visitDocument(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{

  shared_ptr<SerializedDocument> *const doc = document(docID);

  for (int fieldIDX = 0; fieldIDX < doc->numStoredFields; fieldIDX++) {
    constexpr int64_t infoAndBits = doc->in_->readVLong();
    constexpr int fieldNumber = static_cast<int>(static_cast<int64_t>(
        static_cast<uint64_t>(infoAndBits) >> TYPE_BITS));
    shared_ptr<FieldInfo> *const fieldInfo = fieldInfos->fieldInfo(fieldNumber);

    constexpr int bits = static_cast<int>(infoAndBits & TYPE_MASK);
    assert((bits <= NUMERIC_DOUBLE, L"bits=" + Integer::toHexString(bits)));

    switch (visitor->needsField(fieldInfo)) {
    case SegmentInfo::YES:
      readField(doc->in_, visitor, fieldInfo, bits);
      break;
    case SegmentInfo::NO:
      if (fieldIDX ==
          doc->numStoredFields -
              1) { // don't skipField on last field value; treat like STOP
        return;
      }
      skipField(doc->in_, bits);
      break;
    case STOP:
      return;
    }
  }
}

shared_ptr<StoredFieldsReader> CompressingStoredFieldsReader::clone()
{
  ensureOpen();
  return make_shared<CompressingStoredFieldsReader>(shared_from_this(), false);
}

shared_ptr<StoredFieldsReader> CompressingStoredFieldsReader::getMergeInstance()
{
  ensureOpen();
  return make_shared<CompressingStoredFieldsReader>(shared_from_this(), true);
}

int CompressingStoredFieldsReader::getVersion() { return version; }

shared_ptr<CompressionMode> CompressingStoredFieldsReader::getCompressionMode()
{
  return compressionMode;
}

shared_ptr<CompressingStoredFieldsIndexReader>
CompressingStoredFieldsReader::getIndexReader()
{
  return indexReader;
}

int64_t CompressingStoredFieldsReader::getMaxPointer() { return maxPointer; }

shared_ptr<IndexInput> CompressingStoredFieldsReader::getFieldsStream()
{
  return fieldsStream;
}

int CompressingStoredFieldsReader::getChunkSize() { return chunkSize; }

int64_t CompressingStoredFieldsReader::getNumChunks() { return numChunks; }

int64_t CompressingStoredFieldsReader::getNumDirtyChunks()
{
  return numDirtyChunks;
}

int CompressingStoredFieldsReader::getPackedIntsVersion()
{
  return packedIntsVersion;
}

int64_t CompressingStoredFieldsReader::ramBytesUsed()
{
  return indexReader->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CompressingStoredFieldsReader::getChildResources()
{
  return Collections::singleton(
      Accountables::namedAccountable(L"stored field index", indexReader));
}

void CompressingStoredFieldsReader::checkIntegrity() 
{
  CodecUtil::checksumEntireFile(fieldsStream);
}

wstring CompressingStoredFieldsReader::toString()
{
  return getClass().getSimpleName() + L"(mode=" + compressionMode +
         L",chunksize=" + to_wstring(chunkSize) + L")";
}
} // namespace org::apache::lucene::codecs::compressing