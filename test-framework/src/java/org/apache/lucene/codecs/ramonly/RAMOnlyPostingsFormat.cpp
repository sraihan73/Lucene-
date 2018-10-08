using namespace std;

#include "RAMOnlyPostingsFormat.h"

namespace org::apache::lucene::codecs::ramonly
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TermStats = org::apache::lucene::codecs::TermStats;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

RAMOnlyPostingsFormat::RAMOnlyPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(L"RAMOnly")
{
}

shared_ptr<Terms>
RAMOnlyPostingsFormat::RAMPostings::terms(const wstring &field)
{
  return fieldToTerms[field];
}

int RAMOnlyPostingsFormat::RAMPostings::size() { return fieldToTerms.size(); }

shared_ptr<Iterator<wstring>> RAMOnlyPostingsFormat::RAMPostings::iterator()
{
  return Collections::unmodifiableSet(fieldToTerms.keySet()).begin();
}

RAMOnlyPostingsFormat::RAMPostings::~RAMPostings() {}

int64_t RAMOnlyPostingsFormat::RAMPostings::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto field : fieldToTerms) {
    sizeInBytes += field->second.ramBytesUsed();
  }
  return sizeInBytes;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
RAMOnlyPostingsFormat::RAMPostings::getChildResources()
{
  return Accountables::namedAccountables(L"field", fieldToTerms);
}

void RAMOnlyPostingsFormat::RAMPostings::checkIntegrity()  {}

RAMOnlyPostingsFormat::RAMField::RAMField(const wstring &field,
                                          shared_ptr<FieldInfo> info)
    : field(field), info(info)
{
}

int64_t RAMOnlyPostingsFormat::RAMField::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto term : termToDocs) {
    sizeInBytes += term->second.ramBytesUsed();
  }
  return sizeInBytes;
}

int64_t RAMOnlyPostingsFormat::RAMField::size() { return termToDocs->size(); }

int64_t RAMOnlyPostingsFormat::RAMField::getSumTotalTermFreq()
{
  return sumTotalTermFreq;
}

int64_t RAMOnlyPostingsFormat::RAMField::getSumDocFreq() 
{
  return sumDocFreq;
}

int RAMOnlyPostingsFormat::RAMField::getDocCount() 
{
  return docCount;
}

shared_ptr<TermsEnum> RAMOnlyPostingsFormat::RAMField::iterator()
{
  return make_shared<RAMTermsEnum>(RAMOnlyPostingsFormat::RAMField::this);
}

bool RAMOnlyPostingsFormat::RAMField::hasFreqs()
{
  return info->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >= 0;
}

bool RAMOnlyPostingsFormat::RAMField::hasOffsets()
{
  return info->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool RAMOnlyPostingsFormat::RAMField::hasPositions()
{
  return info->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool RAMOnlyPostingsFormat::RAMField::hasPayloads()
{
  return info->hasPayloads();
}

RAMOnlyPostingsFormat::RAMTerm::RAMTerm(const wstring &term) : term(term) {}

int64_t RAMOnlyPostingsFormat::RAMTerm::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  for (auto rDoc : docs) {
    sizeInBytes += rDoc->ramBytesUsed();
  }
  return sizeInBytes;
}

RAMOnlyPostingsFormat::RAMDoc::RAMDoc(int docID, int freq)
    : docID(docID), positions(std::deque<int>(freq))
{
}

int64_t RAMOnlyPostingsFormat::RAMDoc::ramBytesUsed()
{
  int64_t sizeInBytes = 0;
  sizeInBytes +=
      (positions.size() > 0) ? RamUsageEstimator::sizeOf(positions) : 0;

  if (payloads.size() > 0) {
    for (auto payload : payloads) {
      sizeInBytes +=
          (payload.size() > 0) ? RamUsageEstimator::sizeOf(payload) : 0;
    }
  }
  return sizeInBytes;
}

RAMOnlyPostingsFormat::RAMFieldsConsumer::RAMFieldsConsumer(
    shared_ptr<SegmentWriteState> writeState, shared_ptr<RAMPostings> postings)
    : postings(postings), state(writeState)
{
}

void RAMOnlyPostingsFormat::RAMFieldsConsumer::write(
    shared_ptr<Fields> fields) 
{
  for (auto field : fields) {

    shared_ptr<Terms> terms = fields->terms(field);
    if (terms == nullptr) {
      continue;
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();

    shared_ptr<FieldInfo> fieldInfo = state->fieldInfos->fieldInfo(field);
    if (fieldInfo->getIndexOptions().compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0) {
      throw make_shared<UnsupportedOperationException>(
          L"this codec cannot index offsets");
    }

    shared_ptr<RAMField> ramField = make_shared<RAMField>(field, fieldInfo);
    postings->fieldToTerms.emplace(field, ramField);
    termsConsumer->reset(ramField);

    shared_ptr<FixedBitSet> docsSeen =
        make_shared<FixedBitSet>(state->segmentInfo->maxDoc());
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
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
        enumFlags = 0;
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
      shared_ptr<RAMPostingsWriterImpl> postingsWriter =
          termsConsumer->startTerm(term);
      postingsEnum = termsEnum->postings(postingsEnum, enumFlags);

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

        postingsWriter->startDoc(docID, freq);
        if (writePositions) {
          for (int i = 0; i < freq; i++) {
            int pos = postingsEnum->nextPosition();
            shared_ptr<BytesRef> payload =
                writePayloads ? postingsEnum->getPayload() : nullptr;
            int startOffset;
            int endOffset;
            if (writeOffsets) {
              startOffset = postingsEnum->startOffset();
              endOffset = postingsEnum->endOffset();
            } else {
              startOffset = -1;
              endOffset = -1;
            }
            postingsWriter->addPosition(pos, payload, startOffset, endOffset);
          }
        }

        postingsWriter->finishDoc();
      }
      termsConsumer->finishTerm(term,
                                make_shared<TermStats>(docFreq, totalTermFreq));
      sumDocFreq += docFreq;
      sumTotalTermFreq += totalTermFreq;
    }

    termsConsumer->finish(sumTotalTermFreq, sumDocFreq,
                          docsSeen->cardinality());
  }
}

RAMOnlyPostingsFormat::RAMFieldsConsumer::~RAMFieldsConsumer() {}

void RAMOnlyPostingsFormat::RAMTermsConsumer::reset(shared_ptr<RAMField> field)
{
  this->field = field;
}

shared_ptr<RAMPostingsWriterImpl>
RAMOnlyPostingsFormat::RAMTermsConsumer::startTerm(shared_ptr<BytesRef> text)
{
  const wstring term = text->utf8ToString();
  current = make_shared<RAMTerm>(term);
  postingsWriter->reset(current);
  return postingsWriter;
}

void RAMOnlyPostingsFormat::RAMTermsConsumer::finishTerm(
    shared_ptr<BytesRef> text, shared_ptr<TermStats> stats)
{
  assert(stats->docFreq > 0);
  assert(stats->docFreq == current->docs.size());
  current->totalTermFreq = stats->totalTermFreq;
  field->termToDocs->put(current->term, current);
}

void RAMOnlyPostingsFormat::RAMTermsConsumer::finish(int64_t sumTotalTermFreq,
                                                     int64_t sumDocFreq,
                                                     int docCount)
{
  field->sumTotalTermFreq = sumTotalTermFreq;
  field->sumDocFreq = sumDocFreq;
  field->docCount = docCount;
}

void RAMOnlyPostingsFormat::RAMPostingsWriterImpl::reset(
    shared_ptr<RAMTerm> term)
{
  this->term = term;
}

void RAMOnlyPostingsFormat::RAMPostingsWriterImpl::startDoc(int docID, int freq)
{
  current = make_shared<RAMDoc>(docID, freq);
  term->docs.push_back(current);
  posUpto = 0;
}

void RAMOnlyPostingsFormat::RAMPostingsWriterImpl::addPosition(
    int position, shared_ptr<BytesRef> payload, int startOffset, int endOffset)
{
  assert(startOffset == -1);
  assert(endOffset == -1);
  current->positions[posUpto] = position;
  if (payload != nullptr && payload->length > 0) {
    if (current->payloads.empty()) {
      current->payloads =
          std::deque<std::deque<char>>(current->positions.size());
    }
    std::deque<char> bytes = current->payloads[posUpto] =
        std::deque<char>(payload->length);
    System::arraycopy(payload->bytes, payload->offset, bytes, 0,
                      payload->length);
  }
  posUpto++;
}

void RAMOnlyPostingsFormat::RAMPostingsWriterImpl::finishDoc()
{
  assert(posUpto == current->positions.size());
}

RAMOnlyPostingsFormat::RAMTermsEnum::RAMTermsEnum(shared_ptr<RAMField> field)
    : ramField(field)
{
}

shared_ptr<BytesRef> RAMOnlyPostingsFormat::RAMTermsEnum::next()
{
  if (it == nullptr) {
    if (current == L"") {
      it = ramField->termToDocs->keySet().begin();
    } else {
      it = ramField->termToDocs->tailMap(current).keySet().begin();
    }
  }
  if (it->hasNext()) {
    current = it->next();
    return make_shared<BytesRef>(current);
  } else {
    return nullptr;
  }
}

SeekStatus
RAMOnlyPostingsFormat::RAMTermsEnum::seekCeil(shared_ptr<BytesRef> term)
{
  current = term->utf8ToString();
  it.reset();
  if (ramField->termToDocs->containsKey(current)) {
    return SeekStatus::FOUND;
  } else {
    if (current.compare(ramField->termToDocs->lastKey()) > 0) {
      return SeekStatus::END;
    } else {
      return SeekStatus::NOT_FOUND;
    }
  }
}

void RAMOnlyPostingsFormat::RAMTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t RAMOnlyPostingsFormat::RAMTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef> RAMOnlyPostingsFormat::RAMTermsEnum::term()
{
  // TODO: reuse BytesRef
  return make_shared<BytesRef>(current);
}

int RAMOnlyPostingsFormat::RAMTermsEnum::docFreq()
{
  return ramField->termToDocs->get(current).docs->size();
}

int64_t RAMOnlyPostingsFormat::RAMTermsEnum::totalTermFreq()
{
  return ramField->termToDocs->get(current).totalTermFreq;
}

shared_ptr<PostingsEnum>
RAMOnlyPostingsFormat::RAMTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                              int flags)
{
  return make_shared<RAMDocsEnum>(ramField->termToDocs->get(current));
}

RAMOnlyPostingsFormat::RAMDocsEnum::RAMDocsEnum(shared_ptr<RAMTerm> ramTerm)
    : ramTerm(ramTerm)
{
}

int RAMOnlyPostingsFormat::RAMDocsEnum::advance(int targetDocID) throw(
    IOException)
{
  return slowAdvance(targetDocID);
}

int RAMOnlyPostingsFormat::RAMDocsEnum::nextDoc()
{
  upto++;
  if (upto < ramTerm->docs.size()) {
    current = ramTerm->docs[upto];
    posUpto = 0;
    return current->docID;
  } else {
    return NO_MORE_DOCS;
  }
}

int RAMOnlyPostingsFormat::RAMDocsEnum::freq() 
{
  return current->positions.size();
}

int RAMOnlyPostingsFormat::RAMDocsEnum::docID() { return current->docID; }

int RAMOnlyPostingsFormat::RAMDocsEnum::nextPosition()
{
  assert(posUpto < current->positions.size());
  return current->positions[posUpto++];
}

int RAMOnlyPostingsFormat::RAMDocsEnum::startOffset() { return -1; }

int RAMOnlyPostingsFormat::RAMDocsEnum::endOffset() { return -1; }

shared_ptr<BytesRef> RAMOnlyPostingsFormat::RAMDocsEnum::getPayload()
{
  if (current->payloads.size() > 0 &&
      current->payloads[posUpto - 1] != nullptr) {
    return make_shared<BytesRef>(current->payloads[posUpto - 1]);
  } else {
    return nullptr;
  }
}

int64_t RAMOnlyPostingsFormat::RAMDocsEnum::cost()
{
  return ramTerm->docs.size();
}

const wstring RAMOnlyPostingsFormat::ID_EXTENSION = L"id";

shared_ptr<FieldsConsumer> RAMOnlyPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> writeState) 
{
  constexpr int id = nextID->getAndIncrement();

  // TODO -- ok to do this up front instead of
  // on close....?  should be ok?
  // Write our ID:
  const wstring idFileName = IndexFileNames::segmentFileName(
      writeState->segmentInfo->name, writeState->segmentSuffix, ID_EXTENSION);
  shared_ptr<IndexOutput> out =
      writeState->directory->createOutput(idFileName, writeState->context);
  bool success = false;
  try {
    CodecUtil::writeHeader(out, RAM_ONLY_NAME, VERSION_LATEST);
    out->writeVInt(id);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({out});
    } else {
      IOUtils::close({out});
    }
  }

  shared_ptr<RAMPostings> *const postings = make_shared<RAMPostings>();
  shared_ptr<RAMFieldsConsumer> *const consumer =
      make_shared<RAMFieldsConsumer>(writeState, postings);

  {
    lock_guard<mutex> lock(state);
    state.emplace(id, postings);
  }
  return consumer;
}

shared_ptr<FieldsProducer> RAMOnlyPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> readState) 
{

  // Load our ID:
  const wstring idFileName = IndexFileNames::segmentFileName(
      readState->segmentInfo->name, readState->segmentSuffix, ID_EXTENSION);
  shared_ptr<IndexInput> in_ =
      readState->directory->openInput(idFileName, readState->context);
  bool success = false;
  constexpr int id;
  try {
    CodecUtil::checkHeader(in_, RAM_ONLY_NAME, VERSION_START, VERSION_LATEST);
    id = in_->readVInt();
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({in_});
    } else {
      IOUtils::close({in_});
    }
  }

  {
    lock_guard<mutex> lock(state);
    return state[id];
  }
}
} // namespace org::apache::lucene::codecs::ramonly