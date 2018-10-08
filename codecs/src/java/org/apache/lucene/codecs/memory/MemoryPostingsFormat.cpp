using namespace std;

#include "MemoryPostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermStats.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/ByteSequenceOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::codecs::memory
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TermStats = org::apache::lucene::codecs::TermStats;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using Util = org::apache::lucene::util::fst::Util;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

MemoryPostingsFormat::MemoryPostingsFormat()
    : MemoryPostingsFormat(false, PackedInts::DEFAULT)
{
}

MemoryPostingsFormat::MemoryPostingsFormat(bool doPackFST,
                                           float acceptableOverheadRatio)
    : org::apache::lucene::codecs::PostingsFormat(L"Memory")
{
}

wstring MemoryPostingsFormat::toString()
{
  return L"PostingsFormat(name=" + getName() + L")";
}

MemoryPostingsFormat::TermsWriter::TermsWriter(shared_ptr<IndexOutput> out,
                                               shared_ptr<FieldInfo> field)
    : out(out), field(field),
      builder(make_shared<org::apache::lucene::index::FieldInfos::Builder<>>(
          FST::INPUT_TYPE::BYTE1, 0, 0, true, true, numeric_limits<int>::max(),
          outputs, true, 15))
{
}

MemoryPostingsFormat::TermsWriter::PostingsWriter::PostingsWriter(
    shared_ptr<MemoryPostingsFormat::TermsWriter> outerInstance)
    : outerInstance(outerInstance)
{
}

void MemoryPostingsFormat::TermsWriter::PostingsWriter::startDoc(
    int docID, int termDocFreq) 
{
  // System.out.println("    startDoc docID=" + docID + " freq=" + termDocFreq);
  constexpr int delta = docID - lastDocID;
  assert(docID == 0 || delta > 0);
  lastDocID = docID;
  docCount++;

  if (outerInstance->field->getIndexOptions() == IndexOptions::DOCS) {
    buffer->writeVInt(delta);
  } else if (termDocFreq == 1) {
    buffer->writeVInt((delta << 1) | 1);
  } else {
    buffer->writeVInt(delta << 1);
    assert(termDocFreq > 0);
    buffer->writeVInt(termDocFreq);
  }

  lastPos = 0;
  lastOffset = 0;
}

void MemoryPostingsFormat::TermsWriter::PostingsWriter::addPosition(
    int pos, shared_ptr<BytesRef> payload, int startOffset,
    int endOffset) 
{
  assert(payload == nullptr || outerInstance->field->hasPayloads());

  // System.out.println("      addPos pos=" + pos + " payload=" + payload);

  constexpr int delta = pos - lastPos;
  assert(delta >= 0);
  lastPos = pos;

  int payloadLen = 0;

  if (outerInstance->field->hasPayloads()) {
    payloadLen = payload == nullptr ? 0 : payload->length;
    if (payloadLen != lastPayloadLen) {
      lastPayloadLen = payloadLen;
      buffer->writeVInt((delta << 1) | 1);
      buffer->writeVInt(payloadLen);
    } else {
      buffer->writeVInt(delta << 1);
    }
  } else {
    buffer->writeVInt(delta);
  }

  if (outerInstance->field->getIndexOptions().compareTo(
          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0) {
    // don't use startOffset - lastEndOffset, because this creates lots of
    // negative vints for synonyms, and the numbers aren't that much smaller
    // anyways.
    int offsetDelta = startOffset - lastOffset;
    int offsetLength = endOffset - startOffset;
    if (offsetLength != lastOffsetLength) {
      buffer->writeVInt(offsetDelta << 1 | 1);
      buffer->writeVInt(offsetLength);
    } else {
      buffer->writeVInt(offsetDelta << 1);
    }
    lastOffset = startOffset;
    lastOffsetLength = offsetLength;
  }

  if (payloadLen > 0) {
    buffer->writeBytes(payload->bytes, payload->offset, payloadLen);
  }
}

shared_ptr<PostingsWriter>
MemoryPostingsFormat::TermsWriter::PostingsWriter::reset()
{
  assert(buffer->getFilePointer() == 0);
  lastDocID = 0;
  docCount = 0;
  lastPayloadLen = 0;
  // force first offset to write its length
  lastOffsetLength = -1;
  return shared_from_this();
}

void MemoryPostingsFormat::TermsWriter::finishTerm(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats) 
{

  if (stats->docFreq == 0) {
    return;
  }
  assert(postingsWriter->docCount == stats->docFreq);

  assert(buffer2->getFilePointer() == 0);

  buffer2->writeVInt(stats->docFreq);
  if (field->getIndexOptions() != IndexOptions::DOCS) {
    buffer2->writeVLong(stats->totalTermFreq - stats->docFreq);
  }
  int pos = static_cast<int>(buffer2->getFilePointer());
  buffer2->writeTo(finalBuffer, 0);
  buffer2->reset();

  constexpr int totalBytes =
      pos + static_cast<int>(postingsWriter->buffer->getFilePointer());
  if (totalBytes > finalBuffer.size()) {
    finalBuffer = ArrayUtil::grow(finalBuffer, totalBytes);
  }
  postingsWriter->buffer->writeTo(finalBuffer, pos);
  postingsWriter->buffer->reset();

  spare->bytes = finalBuffer;
  spare->length = totalBytes;

  // System.out.println("    finishTerm term=" + text.utf8ToString() + " " +
  // totalBytes + " bytes totalTF=" + stats.totalTermFreq); for(int
  // i=0;i<totalBytes;i++) {
  //  System.out.println("      " + Integer.toHexString(finalBuffer[i]&0xFF));
  //}

  builder->add(Util::toIntsRef(text, scratchIntsRef),
               BytesRef::deepCopyOf(spare));
  termCount++;
}

void MemoryPostingsFormat::TermsWriter::finish(int64_t sumTotalTermFreq,
                                               int64_t sumDocFreq,
                                               int docCount) 
{
  if (termCount > 0) {
    out->writeVInt(termCount);
    out->writeVInt(field->number);
    if (field->getIndexOptions() != IndexOptions::DOCS) {
      out->writeVLong(sumTotalTermFreq);
    }
    out->writeVLong(sumDocFreq);
    out->writeVInt(docCount);
    shared_ptr<FST<std::shared_ptr<BytesRef>>> fst = builder->finish();
    fst->save(out);
    // System.out.println("finish field=" + field.name + " fp=" +
    // out.getFilePointer());
  }
}

wstring MemoryPostingsFormat::EXTENSION = L"ram";
const wstring MemoryPostingsFormat::CODEC_NAME = L"MemoryPostings";

MemoryPostingsFormat::MemoryFieldsConsumer::MemoryFieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
    : state(state),
      out(state->directory->createOutput(fileName, state->context))
{
  const wstring fileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, EXTENSION);
  bool success = false;
  try {
    CodecUtil::writeIndexHeader(out, CODEC_NAME, VERSION_CURRENT,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({out});
    }
  }
}

void MemoryPostingsFormat::MemoryFieldsConsumer::write(
    shared_ptr<Fields> fields) 
{
  for (auto field : fields) {

    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<FieldInfo> fieldInfo = state->fieldInfos->fieldInfo(field);
    shared_ptr<TermsWriter> termsWriter =
        make_shared<TermsWriter>(out, fieldInfo);

    shared_ptr<FixedBitSet> docsSeen =
        make_shared<FixedBitSet>(state->segmentInfo->maxDoc());
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    shared_ptr<PostingsEnum> posEnum = nullptr;
    int enumFlags;

    IndexOptions indexOptions = fieldInfo->getIndexOptions();
    bool writeFreqs = indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
    bool writePositions =
        indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
    bool writeOffsets =
        indexOptions.compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
    bool writePayloads = fieldInfo->hasPayloads();

    if (writeFreqs == false) {
      enumFlags = 0;
    } else if (writePositions == false) {
      enumFlags = PostingsEnum::FREQS;
    } else if (writeOffsets == false) {
      if (writePayloads) {
        enumFlags = PostingsEnum::PAYLOADS;
      } else {
        enumFlags = PostingsEnum::POSITIONS;
      }
    } else {
      if (writePayloads) {
        enumFlags = PostingsEnum::PAYLOADS | PostingsEnum::OFFSETS;
      } else {
        enumFlags = PostingsEnum::OFFSETS;
      }
    }

    while (true) {
      shared_ptr<BytesRef> term = termsEnum->next();
      if (term == nullptr) {
        break;
      }
      termsWriter->postingsWriter->reset();

      if (writePositions) {
        posEnum = termsEnum->postings(posEnum, enumFlags);
        postingsEnum = posEnum;
      } else {
        postingsEnum = termsEnum->postings(postingsEnum, enumFlags);
        posEnum.reset();
      }

      int docFreq = 0;
      int64_t totalTermFreq = 0;
      while (true) {
        int docID = postingsEnum->nextDoc();
        if (docID == PostingsEnum::NO_MORE_DOCS) {
          break;
        }
        docsSeen->set(docID);
        docFreq++;

        int freq;
        if (writeFreqs) {
          freq = postingsEnum->freq();
          totalTermFreq += freq;
        } else {
          freq = -1;
        }

        termsWriter->postingsWriter->startDoc(docID, freq);
        if (writePositions) {
          for (int i = 0; i < freq; i++) {
            int pos = posEnum->nextPosition();
            shared_ptr<BytesRef> payload =
                writePayloads ? posEnum->getPayload() : nullptr;
            int startOffset;
            int endOffset;
            if (writeOffsets) {
              startOffset = posEnum->startOffset();
              endOffset = posEnum->endOffset();
            } else {
              startOffset = -1;
              endOffset = -1;
            }
            termsWriter->postingsWriter->addPosition(pos, payload, startOffset,
                                                     endOffset);
          }
        }
      }
      termsWriter->finishTerm(term,
                              make_shared<TermStats>(docFreq, totalTermFreq));
      sumDocFreq += docFreq;
      sumTotalTermFreq += totalTermFreq;
    }

    termsWriter->finish(sumTotalTermFreq, sumDocFreq, docsSeen->cardinality());
  }
}

MemoryPostingsFormat::MemoryFieldsConsumer::~MemoryFieldsConsumer()
{
  if (closed) {
    return;
  }
  closed = true;

  // EOF marker:
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput out =
  // this.out)
  {
    org::apache::lucene::store::IndexOutput out = this->out;
    out->writeVInt(0);
    CodecUtil::writeFooter(out);
  }
}

shared_ptr<FieldsConsumer> MemoryPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<MemoryFieldsConsumer>(state);
}

MemoryPostingsFormat::FSTDocsEnum::FSTDocsEnum(IndexOptions indexOptions,
                                               bool storePayloads)
    : indexOptions(indexOptions), storePayloads(storePayloads)
{
}

bool MemoryPostingsFormat::FSTDocsEnum::canReuse(IndexOptions indexOptions,
                                                 bool storePayloads)
{
  return indexOptions == this->indexOptions &&
         storePayloads == this->storePayloads;
}

shared_ptr<FSTDocsEnum>
MemoryPostingsFormat::FSTDocsEnum::reset(shared_ptr<BytesRef> bufferIn,
                                         int numDocs)
{
  assert(numDocs > 0);
  if (buffer.size() < bufferIn->length) {
    buffer = ArrayUtil::grow(buffer, bufferIn->length);
  }
  in_->reset(buffer, 0, bufferIn->length);
  System::arraycopy(bufferIn->bytes, bufferIn->offset, buffer, 0,
                    bufferIn->length);
  docID_ = -1;
  accum = 0;
  docUpto = 0;
  freq_ = 1;
  payloadLen = 0;
  this->numDocs = numDocs;
  return shared_from_this();
}

int MemoryPostingsFormat::FSTDocsEnum::nextDoc()
{
  // System.out.println("  nextDoc cycle docUpto=" + docUpto + " numDocs=" +
  // numDocs + " fp=" + in.getPosition() + " this=" + this);
  if (docUpto == numDocs) {
    // System.out.println("    END");
    return docID_ = NO_MORE_DOCS;
  }
  docUpto++;
  if (indexOptions == IndexOptions::DOCS) {
    accum += in_->readVInt();
  } else {
    constexpr int code = in_->readVInt();
    accum += static_cast<int>(static_cast<unsigned int>(code) >> 1);
    // System.out.println("  docID=" + accum + " code=" + code);
    if ((code & 1) != 0) {
      freq_ = 1;
    } else {
      freq_ = in_->readVInt();
      assert(freq_ > 0);
    }

    if (indexOptions == IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) {
      // Skip positions/payloads
      for (int posUpto = 0; posUpto < freq_; posUpto++) {
        if (!storePayloads) {
          in_->readVInt();
        } else {
          constexpr int posCode = in_->readVInt();
          if ((posCode & 1) != 0) {
            payloadLen = in_->readVInt();
          }
          in_->skipBytes(payloadLen);
        }
      }
    } else if (indexOptions ==
               IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) {
      // Skip positions/offsets/payloads
      for (int posUpto = 0; posUpto < freq_; posUpto++) {
        int posCode = in_->readVInt();
        if (storePayloads && ((posCode & 1) != 0)) {
          payloadLen = in_->readVInt();
        }
        if ((in_->readVInt() & 1) != 0) {
          // new offset length
          in_->readVInt();
        }
        if (storePayloads) {
          in_->skipBytes(payloadLen);
        }
      }
    }
  }

  // System.out.println("    return docID=" + accum + " freq=" + freq);
  return (docID_ = accum);
}

int MemoryPostingsFormat::FSTDocsEnum::docID() { return docID_; }

int MemoryPostingsFormat::FSTDocsEnum::advance(int target) 
{
  // TODO: we could make more efficient version, but, it
  // should be rare that this will matter in practice
  // since usually apps will not store "big" fields in
  // this codec!
  return slowAdvance(target);
}

int MemoryPostingsFormat::FSTDocsEnum::freq() { return freq_; }

int64_t MemoryPostingsFormat::FSTDocsEnum::cost() { return numDocs; }

int MemoryPostingsFormat::FSTDocsEnum::nextPosition() 
{
  return -1;
}

int MemoryPostingsFormat::FSTDocsEnum::startOffset() 
{
  return -1;
}

int MemoryPostingsFormat::FSTDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
MemoryPostingsFormat::FSTDocsEnum::getPayload() 
{
  return nullptr;
}

MemoryPostingsFormat::FSTPostingsEnum::FSTPostingsEnum(bool storePayloads,
                                                       bool storeOffsets)
    : storePayloads(storePayloads), storeOffsets(storeOffsets)
{
}

bool MemoryPostingsFormat::FSTPostingsEnum::canReuse(bool storePayloads,
                                                     bool storeOffsets)
{
  return storePayloads == this->storePayloads &&
         storeOffsets == this->storeOffsets;
}

shared_ptr<FSTPostingsEnum>
MemoryPostingsFormat::FSTPostingsEnum::reset(shared_ptr<BytesRef> bufferIn,
                                             int numDocs)
{
  assert(numDocs > 0);

  // System.out.println("D&P reset bytes this=" + this);
  // for(int i=bufferIn.offset;i<bufferIn.length;i++) {
  //   System.out.println("  " + Integer.toHexString(bufferIn.bytes[i]&0xFF));
  // }

  if (buffer.size() < bufferIn->length) {
    buffer = ArrayUtil::grow(buffer, bufferIn->length);
  }
  in_->reset(buffer, 0, bufferIn->length - bufferIn->offset);
  System::arraycopy(bufferIn->bytes, bufferIn->offset, buffer, 0,
                    bufferIn->length);
  docID_ = -1;
  accum = 0;
  docUpto = 0;
  payload->bytes = buffer;
  payloadLength = 0;
  this->numDocs = numDocs;
  posPending = 0;
  startOffset_ =
      storeOffsets ? 0 : -1; // always return -1 if no offsets are stored
  offsetLength = 0;
  return shared_from_this();
}

int MemoryPostingsFormat::FSTPostingsEnum::nextDoc()
{
  while (posPending > 0) {
    nextPosition();
  }
  while (true) {
    // System.out.println("  nextDoc cycle docUpto=" + docUpto + " numDocs=" +
    // numDocs + " fp=" + in.getPosition() + " this=" + this);
    if (docUpto == numDocs) {
      // System.out.println("    END");
      return docID_ = NO_MORE_DOCS;
    }
    docUpto++;

    constexpr int code = in_->readVInt();
    accum += static_cast<int>(static_cast<unsigned int>(code) >> 1);
    if ((code & 1) != 0) {
      freq_ = 1;
    } else {
      freq_ = in_->readVInt();
      assert(freq_ > 0);
    }

    pos = 0;
    startOffset_ = storeOffsets ? 0 : -1;
    posPending = freq_;
    // System.out.println("    return docID=" + accum + " freq=" + freq);
    return (docID_ = accum);
  }
}

int MemoryPostingsFormat::FSTPostingsEnum::nextPosition()
{
  // System.out.println("    nextPos storePayloads=" + storePayloads + " this="
  // + this);
  assert(posPending > 0);
  posPending--;
  if (!storePayloads) {
    pos += in_->readVInt();
  } else {
    constexpr int code = in_->readVInt();
    pos += static_cast<int>(static_cast<unsigned int>(code) >> 1);
    if ((code & 1) != 0) {
      payloadLength = in_->readVInt();
      // System.out.println("      new payloadLen=" + payloadLength);
      //} else {
      // System.out.println("      same payloadLen=" + payloadLength);
    }
  }

  if (storeOffsets) {
    int offsetCode = in_->readVInt();
    if ((offsetCode & 1) != 0) {
      // new offset length
      offsetLength = in_->readVInt();
    }
    startOffset_ +=
        static_cast<int>(static_cast<unsigned int>(offsetCode) >> 1);
  }

  if (storePayloads) {
    payload->offset = in_->getPosition();
    in_->skipBytes(payloadLength);
    payload->length = payloadLength;
  }

  // System.out.println("      pos=" + pos + " payload=" + payload + " fp=" +
  // in.getPosition());
  return pos;
}

int MemoryPostingsFormat::FSTPostingsEnum::startOffset()
{
  return startOffset_;
}

int MemoryPostingsFormat::FSTPostingsEnum::endOffset()
{
  return startOffset_ + offsetLength;
}

shared_ptr<BytesRef> MemoryPostingsFormat::FSTPostingsEnum::getPayload()
{
  return payload->length > 0 ? payload : nullptr;
}

int MemoryPostingsFormat::FSTPostingsEnum::docID() { return docID_; }

int MemoryPostingsFormat::FSTPostingsEnum::advance(int target) throw(
    IOException)
{
  // TODO: we could make more efficient version, but, it
  // should be rare that this will matter in practice
  // since usually apps will not store "big" fields in
  // this codec!
  return slowAdvance(target);
}

int MemoryPostingsFormat::FSTPostingsEnum::freq() { return freq_; }

int64_t MemoryPostingsFormat::FSTPostingsEnum::cost() { return numDocs; }

MemoryPostingsFormat::FSTTermsEnum::FSTTermsEnum(
    shared_ptr<FieldInfo> field, shared_ptr<FST<std::shared_ptr<BytesRef>>> fst)
    : field(field), fstEnum(make_shared<BytesRefFSTEnum<>>(fst))
{
}

void MemoryPostingsFormat::FSTTermsEnum::decodeMetaData()
{
  if (!didDecode) {
    buffer->reset(current->output->bytes, current->output->offset,
                  current->output->length);
    docFreq_ = buffer->readVInt();
    if (field->getIndexOptions() != IndexOptions::DOCS) {
      totalTermFreq_ = docFreq_ + buffer->readVLong();
    } else {
      totalTermFreq_ = -1;
    }
    postingsSpare->bytes = current->output->bytes;
    postingsSpare->offset = buffer->getPosition();
    postingsSpare->length = current->output->length -
                            (buffer->getPosition() - current->output->offset);
    // System.out.println("  df=" + docFreq + " totTF=" + totalTermFreq + "
    // offset=" + buffer.getPosition() + " len=" + current.output.length);
    didDecode = true;
  }
}

bool MemoryPostingsFormat::FSTTermsEnum::seekExact(
    shared_ptr<BytesRef> text) 
{
  // System.out.println("te.seekExact text=" + field.name + ":" +
  // text.utf8ToString() + " this=" + this);
  current = fstEnum->seekExact(text);
  didDecode = false;
  return current != nullptr;
}

SeekStatus MemoryPostingsFormat::FSTTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{
  // System.out.println("te.seek text=" + field.name + ":" + text.utf8ToString()
  // + " this=" + this);
  current = fstEnum->seekCeil(text);
  if (current == nullptr) {
    return SeekStatus::END;
  } else {

    // System.out.println("  got term=" + current.input.utf8ToString());
    // for(int i=0;i<current.output.length;i++) {
    //   System.out.println("    " +
    //   Integer.toHexString(current.output.bytes[i]&0xFF));
    // }

    didDecode = false;

    if (text->equals(current->input)) {
      // System.out.println("  found!");
      return SeekStatus::FOUND;
    } else {
      // System.out.println("  not found: " + current.input.utf8ToString());
      return SeekStatus::NOT_FOUND;
    }
  }
}

shared_ptr<PostingsEnum>
MemoryPostingsFormat::FSTTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                             int flags)
{

  // TODO: the logic of which enum impl to choose should be refactored to be
  // simpler...
  bool hasPositions = field->getIndexOptions().compareTo(
                          IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
  if (hasPositions &&
      PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    bool hasOffsets =
        field->getIndexOptions().compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
    decodeMetaData();
    shared_ptr<FSTPostingsEnum> docsAndPositionsEnum;
    if (reuse == nullptr ||
        !(std::dynamic_pointer_cast<FSTPostingsEnum>(reuse) != nullptr)) {
      docsAndPositionsEnum =
          make_shared<FSTPostingsEnum>(field->hasPayloads(), hasOffsets);
    } else {
      docsAndPositionsEnum = std::static_pointer_cast<FSTPostingsEnum>(reuse);
      if (!docsAndPositionsEnum->canReuse(field->hasPayloads(), hasOffsets)) {
        docsAndPositionsEnum =
            make_shared<FSTPostingsEnum>(field->hasPayloads(), hasOffsets);
      }
    }
    // System.out.println("D&P reset this=" + this);
    return docsAndPositionsEnum->reset(postingsSpare, docFreq_);
  }

  decodeMetaData();
  shared_ptr<FSTDocsEnum> docsEnum;

  if (reuse == nullptr ||
      !(std::dynamic_pointer_cast<FSTDocsEnum>(reuse) != nullptr)) {
    docsEnum = make_shared<FSTDocsEnum>(field->getIndexOptions(),
                                        field->hasPayloads());
  } else {
    docsEnum = std::static_pointer_cast<FSTDocsEnum>(reuse);
    if (!docsEnum->canReuse(field->getIndexOptions(), field->hasPayloads())) {
      docsEnum = make_shared<FSTDocsEnum>(field->getIndexOptions(),
                                          field->hasPayloads());
    }
  }
  return docsEnum->reset(this->postingsSpare, docFreq_);
}

shared_ptr<BytesRef> MemoryPostingsFormat::FSTTermsEnum::term()
{
  return current->input;
}

shared_ptr<BytesRef>
MemoryPostingsFormat::FSTTermsEnum::next() 
{
  // System.out.println("te.next");
  current = fstEnum->next();
  if (current == nullptr) {
    // System.out.println("  END");
    return nullptr;
  }
  didDecode = false;
  // System.out.println("  term=" + field.name + ":" +
  // current.input.utf8ToString());
  return current->input;
}

int MemoryPostingsFormat::FSTTermsEnum::docFreq()
{
  decodeMetaData();
  return docFreq_;
}

int64_t MemoryPostingsFormat::FSTTermsEnum::totalTermFreq()
{
  decodeMetaData();
  return totalTermFreq_;
}

void MemoryPostingsFormat::FSTTermsEnum::seekExact(int64_t ord)
{
  // NOTE: we could add this...
  throw make_shared<UnsupportedOperationException>();
}

int64_t MemoryPostingsFormat::FSTTermsEnum::ord()
{
  // NOTE: we could add this...
  throw make_shared<UnsupportedOperationException>();
}

MemoryPostingsFormat::TermsReader::TermsReader(
    shared_ptr<FieldInfos> fieldInfos, shared_ptr<IndexInput> in_,
    int termCount) 
    : sumDocFreq(in_->readVLong()), docCount(in_->readVInt()),
      termCount(termCount), field(fieldInfos->fieldInfo(fieldNumber))
{
  constexpr int fieldNumber = in_->readVInt();
  if (field == nullptr) {
    throw make_shared<CorruptIndexException>(
        L"invalid field number: " + to_wstring(fieldNumber), in_);
  } else if (field->getIndexOptions() != IndexOptions::DOCS) {
    sumTotalTermFreq = in_->readVLong();
  } else {
    sumTotalTermFreq = -1;
  }

  fst = make_shared<FST<BytesRef>>(in_, outputs);
}

int64_t MemoryPostingsFormat::TermsReader::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t MemoryPostingsFormat::TermsReader::getSumDocFreq()
{
  return sumDocFreq;
}

int MemoryPostingsFormat::TermsReader::getDocCount() { return docCount; }

int64_t MemoryPostingsFormat::TermsReader::size() { return termCount; }

shared_ptr<TermsEnum> MemoryPostingsFormat::TermsReader::iterator()
{
  return make_shared<FSTTermsEnum>(field, fst);
}

bool MemoryPostingsFormat::TermsReader::hasFreqs()
{
  return field->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
}

bool MemoryPostingsFormat::TermsReader::hasOffsets()
{
  return field->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool MemoryPostingsFormat::TermsReader::hasPositions()
{
  return field->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool MemoryPostingsFormat::TermsReader::hasPayloads()
{
  return field->hasPayloads();
}

int64_t MemoryPostingsFormat::TermsReader::ramBytesUsed()
{
  return ((fst != nullptr) ? fst->ramBytesUsed() : 0);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
MemoryPostingsFormat::TermsReader::getChildResources()
{
  if (fst == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"terms", fst));
  }
}

wstring MemoryPostingsFormat::TermsReader::toString()
{
  return L"MemoryTerms(terms=" + to_wstring(termCount) + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}

shared_ptr<FieldsProducer> MemoryPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, EXTENSION);

  shared_ptr<SortedMap<wstring, std::shared_ptr<TermsReader>>> *const fields =
      map_obj<wstring, std::shared_ptr<TermsReader>>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // in = state.directory.openChecksumInput(fileName,
  // org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::ChecksumIndexInput in_ =
        state->directory->openChecksumInput(
            fileName, org::apache::lucene::store::IOContext::READONCE);
    runtime_error priorE = nullptr;
    try {
      CodecUtil::checkIndexHeader(in_, CODEC_NAME, VERSION_START,
                                  VERSION_CURRENT, state->segmentInfo->getId(),
                                  state->segmentSuffix);
      while (true) {
        constexpr int termCount = in_->readVInt();
        if (termCount == 0) {
          break;
        }
        shared_ptr<TermsReader> *const termsReader =
            make_shared<TermsReader>(state->fieldInfos, in_, termCount);
        // System.out.println("load field=" + termsReader.field.name);
        fields->put(termsReader->field.name, termsReader);
      }
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(in_, priorE);
    }
  }

  return make_shared<FieldsProducerAnonymousInnerClass>(shared_from_this(),
                                                        fields);
}

MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::
    FieldsProducerAnonymousInnerClass(
        shared_ptr<MemoryPostingsFormat> outerInstance,
        shared_ptr<SortedMap<wstring, std::shared_ptr<TermsReader>>> fields)
{
  this->outerInstance = outerInstance;
  this->fields = fields;
}

shared_ptr<Iterator<wstring>>
MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::iterator()
{
  return Collections::unmodifiableSet(fields->keySet()).begin();
}

shared_ptr<Terms>
MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::terms(
    const wstring &field)
{
  return fields->get(field);
}

int MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::size()
{
  return fields->size();
}

MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::
    ~FieldsProducerAnonymousInnerClass()
{
  // Drop ref to FST:
  for (auto termsReader : fields) {
    termsReader->second->fst = nullptr;
  }
}

int64_t
MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto entry : fields) {
    sizeInBytes += (entry.first->length() * Character::BYTES);
    sizeInBytes += entry.second::ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::getChildResources()
{
  return Accountables::namedAccountables(L"field", fields);
}

wstring MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::toString()
{
  return L"MemoryPostings(fields=" + fields->size() + L")";
}

void MemoryPostingsFormat::FieldsProducerAnonymousInnerClass::
    checkIntegrity() 
{
}
} // namespace org::apache::lucene::codecs::memory