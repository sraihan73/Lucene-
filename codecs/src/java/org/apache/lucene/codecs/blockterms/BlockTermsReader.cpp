using namespace std;

#include "BlockTermsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "BlockTermsWriter.h"
#include "TermsIndexReaderBase.h"

namespace org::apache::lucene::codecs::blockterms
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

BlockTermsReader::FieldAndTerm::FieldAndTerm() {}

BlockTermsReader::FieldAndTerm::FieldAndTerm(shared_ptr<FieldAndTerm> other)
{
  field = other->field;
  term = BytesRef::deepCopyOf(other->term);
}

bool BlockTermsReader::FieldAndTerm::equals(any _other)
{
  shared_ptr<FieldAndTerm> other =
      any_cast<std::shared_ptr<FieldAndTerm>>(_other);
  return other->field == field && term->bytesEquals(other->term);
}

shared_ptr<FieldAndTerm> BlockTermsReader::FieldAndTerm::clone()
{
  return make_shared<FieldAndTerm>(shared_from_this());
}

int BlockTermsReader::FieldAndTerm::hashCode()
{
  return field.hashCode() * 31 + term->hashCode();
}

BlockTermsReader::BlockTermsReader(
    shared_ptr<TermsIndexReaderBase> indexReader,
    shared_ptr<PostingsReaderBase> postingsReader,
    shared_ptr<SegmentReadState> state) 
    : in_(state->directory->openInput(filename, state->context)),
      postingsReader(postingsReader)
{

  wstring filename = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix,
      BlockTermsWriter::TERMS_EXTENSION);

  bool success = false;
  try {
    CodecUtil::checkIndexHeader(
        in_, BlockTermsWriter::CODEC_NAME, BlockTermsWriter::VERSION_START,
        BlockTermsWriter::VERSION_CURRENT, state->segmentInfo->getId(),
        state->segmentSuffix);

    // Have PostingsReader init itself
    postingsReader->init(in_, state);

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(in_);

    // Read per-field details
    seekDir(in_);

    constexpr int numFields = in_->readVInt();
    if (numFields < 0) {
      throw make_shared<CorruptIndexException>(
          L"invalid number of fields: " + to_wstring(numFields), in_);
    }
    for (int i = 0; i < numFields; i++) {
      constexpr int field = in_->readVInt();
      constexpr int64_t numTerms = in_->readVLong();
      assert(numTerms >= 0);
      constexpr int64_t termsStartPointer = in_->readVLong();
      shared_ptr<FieldInfo> *const fieldInfo =
          state->fieldInfos->fieldInfo(field);
      constexpr int64_t sumTotalTermFreq =
          fieldInfo->getIndexOptions() == IndexOptions::DOCS ? -1
                                                             : in_->readVLong();
      constexpr int64_t sumDocFreq = in_->readVLong();
      constexpr int docCount = in_->readVInt();
      constexpr int longsSize = in_->readVInt();
      if (docCount < 0 ||
          docCount > state->segmentInfo
                         ->maxDoc()) { // #docs with field must be <= #docs
        throw make_shared<CorruptIndexException>(
            L"invalid docCount: " + to_wstring(docCount) + L" maxDoc: " +
                to_wstring(state->segmentInfo->maxDoc()),
            in_);
      }
      if (sumDocFreq < docCount) { // #postings must be >= #docs with field
        throw make_shared<CorruptIndexException>(
            L"invalid sumDocFreq: " + to_wstring(sumDocFreq) + L" docCount: " +
                to_wstring(docCount),
            in_);
      }
      if (sumTotalTermFreq != -1 &&
          sumTotalTermFreq < sumDocFreq) { // #positions must be >= #postings
        throw make_shared<CorruptIndexException>(
            L"invalid sumTotalTermFreq: " + to_wstring(sumTotalTermFreq) +
                L" sumDocFreq: " + to_wstring(sumDocFreq),
            in_);
      }
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      shared_ptr<FieldReader> previous = fields.emplace(
          fieldInfo->name,
          make_shared<FieldReader>(shared_from_this(), fieldInfo, numTerms,
                                   termsStartPointer, sumTotalTermFreq,
                                   sumDocFreq, docCount, longsSize));
      if (previous != nullptr) {
        throw make_shared<CorruptIndexException>(
            L"duplicate fields: " + fieldInfo->name, in_);
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      delete in_;
    }
  }

  this->indexReader = indexReader;
}

void BlockTermsReader::seekDir(shared_ptr<IndexInput> input) 
{
  input->seek(input->length() - CodecUtil::footerLength() - 8);
  int64_t dirOffset = input->readLong();
  input->seek(dirOffset);
}

BlockTermsReader::~BlockTermsReader()
{
  try {
    try {
      if (indexReader != nullptr) {
        delete indexReader;
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // null so if an app hangs on to us (ie, we are not
      // GCable, despite being closed) we still free most
      // ram
      indexReader.reset();
      if (in_ != nullptr) {
        delete in_;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (postingsReader != nullptr) {
      delete postingsReader;
    }
  }
}

shared_ptr<Iterator<wstring>> BlockTermsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms>
BlockTermsReader::terms(const wstring &field) 
{
  assert(field != L"");
  return fields[field];
}

int BlockTermsReader::size() { return fields.size(); }

BlockTermsReader::FieldReader::FieldReader(
    shared_ptr<BlockTermsReader> outerInstance, shared_ptr<FieldInfo> fieldInfo,
    int64_t numTerms, int64_t termsStartPointer, int64_t sumTotalTermFreq,
    int64_t sumDocFreq, int docCount, int longsSize)
    : numTerms(numTerms), fieldInfo(fieldInfo),
      termsStartPointer(termsStartPointer), sumTotalTermFreq(sumTotalTermFreq),
      sumDocFreq(sumDocFreq), docCount(docCount), longsSize(longsSize),
      outerInstance(outerInstance)
{
  assert(numTerms > 0);
}

int64_t BlockTermsReader::FieldReader::ramBytesUsed()
{
  return FIELD_READER_RAM_BYTES_USED;
}

shared_ptr<TermsEnum>
BlockTermsReader::FieldReader::iterator() 
{
  return make_shared<SegmentTermsEnum>(shared_from_this());
}

bool BlockTermsReader::FieldReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool BlockTermsReader::FieldReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool BlockTermsReader::FieldReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool BlockTermsReader::FieldReader::hasPayloads()
{
  return fieldInfo->hasPayloads();
}

int64_t BlockTermsReader::FieldReader::size() { return numTerms; }

int64_t BlockTermsReader::FieldReader::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t BlockTermsReader::FieldReader::getSumDocFreq() 
{
  return sumDocFreq;
}

int BlockTermsReader::FieldReader::getDocCount() 
{
  return docCount;
}

BlockTermsReader::FieldReader::SegmentTermsEnum::SegmentTermsEnum(
    shared_ptr<BlockTermsReader::FieldReader> outerInstance) 
    : in_(outerInstance->outerInstance->in_->clone()),
      state(outerInstance->outerInstance->postingsReader.newTermState()),
      doOrd(outerInstance->outerInstance->indexReader.supportsOrd()),
      indexEnum(outerInstance->outerInstance->indexReader.getFieldEnum(
          outerInstance->fieldInfo)),
      outerInstance(outerInstance)
{
  in_->seek(outerInstance->termsStartPointer);
  fieldTerm->field = outerInstance->fieldInfo->name;
  state->totalTermFreq = -1;
  state->ord = -1;

  termSuffixes = std::deque<char>(128);
  docFreqBytes = std::deque<char>(64);
  // System.out.println("BTR.enum init this=" + this + " postingsReader=" +
  // postingsReader);
  longs = std::deque<int64_t>(outerInstance->longsSize);
}

SeekStatus BlockTermsReader::FieldReader::SegmentTermsEnum::seekCeil(
    shared_ptr<BytesRef> target) 
{

  if (indexEnum == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  // System.out.println("BTR.seek seg=" + segment + " target=" + fieldInfo.name
  // + ":" + target.utf8ToString() + " " + target + " current=" +
  // term().utf8ToString() + " " + term() + " indexIsCurrent=" + indexIsCurrent +
  // " didIndexNext=" + didIndexNext + " seekPending=" + seekPending + "
  // divisor=" + indexReader.getDivisor() + " this="  + this);
  if (didIndexNext) {
    if (nextIndexTerm == nullptr) {
      // System.out.println("  nextIndexTerm=null");
    } else {
      // System.out.println("  nextIndexTerm=" + nextIndexTerm.utf8ToString());
    }
  }

  bool doSeek = true;

  // See if we can avoid seeking, because target term
  // is after current term but before next index term:
  if (indexIsCurrent) {

    constexpr int cmp = term_->get()->compareTo(target);

    if (cmp == 0) {
      // Already at the requested term
      return SeekStatus::FOUND;
    } else if (cmp < 0) {

      // Target term is after current term
      if (!didIndexNext) {
        if (indexEnum->next() == -1) {
          nextIndexTerm.reset();
        } else {
          nextIndexTerm = indexEnum->term();
        }
        // System.out.println("  now do index next() nextIndexTerm=" +
        // (nextIndexTerm == null ? "null" : nextIndexTerm.utf8ToString()));
        didIndexNext = true;
      }

      if (nextIndexTerm == nullptr || target->compareTo(nextIndexTerm) < 0) {
        // Optimization: requested term is within the
        // same term block we are now in; skip seeking
        // (but do scanning):
        doSeek = false;
        // System.out.println("  skip seek: nextIndexTerm=" + (nextIndexTerm ==
        // null ? "null" : nextIndexTerm.utf8ToString()));
      }
    }
  }

  if (doSeek) {
    // System.out.println("  seek");

    // Ask terms index to find biggest indexed term (=
    // first term in a block) that's <= our text:
    in_->seek(indexEnum->seek(target));
    bool result = nextBlock();

    // Block must exist since, at least, the indexed term
    // is in the block:
    assert(result);

    indexIsCurrent = true;
    didIndexNext = false;

    if (doOrd) {
      state->ord = indexEnum->ord() - 1;
    }

    term_->copyBytes(indexEnum->term());
    // System.out.println("  seek: term=" + term.utf8ToString());
  } else {
    // System.out.println("  skip seek");
    if (state->termBlockOrd == blockTermCount && !nextBlock()) {
      indexIsCurrent = false;
      return SeekStatus::END;
    }
  }

  seekPending = false;

  int common = 0;

  // Scan within block.  We could do this by calling
  // _next() and testing the resulting term, but this
  // is wasteful.  Instead, we first confirm the
  // target matches the common prefix of this block,
  // and then we scan the term bytes directly from the
  // termSuffixesreader's byte[], saving a copy into
  // the BytesRef term per term.  Only when we return
  // do we then copy the bytes into the term.

  while (true) {

    // First, see if target term matches common prefix
    // in this block:
    if (common < termBlockPrefix) {
      constexpr int cmp = (term_->byteAt(common) & 0xFF) -
                          (target->bytes[target->offset + common] & 0xFF);
      if (cmp < 0) {

        // TODO: maybe we should store common prefix
        // in block header?  (instead of relying on
        // last term of previous block)

        // Target's prefix is after the common block
        // prefix, so term cannot be in this block
        // but it could be in next block.  We
        // must scan to end-of-block to set common
        // prefix for next block:
        if (state->termBlockOrd < blockTermCount) {
          while (state->termBlockOrd < blockTermCount - 1) {
            state->termBlockOrd++;
            state->ord++;
            termSuffixesReader->skipBytes(termSuffixesReader->readVInt());
          }
          constexpr int suffix = termSuffixesReader->readVInt();
          term_->setLength(termBlockPrefix + suffix);
          term_->grow(term_->length());
          termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix,
                                        suffix);
        }
        state->ord++;

        if (!nextBlock()) {
          indexIsCurrent = false;
          return SeekStatus::END;
        }
        common = 0;

      } else if (cmp > 0) {
        // Target's prefix is before the common prefix
        // of this block, so we position to start of
        // block and return NOT_FOUND:
        assert(state->termBlockOrd == 0);

        constexpr int suffix = termSuffixesReader->readVInt();
        term_->setLength(termBlockPrefix + suffix);
        term_->grow(term_->length());
        termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix, suffix);
        return SeekStatus::NOT_FOUND;
      } else {
        common++;
      }

      continue;
    }

    // Test every term in this block
    while (true) {
      state->termBlockOrd++;
      state->ord++;

      constexpr int suffix = termSuffixesReader->readVInt();

      // We know the prefix matches, so just compare the new suffix:
      constexpr int termLen = termBlockPrefix + suffix;
      int bytePos = termSuffixesReader->getPosition();

      bool next = false;
      constexpr int limit =
          target->offset +
          (termLen < target->length ? termLen : target->length);
      int targetPos = target->offset + termBlockPrefix;
      while (targetPos < limit) {
        constexpr int cmp = (termSuffixes[bytePos++] & 0xFF) -
                            (target->bytes[targetPos++] & 0xFF);
        if (cmp < 0) {
          // Current term is still before the target;
          // keep scanning
          next = true;
          break;
        } else if (cmp > 0) {
          // Done!  Current term is after target. Stop
          // here, fill in real term, return NOT_FOUND.
          term_->setLength(termBlockPrefix + suffix);
          term_->grow(term_->length());
          termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix,
                                        suffix);
          // System.out.println("  NOT_FOUND");
          return SeekStatus::NOT_FOUND;
        }
      }

      if (!next && target->length <= termLen) {
        term_->setLength(termBlockPrefix + suffix);
        term_->grow(term_->length());
        termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix, suffix);

        if (target->length == termLen) {
          // Done!  Exact match.  Stop here, fill in
          // real term, return FOUND.
          // System.out.println("  FOUND");
          return SeekStatus::FOUND;
        } else {
          // System.out.println("  NOT_FOUND");
          return SeekStatus::NOT_FOUND;
        }
      }

      if (state->termBlockOrd == blockTermCount) {
        // Must pre-fill term for next block's common prefix
        term_->setLength(termBlockPrefix + suffix);
        term_->grow(term_->length());
        termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix, suffix);
        break;
      } else {
        termSuffixesReader->skipBytes(suffix);
      }
    }

    // The purpose of the terms dict index is to seek
    // the enum to the closest index term before the
    // term we are looking for.  So, we should never
    // cross another index term (besides the first
    // one) while we are scanning:

    assert(indexIsCurrent);

    if (!nextBlock()) {
      // System.out.println("  END");
      indexIsCurrent = false;
      return SeekStatus::END;
    }
    common = 0;
  }
}

shared_ptr<BytesRef>
BlockTermsReader::FieldReader::SegmentTermsEnum::next() 
{
  // System.out.println("BTR.next() seekPending=" + seekPending + "
  // pendingSeekCount=" + state.termBlockOrd);

  // If seek was previously called and the term was cached,
  // usually caller is just going to pull a D/&PEnum or get
  // docFreq, etc.  But, if they then call next(),
  // this method catches up all internal state so next()
  // works properly:
  if (seekPending) {
    assert(!indexIsCurrent);
    in_->seek(state->blockFilePointer);
    constexpr int pendingSeekCount = state->termBlockOrd;
    bool result = nextBlock();

    constexpr int64_t savOrd = state->ord;

    // Block must exist since seek(TermState) was called w/ a
    // TermState previously returned by this enum when positioned
    // on a real term:
    assert(result);

    while (state->termBlockOrd < pendingSeekCount) {
      shared_ptr<BytesRef> nextResult = _next();
      assert(nextResult != nullptr);
    }
    seekPending = false;
    state->ord = savOrd;
  }
  return _next();
}

shared_ptr<BytesRef>
BlockTermsReader::FieldReader::SegmentTermsEnum::_next() 
{
  // System.out.println("BTR._next seg=" + segment + " this=" + this + "
  // termCount=" + state.termBlockOrd + " (vs " + blockTermCount + ")");
  if (state->termBlockOrd == blockTermCount && !nextBlock()) {
    // System.out.println("  eof");
    indexIsCurrent = false;
    return nullptr;
  }

  // TODO: cutover to something better for these ints!  simple64?
  constexpr int suffix = termSuffixesReader->readVInt();
  // System.out.println("  suffix=" + suffix);

  term_->setLength(termBlockPrefix + suffix);
  term_->grow(term_->length());
  termSuffixesReader->readBytes(term_->bytes(), termBlockPrefix, suffix);
  state->termBlockOrd++;

  // NOTE: meaningless in the non-ord case
  state->ord++;

  // System.out.println("  return term=" + fieldInfo.name + ":" +
  // term.utf8ToString() + " " + term + " tbOrd=" + state.termBlockOrd);
  return term_->get();
}

shared_ptr<BytesRef> BlockTermsReader::FieldReader::SegmentTermsEnum::term()
{
  return term_->get();
}

int BlockTermsReader::FieldReader::SegmentTermsEnum::docFreq() throw(
    IOException)
{
  // System.out.println("BTR.docFreq");
  decodeMetaData();
  // System.out.println("  return " + state.docFreq);
  return state->docFreq;
}

int64_t
BlockTermsReader::FieldReader::SegmentTermsEnum::totalTermFreq() throw(
    IOException)
{
  decodeMetaData();
  return state->totalTermFreq;
}

shared_ptr<PostingsEnum>
BlockTermsReader::FieldReader::SegmentTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  // System.out.println("BTR.docs this=" + this);
  decodeMetaData();
  // System.out.println("BTR.docs:  state.docFreq=" + state.docFreq);
  return outerInstance->outerInstance->postingsReader.postings(
      outerInstance->fieldInfo, state, reuse, flags);
}

void BlockTermsReader::FieldReader::SegmentTermsEnum::seekExact(
    shared_ptr<BytesRef> target, shared_ptr<TermState> otherState)
{
  // System.out.println("BTR.seekExact termState target=" +
  // target.utf8ToString() + " " + target + " this=" + this);
  assert(otherState != nullptr &&
         std::dynamic_pointer_cast<BlockTermState>(otherState) != nullptr);
  assert(!doOrd || (std::static_pointer_cast<BlockTermState>(otherState))->ord <
                       outerInstance->numTerms);
  state->copyFrom(otherState);
  seekPending = true;
  indexIsCurrent = false;
  term_->copyBytes(target);
}

shared_ptr<TermState>
BlockTermsReader::FieldReader::SegmentTermsEnum::termState() 
{
  // System.out.println("BTR.termState this=" + this);
  decodeMetaData();
  shared_ptr<TermState> ts = state->clone();
  // System.out.println("  return ts=" + ts);
  return ts;
}

void BlockTermsReader::FieldReader::SegmentTermsEnum::seekExact(
    int64_t ord) 
{
  // System.out.println("BTR.seek by ord ord=" + ord);
  if (indexEnum == nullptr) {
    throw make_shared<IllegalStateException>(L"terms index was not loaded");
  }

  assert(ord < outerInstance->numTerms);

  // TODO: if ord is in same terms block and
  // after current ord, we should avoid this seek just
  // like we do in the seek(BytesRef) case
  in_->seek(indexEnum->seek(ord));
  bool result = nextBlock();

  // Block must exist since ord < numTerms:
  assert(result);

  indexIsCurrent = true;
  didIndexNext = false;
  seekPending = false;

  state->ord = indexEnum->ord() - 1;
  assert((state->ord >= -1, L"ord=" + to_wstring(state->ord)));
  term_->copyBytes(indexEnum->term());

  // Now, scan:
  int left = static_cast<int>(ord - state->ord);
  while (left > 0) {
    shared_ptr<BytesRef> *const term = _next();
    assert(term != nullptr);
    left--;
    assert(indexIsCurrent);
  }
}

int64_t BlockTermsReader::FieldReader::SegmentTermsEnum::ord()
{
  if (!doOrd) {
    throw make_shared<UnsupportedOperationException>();
  }
  return state->ord;
}

bool BlockTermsReader::FieldReader::SegmentTermsEnum::nextBlock() throw(
    IOException)
{

  // TODO: we still lazy-decode the byte[] for each
  // term (the suffix), but, if we decoded
  // all N terms up front then seeking could do a fast
  // bsearch w/in the block...

  // System.out.println("BTR.nextBlock() fp=" + in.getFilePointer() + " this=" +
  // this);
  state->blockFilePointer = in_->getFilePointer();
  blockTermCount = in_->readVInt();
  // System.out.println("  blockTermCount=" + blockTermCount);
  if (blockTermCount == 0) {
    return false;
  }
  termBlockPrefix = in_->readVInt();

  // term suffixes:
  int len = in_->readVInt();
  if (termSuffixes.size() < len) {
    termSuffixes = std::deque<char>(ArrayUtil::oversize(len, 1));
  }
  // System.out.println("  termSuffixes len=" + len);
  in_->readBytes(termSuffixes, 0, len);
  termSuffixesReader->reset(termSuffixes, 0, len);

  // docFreq, totalTermFreq
  len = in_->readVInt();
  if (docFreqBytes.size() < len) {
    docFreqBytes = std::deque<char>(ArrayUtil::oversize(len, 1));
  }
  // System.out.println("  freq bytes len=" + len);
  in_->readBytes(docFreqBytes, 0, len);
  freqReader->reset(docFreqBytes, 0, len);

  // metadata
  len = in_->readVInt();
  if (bytes.empty()) {
    bytes = std::deque<char>(ArrayUtil::oversize(len, 1));
    bytesReader = make_shared<ByteArrayDataInput>();
  } else if (bytes.size() < len) {
    bytes = std::deque<char>(ArrayUtil::oversize(len, 1));
  }
  in_->readBytes(bytes, 0, len);
  bytesReader->reset(bytes, 0, len);

  metaDataUpto = 0;
  state->termBlockOrd = 0;

  indexIsCurrent = false;
  // System.out.println("  indexIsCurrent=" + indexIsCurrent);

  return true;
}

void BlockTermsReader::FieldReader::SegmentTermsEnum::decodeMetaData() throw(
    IOException)
{
  // System.out.println("BTR.decodeMetadata mdUpto=" + metaDataUpto + " vs
  // termCount=" + state.termBlockOrd + " state=" + state);
  if (!seekPending) {
    // TODO: cutover to random-access API
    // here.... really stupid that we have to decode N
    // wasted term metadata just to get to the N+1th
    // that we really need...

    // lazily catch up on metadata decode:
    constexpr int limit = state->termBlockOrd;
    bool absolute = metaDataUpto == 0;
    // TODO: better API would be "jump straight to term=N"???
    while (metaDataUpto < limit) {
      // System.out.println("  decode mdUpto=" + metaDataUpto);
      // TODO: we could make "tiers" of metadata, ie,
      // decode docFreq/totalTF but don't decode postings
      // metadata; this way caller could get
      // docFreq/totalTF w/o paying decode cost for
      // postings

      // TODO: if docFreq were bulk decoded we could
      // just skipN here:

      // docFreq, totalTermFreq
      state->docFreq = freqReader->readVInt();
      // System.out.println("    dF=" + state.docFreq);
      if (outerInstance->fieldInfo->getIndexOptions() != IndexOptions::DOCS) {
        state->totalTermFreq = state->docFreq + freqReader->readVLong();
        // System.out.println("    totTF=" + state.totalTermFreq);
      }
      // metadata
      for (int i = 0; i < longs.size(); i++) {
        longs[i] = bytesReader->readVLong();
      }
      outerInstance->outerInstance->postingsReader.decodeTerm(
          longs, bytesReader, outerInstance->fieldInfo, state, absolute);
      metaDataUpto++;
      absolute = false;
    }
  } else {
    // System.out.println("  skip! seekPending");
  }
}

int64_t BlockTermsReader::ramBytesUsed()
{
  int64_t ramBytesUsed = BASE_RAM_BYTES_USED;
  ramBytesUsed +=
      (postingsReader != nullptr) ? postingsReader->ramBytesUsed() : 0;
  ramBytesUsed += (indexReader != nullptr) ? indexReader->ramBytesUsed() : 0;
  ramBytesUsed += fields.size() * 2LL * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  for (auto reader : fields) {
    ramBytesUsed += reader->second.ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
BlockTermsReader::getChildResources()
{
  deque<std::shared_ptr<Accountable>> resources =
      deque<std::shared_ptr<Accountable>>();
  if (indexReader != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"term index", indexReader));
  }
  if (postingsReader != nullptr) {
    resources.push_back(
        Accountables::namedAccountable(L"delegate", postingsReader));
  }
  return Collections::unmodifiableList(resources);
}

wstring BlockTermsReader::toString()
{
  return getClass().getSimpleName() + L"(index=" + indexReader + L",delegate=" +
         postingsReader + L")";
}

void BlockTermsReader::checkIntegrity() 
{
  // verify terms
  CodecUtil::checksumEntireFile(in_);

  // verify postings
  postingsReader->checkIntegrity();
}
} // namespace org::apache::lucene::codecs::blockterms