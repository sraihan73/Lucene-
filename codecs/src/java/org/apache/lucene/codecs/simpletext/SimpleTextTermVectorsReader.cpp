using namespace std;

#include "SimpleTextTermVectorsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/AlreadyClosedException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/BufferedChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextTermVectorsWriter.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using namespace org::apache::lucene::codecs::simpletext;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextTermVectorsWriter.*;

SimpleTextTermVectorsReader::SimpleTextTermVectorsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  bool success = false;
  try {
    in_ = directory->openInput(
        IndexFileNames::segmentFileName(
            si->name, L"", SimpleTextTermVectorsWriter::VECTORS_EXTENSION),
        context);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        close();
      } catch (const runtime_error &t) {
      } // ensure we throw our original exception
    }
  }
  readIndex(si->maxDoc());
}

SimpleTextTermVectorsReader::SimpleTextTermVectorsReader(
    std::deque<int64_t> &offsets, shared_ptr<IndexInput> in_)
{
  this->offsets = offsets;
  this->in_ = in_;
}

void SimpleTextTermVectorsReader::readIndex(int maxDoc) 
{
  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(in_);
  offsets = std::deque<int64_t>(maxDoc);
  int upto = 0;
  while (!scratch->get().equals(SimpleTextTermVectorsWriter::END)) {
    SimpleTextUtil::readLine(input, scratch);
    if (StringHelper::startsWith(scratch->get(),
                                 SimpleTextTermVectorsWriter::DOC)) {
      offsets[upto] = input->getFilePointer();
      upto++;
    }
  }
  SimpleTextUtil::checkFooter(input);
  assert(upto == offsets.size());
}

shared_ptr<Fields> SimpleTextTermVectorsReader::get(int doc) 
{
  shared_ptr<SortedMap<wstring, std::shared_ptr<SimpleTVTerms>>> fields =
      map_obj<wstring, std::shared_ptr<SimpleTVTerms>>();
  in_->seek(offsets[doc]);
  readLine();
  assert((StringHelper::startsWith(scratch->get(),
                                   SimpleTextTermVectorsWriter::NUMFIELDS)));
  int numFields = parseIntAt(SimpleTextTermVectorsWriter::NUMFIELDS->length);
  if (numFields == 0) {
    return nullptr; // no vectors for this doc
  }
  for (int i = 0; i < numFields; i++) {
    readLine();
    assert((StringHelper::startsWith(scratch->get(),
                                     SimpleTextTermVectorsWriter::FIELD)));
    // skip fieldNumber:
    parseIntAt(SimpleTextTermVectorsWriter::FIELD->length);

    readLine();
    assert((StringHelper::startsWith(scratch->get(),
                                     SimpleTextTermVectorsWriter::FIELDNAME)));
    wstring fieldName =
        readString(SimpleTextTermVectorsWriter::FIELDNAME->length, scratch);

    readLine();
    assert((StringHelper::startsWith(
        scratch->get(), SimpleTextTermVectorsWriter::FIELDPOSITIONS)));
    bool positions = StringHelper::fromString<bool>(readString(
        SimpleTextTermVectorsWriter::FIELDPOSITIONS->length, scratch));

    readLine();
    assert((StringHelper::startsWith(
        scratch->get(), SimpleTextTermVectorsWriter::FIELDOFFSETS)));
    bool offsets = StringHelper::fromString<bool>(
        readString(SimpleTextTermVectorsWriter::FIELDOFFSETS->length, scratch));

    readLine();
    assert((StringHelper::startsWith(
        scratch->get(), SimpleTextTermVectorsWriter::FIELDPAYLOADS)));
    bool payloads = StringHelper::fromString<bool>(readString(
        SimpleTextTermVectorsWriter::FIELDPAYLOADS->length, scratch));

    readLine();
    assert((StringHelper::startsWith(
        scratch->get(), SimpleTextTermVectorsWriter::FIELDTERMCOUNT)));
    int termCount =
        parseIntAt(SimpleTextTermVectorsWriter::FIELDTERMCOUNT->length);

    shared_ptr<SimpleTVTerms> terms =
        make_shared<SimpleTVTerms>(offsets, positions, payloads);
    fields->put(fieldName, terms);

    shared_ptr<BytesRefBuilder> term = make_shared<BytesRefBuilder>();
    for (int j = 0; j < termCount; j++) {
      readLine();
      assert((StringHelper::startsWith(scratch->get(),
                                       SimpleTextTermVectorsWriter::TERMTEXT)));
      int termLength =
          scratch->length() - SimpleTextTermVectorsWriter::TERMTEXT->length;
      term->grow(termLength);
      term->setLength(termLength);
      System::arraycopy(scratch->bytes(),
                        SimpleTextTermVectorsWriter::TERMTEXT->length,
                        term->bytes(), 0, termLength);

      shared_ptr<SimpleTVPostings> postings = make_shared<SimpleTVPostings>();
      terms->terms->put(term->toBytesRef(), postings);

      readLine();
      assert((StringHelper::startsWith(scratch->get(),
                                       SimpleTextTermVectorsWriter::TERMFREQ)));
      postings->freq =
          parseIntAt(SimpleTextTermVectorsWriter::TERMFREQ->length);

      if (positions || offsets) {
        if (positions) {
          postings->positions = std::deque<int>(postings->freq);
          if (payloads) {
            postings->payloads =
                std::deque<std::shared_ptr<BytesRef>>(postings->freq);
          }
        }

        if (offsets) {
          postings->startOffsets = std::deque<int>(postings->freq);
          postings->endOffsets = std::deque<int>(postings->freq);
        }

        for (int k = 0; k < postings->freq; k++) {
          if (positions) {
            readLine();
            assert((StringHelper::startsWith(
                scratch->get(), SimpleTextTermVectorsWriter::POSITION)));
            postings->positions[k] =
                parseIntAt(SimpleTextTermVectorsWriter::POSITION->length);
            if (payloads) {
              readLine();
              assert((StringHelper::startsWith(
                  scratch->get(), SimpleTextTermVectorsWriter::PAYLOAD)));
              if (scratch->length() -
                      SimpleTextTermVectorsWriter::PAYLOAD->length ==
                  0) {
                postings->payloads[k] = nullptr;
              } else {
                std::deque<char> payloadBytes(
                    scratch->length() -
                    SimpleTextTermVectorsWriter::PAYLOAD->length);
                System::arraycopy(scratch->bytes(),
                                  SimpleTextTermVectorsWriter::PAYLOAD->length,
                                  payloadBytes, 0, payloadBytes.size());
                postings->payloads[k] = make_shared<BytesRef>(payloadBytes);
              }
            }
          }

          if (offsets) {
            readLine();
            assert((StringHelper::startsWith(
                scratch->get(), SimpleTextTermVectorsWriter::STARTOFFSET)));
            postings->startOffsets[k] =
                parseIntAt(SimpleTextTermVectorsWriter::STARTOFFSET->length);

            readLine();
            assert((StringHelper::startsWith(
                scratch->get(), SimpleTextTermVectorsWriter::ENDOFFSET)));
            postings->endOffsets[k] =
                parseIntAt(SimpleTextTermVectorsWriter::ENDOFFSET->length);
          }
        }
      }
    }
  }
  return make_shared<SimpleTVFields>(fields);
}

shared_ptr<TermVectorsReader> SimpleTextTermVectorsReader::clone()
{
  if (in_ == nullptr) {
    throw make_shared<AlreadyClosedException>(
        L"this TermVectorsReader is closed");
  }
  return make_shared<SimpleTextTermVectorsReader>(offsets, in_->clone());
}

SimpleTextTermVectorsReader::~SimpleTextTermVectorsReader()
{
  try {
    IOUtils::close({in_});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    in_.reset();
    offsets.clear();
  }
}

void SimpleTextTermVectorsReader::readLine() 
{
  SimpleTextUtil::readLine(in_, scratch);
}

int SimpleTextTermVectorsReader::parseIntAt(int offset)
{
  scratchUTF16->copyUTF8Bytes(scratch->bytes(), offset,
                              scratch->length() - offset);
  return ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
}

wstring
SimpleTextTermVectorsReader::readString(int offset,
                                        shared_ptr<BytesRefBuilder> scratch)
{
  scratchUTF16->copyUTF8Bytes(scratch->bytes(), offset,
                              scratch->length() - offset);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return scratchUTF16->toString();
}

SimpleTextTermVectorsReader::SimpleTVFields::SimpleTVFields(
    shared_ptr<SortedMap<wstring, std::shared_ptr<SimpleTVTerms>>> fields)
    : fields(fields)
{
}

shared_ptr<Iterator<wstring>>
SimpleTextTermVectorsReader::SimpleTVFields::iterator()
{
  return Collections::unmodifiableSet(fields->keySet()).begin();
}

shared_ptr<Terms> SimpleTextTermVectorsReader::SimpleTVFields::terms(
    const wstring &field) 
{
  return fields->get(field);
}

int SimpleTextTermVectorsReader::SimpleTVFields::size()
{
  return fields->size();
}

SimpleTextTermVectorsReader::SimpleTVTerms::SimpleTVTerms(bool hasOffsets,
                                                          bool hasPositions,
                                                          bool hasPayloads)
    : terms(map_obj<>()), hasOffsets(hasOffsets), hasPositions(hasPositions),
      hasPayloads(hasPayloads)
{
}

shared_ptr<TermsEnum>
SimpleTextTermVectorsReader::SimpleTVTerms::iterator() 
{
  // TODO: reuse
  return make_shared<SimpleTVTermsEnum>(terms);
}

int64_t SimpleTextTermVectorsReader::SimpleTVTerms::size() 
{
  return terms->size();
}

int64_t
SimpleTextTermVectorsReader::SimpleTVTerms::getSumTotalTermFreq() throw(
    IOException)
{
  return -1;
}

int64_t
SimpleTextTermVectorsReader::SimpleTVTerms::getSumDocFreq() 
{
  return terms->size();
}

int SimpleTextTermVectorsReader::SimpleTVTerms::getDocCount() 
{
  return 1;
}

bool SimpleTextTermVectorsReader::SimpleTVTerms::hasFreqs() { return true; }

bool SimpleTextTermVectorsReader::SimpleTVTerms::hasOffsets()
{
  return hasOffsets_;
}

bool SimpleTextTermVectorsReader::SimpleTVTerms::hasPositions()
{
  return hasPositions_;
}

bool SimpleTextTermVectorsReader::SimpleTVTerms::hasPayloads()
{
  return hasPayloads_;
}

SimpleTextTermVectorsReader::SimpleTVTermsEnum::SimpleTVTermsEnum(
    shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SimpleTVPostings>>>
        terms)
{
  this->terms = terms;
  this->iterator = terms->entrySet().begin();
}

SeekStatus SimpleTextTermVectorsReader::SimpleTVTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{
  iterator = terms->tailMap(text).entrySet().begin();
  if (!iterator->hasNext()) {
    return SeekStatus::END;
  } else {
    return next()->equals(text) ? SeekStatus::FOUND : SeekStatus::NOT_FOUND;
  }
}

void SimpleTextTermVectorsReader::SimpleTVTermsEnum::seekExact(
    int64_t ord) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef>
SimpleTextTermVectorsReader::SimpleTVTermsEnum::next() 
{
  if (!iterator->hasNext()) {
    return nullptr;
  } else {
    current = iterator->next();
    return current.getKey();
  }
}

shared_ptr<BytesRef>
SimpleTextTermVectorsReader::SimpleTVTermsEnum::term() 
{
  return current.getKey();
}

int64_t
SimpleTextTermVectorsReader::SimpleTVTermsEnum::ord() 
{
  throw make_shared<UnsupportedOperationException>();
}

int SimpleTextTermVectorsReader::SimpleTVTermsEnum::docFreq() 
{
  return 1;
}

int64_t SimpleTextTermVectorsReader::SimpleTVTermsEnum::totalTermFreq() throw(
    IOException)
{
  return current.getValue().freq;
}

shared_ptr<PostingsEnum>
SimpleTextTermVectorsReader::SimpleTVTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{

  if (PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    shared_ptr<SimpleTVPostings> postings = current.getValue();
    if (postings->positions != nullptr || postings->startOffsets != nullptr) {
      // TODO: reuse
      shared_ptr<SimpleTVPostingsEnum> e = make_shared<SimpleTVPostingsEnum>();
      e->reset(postings->positions, postings->startOffsets,
               postings->endOffsets, postings->payloads);
      return e;
    }
  }

  // TODO: reuse
  shared_ptr<SimpleTVDocsEnum> e = make_shared<SimpleTVDocsEnum>();
  e->reset(PostingsEnum::featureRequested(flags, PostingsEnum::FREQS) == false
               ? 1
               : current.getValue().freq);
  return e;
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::freq() 
{
  assert(freq_ != -1);
  return freq_;
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::nextPosition() throw(
    IOException)
{
  return -1;
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::startOffset() throw(
    IOException)
{
  return -1;
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::endOffset() throw(
    IOException)
{
  return -1;
}

shared_ptr<BytesRef>
SimpleTextTermVectorsReader::SimpleTVDocsEnum::getPayload() 
{
  return nullptr;
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::docID() { return doc; }

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::nextDoc()
{
  if (!didNext) {
    didNext = true;
    return (doc = 0);
  } else {
    return (doc = NO_MORE_DOCS);
  }
}

int SimpleTextTermVectorsReader::SimpleTVDocsEnum::advance(int target) throw(
    IOException)
{
  return slowAdvance(target);
}

void SimpleTextTermVectorsReader::SimpleTVDocsEnum::reset(int freq)
{
  this->freq_ = freq;
  this->doc = -1;
  didNext = false;
}

int64_t SimpleTextTermVectorsReader::SimpleTVDocsEnum::cost() { return 1; }

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::freq() 
{
  if (positions.size() > 0) {
    return positions.size();
  } else {
    assert(startOffsets.size() > 0);
    return startOffsets.size();
  }
}

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::docID() { return doc; }

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::nextDoc()
{
  if (!didNext) {
    didNext = true;
    return (doc = 0);
  } else {
    return (doc = NO_MORE_DOCS);
  }
}

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::advance(
    int target) 
{
  return slowAdvance(target);
}

void SimpleTextTermVectorsReader::SimpleTVPostingsEnum::reset(
    std::deque<int> &positions, std::deque<int> &startOffsets,
    std::deque<int> &endOffsets,
    std::deque<std::shared_ptr<BytesRef>> &payloads)
{
  this->positions = positions;
  this->startOffsets = startOffsets;
  this->endOffsets = endOffsets;
  this->payloads = payloads;
  this->doc = -1;
  didNext = false;
  nextPos = 0;
}

shared_ptr<BytesRef>
SimpleTextTermVectorsReader::SimpleTVPostingsEnum::getPayload()
{
  return payloads.empty() ? nullptr : payloads[nextPos - 1];
}

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::nextPosition()
{
  if (positions.size() > 0) {
    assert(nextPos < positions.size());
    return positions[nextPos++];
  } else {
    assert(nextPos < startOffsets.size());
    nextPos++;
    return -1;
  }
}

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::startOffset()
{
  if (startOffsets.empty()) {
    return -1;
  } else {
    return startOffsets[nextPos - 1];
  }
}

int SimpleTextTermVectorsReader::SimpleTVPostingsEnum::endOffset()
{
  if (endOffsets.empty()) {
    return -1;
  } else {
    return endOffsets[nextPos - 1];
  }
}

int64_t SimpleTextTermVectorsReader::SimpleTVPostingsEnum::cost()
{
  return 1;
}

int64_t SimpleTextTermVectorsReader::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(offsets);
}

wstring SimpleTextTermVectorsReader::toString()
{
  return getClass().getSimpleName();
}

void SimpleTextTermVectorsReader::checkIntegrity()  {}
} // namespace org::apache::lucene::codecs::simpletext