using namespace std;

#include "FreqProxFields.h"

namespace org::apache::lucene::index
{
using FreqProxPostingsArray = org::apache::lucene::index::
    FreqProxTermsWriterPerField::FreqProxPostingsArray;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

FreqProxFields::FreqProxFields(
    deque<std::shared_ptr<FreqProxTermsWriterPerField>> &fieldList)
{
  // NOTE: fields are already sorted by field name
  for (auto field : fieldList) {
    fields.emplace(field->fieldInfo->name, field);
  }
}

shared_ptr<Iterator<wstring>> FreqProxFields::iterator()
{
  return fields.keySet().begin();
}

shared_ptr<Terms> FreqProxFields::terms(const wstring &field) 
{
  shared_ptr<FreqProxTermsWriterPerField> perField = fields[field];
  return perField == nullptr ? nullptr : make_shared<FreqProxTerms>(perField);
}

int FreqProxFields::size()
{
  // return fields.size();
  throw make_shared<UnsupportedOperationException>();
}

FreqProxFields::FreqProxTerms::FreqProxTerms(
    shared_ptr<FreqProxTermsWriterPerField> terms)
    : terms(terms)
{
}

shared_ptr<TermsEnum> FreqProxFields::FreqProxTerms::iterator()
{
  shared_ptr<FreqProxTermsEnum> termsEnum =
      make_shared<FreqProxTermsEnum>(terms);
  termsEnum->reset();
  return termsEnum;
}

int64_t FreqProxFields::FreqProxTerms::size()
{
  // return terms.termsHashPerField.bytesHash.size();
  throw make_shared<UnsupportedOperationException>();
}

int64_t FreqProxFields::FreqProxTerms::getSumTotalTermFreq()
{
  // return terms.sumTotalTermFreq;
  throw make_shared<UnsupportedOperationException>();
}

int64_t FreqProxFields::FreqProxTerms::getSumDocFreq()
{
  // return terms.sumDocFreq;
  throw make_shared<UnsupportedOperationException>();
}

int FreqProxFields::FreqProxTerms::getDocCount()
{
  // return terms.docCount;
  throw make_shared<UnsupportedOperationException>();
}

bool FreqProxFields::FreqProxTerms::hasFreqs()
{
  return terms->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS) >= 0;
}

bool FreqProxFields::FreqProxTerms::hasOffsets()
{
  // NOTE: the in-memory buffer may have indexed offsets
  // because that's what FieldInfo said when we started,
  // but during indexing this may have been downgraded:
  return terms->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool FreqProxFields::FreqProxTerms::hasPositions()
{
  // NOTE: the in-memory buffer may have indexed positions
  // because that's what FieldInfo said when we started,
  // but during indexing this may have been downgraded:
  return terms->fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool FreqProxFields::FreqProxTerms::hasPayloads() { return terms->sawPayloads; }

FreqProxFields::FreqProxTermsEnum::FreqProxTermsEnum(
    shared_ptr<FreqProxTermsWriterPerField> terms)
    : terms(terms), sortedTermIDs(terms->sortedTermIDs),
      postingsArray(std::static_pointer_cast<FreqProxPostingsArray>(
          terms->postingsArray)),
      numTerms(terms->bytesHash->size())
{
  assert(sortedTermIDs.size() > 0);
}

void FreqProxFields::FreqProxTermsEnum::reset() { ord_ = -1; }

SeekStatus
FreqProxFields::FreqProxTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  // TODO: we could instead keep the BytesRefHash
  // intact so this is a hash lookup

  // binary search:
  int lo = 0;
  int hi = numTerms - 1;
  while (hi >= lo) {
    int mid = static_cast<int>(static_cast<unsigned int>((lo + hi)) >> 1);
    int textStart = postingsArray->textStarts[sortedTermIDs[mid]];
    terms->bytePool->setBytesRef(scratch, textStart);
    int cmp = scratch->compareTo(text);
    if (cmp < 0) {
      lo = mid + 1;
    } else if (cmp > 0) {
      hi = mid - 1;
    } else {
      // found:
      ord_ = mid;
      assert(term()->compareTo(text) == 0);
      return SeekStatus::FOUND;
    }
  }

  // not found:
  ord_ = lo;
  if (ord_ >= numTerms) {
    return SeekStatus::END;
  } else {
    int textStart = postingsArray->textStarts[sortedTermIDs[ord_]];
    terms->bytePool->setBytesRef(scratch, textStart);
    assert(term()->compareTo(text) > 0);
    return SeekStatus::NOT_FOUND;
  }
}

void FreqProxFields::FreqProxTermsEnum::seekExact(int64_t ord)
{
  this->ord_ = static_cast<int>(ord);
  int textStart = postingsArray->textStarts[sortedTermIDs[this->ord_]];
  terms->bytePool->setBytesRef(scratch, textStart);
}

shared_ptr<BytesRef> FreqProxFields::FreqProxTermsEnum::next()
{
  ord_++;
  if (ord_ >= numTerms) {
    return nullptr;
  } else {
    int textStart = postingsArray->textStarts[sortedTermIDs[ord_]];
    terms->bytePool->setBytesRef(scratch, textStart);
    return scratch;
  }
}

shared_ptr<BytesRef> FreqProxFields::FreqProxTermsEnum::term()
{
  return scratch;
}

int64_t FreqProxFields::FreqProxTermsEnum::ord() { return ord_; }

int FreqProxFields::FreqProxTermsEnum::docFreq()
{
  // We do not store this per-term, and we cannot
  // implement this at merge time w/o an added pass
  // through the postings:
  throw make_shared<UnsupportedOperationException>();
}

int64_t FreqProxFields::FreqProxTermsEnum::totalTermFreq()
{
  // We do not store this per-term, and we cannot
  // implement this at merge time w/o an added pass
  // through the postings:
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
FreqProxFields::FreqProxTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                            int flags)
{
  if (PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    shared_ptr<FreqProxPostingsEnum> posEnum;

    if (!terms->hasProx) {
      // Caller wants positions but we didn't index them;
      // don't lie:
      throw invalid_argument(L"did not index positions");
    }

    if (!terms->hasOffsets &&
        PostingsEnum::featureRequested(flags, PostingsEnum::OFFSETS)) {
      // Caller wants offsets but we didn't index them;
      // don't lie:
      throw invalid_argument(L"did not index offsets");
    }

    if (std::dynamic_pointer_cast<FreqProxPostingsEnum>(reuse) != nullptr) {
      posEnum = std::static_pointer_cast<FreqProxPostingsEnum>(reuse);
      if (posEnum->postingsArray != postingsArray) {
        posEnum = make_shared<FreqProxPostingsEnum>(terms, postingsArray);
      }
    } else {
      posEnum = make_shared<FreqProxPostingsEnum>(terms, postingsArray);
    }
    posEnum->reset(sortedTermIDs[ord_]);
    return posEnum;
  }

  shared_ptr<FreqProxDocsEnum> docsEnum;

  if (!terms->hasFreq &&
      PostingsEnum::featureRequested(flags, PostingsEnum::FREQS)) {
    // Caller wants freqs but we didn't index them;
    // don't lie:
    throw invalid_argument(L"did not index freq");
  }

  if (std::dynamic_pointer_cast<FreqProxDocsEnum>(reuse) != nullptr) {
    docsEnum = std::static_pointer_cast<FreqProxDocsEnum>(reuse);
    if (docsEnum->postingsArray != postingsArray) {
      docsEnum = make_shared<FreqProxDocsEnum>(terms, postingsArray);
    }
  } else {
    docsEnum = make_shared<FreqProxDocsEnum>(terms, postingsArray);
  }
  docsEnum->reset(sortedTermIDs[ord_]);
  return docsEnum;
}

shared_ptr<TermState>
FreqProxFields::FreqProxTermsEnum::termState() 
{
  return make_shared<TermStateAnonymousInnerClass>(shared_from_this());
}

FreqProxFields::FreqProxTermsEnum::TermStateAnonymousInnerClass::
    TermStateAnonymousInnerClass(shared_ptr<FreqProxTermsEnum> outerInstance)
{
  this->outerInstance = outerInstance;
}

void FreqProxFields::FreqProxTermsEnum::TermStateAnonymousInnerClass::copyFrom(
    shared_ptr<TermState> other)
{
  throw make_shared<UnsupportedOperationException>();
}

FreqProxFields::FreqProxDocsEnum::FreqProxDocsEnum(
    shared_ptr<FreqProxTermsWriterPerField> terms,
    shared_ptr<FreqProxPostingsArray> postingsArray)
    : terms(terms), postingsArray(postingsArray), readTermFreq(terms->hasFreq)
{
}

void FreqProxFields::FreqProxDocsEnum::reset(int termID)
{
  this->termID = termID;
  terms->initReader(reader, termID, 0);
  ended = false;
  docID_ = -1;
}

int FreqProxFields::FreqProxDocsEnum::docID() { return docID_; }

int FreqProxFields::FreqProxDocsEnum::freq()
{
  // Don't lie here ... don't want codecs writings lots
  // of wasted 1s into the index:
  if (!readTermFreq) {
    throw make_shared<IllegalStateException>(L"freq was not indexed");
  } else {
    return freq_;
  }
}

int FreqProxFields::FreqProxDocsEnum::nextPosition() 
{
  return -1;
}

int FreqProxFields::FreqProxDocsEnum::startOffset() 
{
  return -1;
}

int FreqProxFields::FreqProxDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
FreqProxFields::FreqProxDocsEnum::getPayload() 
{
  return nullptr;
}

int FreqProxFields::FreqProxDocsEnum::nextDoc() 
{
  if (docID_ == -1) {
    docID_ = 0;
  }
  if (reader->eof()) {
    if (ended) {
      return NO_MORE_DOCS;
    } else {
      ended = true;
      docID_ = postingsArray->lastDocIDs[termID];
      if (readTermFreq) {
        freq_ = postingsArray->termFreqs[termID];
      }
    }
  } else {
    int code = reader->readVInt();
    if (!readTermFreq) {
      docID_ += code;
    } else {
      docID_ += static_cast<int>(static_cast<unsigned int>(code) >> 1);
      if ((code & 1) != 0) {
        freq_ = 1;
      } else {
        freq_ = reader->readVInt();
      }
    }

    assert(docID_ != postingsArray->lastDocIDs[termID]);
  }

  return docID_;
}

int FreqProxFields::FreqProxDocsEnum::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t FreqProxFields::FreqProxDocsEnum::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

FreqProxFields::FreqProxPostingsEnum::FreqProxPostingsEnum(
    shared_ptr<FreqProxTermsWriterPerField> terms,
    shared_ptr<FreqProxPostingsArray> postingsArray)
    : terms(terms), postingsArray(postingsArray), readOffsets(terms->hasOffsets)
{
  assert(terms->hasProx);
  assert(terms->hasFreq);
}

void FreqProxFields::FreqProxPostingsEnum::reset(int termID)
{
  this->termID = termID;
  terms->initReader(reader, termID, 0);
  terms->initReader(posReader, termID, 1);
  ended = false;
  docID_ = -1;
  posLeft = 0;
}

int FreqProxFields::FreqProxPostingsEnum::docID() { return docID_; }

int FreqProxFields::FreqProxPostingsEnum::freq() { return freq_; }

int FreqProxFields::FreqProxPostingsEnum::nextDoc() 
{
  if (docID_ == -1) {
    docID_ = 0;
  }
  while (posLeft != 0) {
    nextPosition();
  }

  if (reader->eof()) {
    if (ended) {
      return NO_MORE_DOCS;
    } else {
      ended = true;
      docID_ = postingsArray->lastDocIDs[termID];
      freq_ = postingsArray->termFreqs[termID];
    }
  } else {
    int code = reader->readVInt();
    docID_ += static_cast<int>(static_cast<unsigned int>(code) >> 1);
    if ((code & 1) != 0) {
      freq_ = 1;
    } else {
      freq_ = reader->readVInt();
    }

    assert(docID_ != postingsArray->lastDocIDs[termID]);
  }

  posLeft = freq_;
  pos = 0;
  startOffset_ = 0;
  return docID_;
}

int FreqProxFields::FreqProxPostingsEnum::advance(int target)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t FreqProxFields::FreqProxPostingsEnum::cost()
{
  throw make_shared<UnsupportedOperationException>();
}

int FreqProxFields::FreqProxPostingsEnum::nextPosition() 
{
  assert(posLeft > 0);
  posLeft--;
  int code = posReader->readVInt();
  pos += static_cast<int>(static_cast<unsigned int>(code) >> 1);
  if ((code & 1) != 0) {
    hasPayload = true;
    // has a payload
    payload->setLength(posReader->readVInt());
    payload->grow(payload->length());
    posReader->readBytes(payload->bytes(), 0, payload->length());
  } else {
    hasPayload = false;
  }

  if (readOffsets) {
    startOffset_ += posReader->readVInt();
    endOffset_ = startOffset_ + posReader->readVInt();
  }

  return pos;
}

int FreqProxFields::FreqProxPostingsEnum::startOffset()
{
  if (!readOffsets) {
    throw make_shared<IllegalStateException>(L"offsets were not indexed");
  }
  return startOffset_;
}

int FreqProxFields::FreqProxPostingsEnum::endOffset()
{
  if (!readOffsets) {
    throw make_shared<IllegalStateException>(L"offsets were not indexed");
  }
  return endOffset_;
}

shared_ptr<BytesRef> FreqProxFields::FreqProxPostingsEnum::getPayload()
{
  if (hasPayload) {
    return payload->get();
  } else {
    return nullptr;
  }
}
} // namespace org::apache::lucene::index