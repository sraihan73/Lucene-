using namespace std;

#include "SortingLeafReader.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Sort = org::apache::lucene::search::Sort;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMInputStream = org::apache::lucene::store::RAMInputStream;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BitSet = org::apache::lucene::util::BitSet;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using TimSorter = org::apache::lucene::util::TimSorter;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

SortingLeafReader::SortingFields::SortingFields(
    shared_ptr<Fields> in_, shared_ptr<FieldInfos> infos,
    shared_ptr<Sorter::DocMap> docMap)
    : FilterFields(in_), docMap(docMap), infos(infos)
{
}

shared_ptr<Terms>
SortingLeafReader::SortingFields::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = in_->terms(field);
  if (terms == nullptr) {
    return nullptr;
  } else {
    return make_shared<SortingTerms>(
        terms, infos->fieldInfo(field)->getIndexOptions(), docMap);
  }
}

SortingLeafReader::SortingTerms::SortingTerms(shared_ptr<Terms> in_,
                                              IndexOptions indexOptions,
                                              shared_ptr<Sorter::DocMap> docMap)
    : FilterTerms(in_), docMap(docMap), indexOptions(indexOptions)
{
}

shared_ptr<TermsEnum>
SortingLeafReader::SortingTerms::iterator() 
{
  return make_shared<SortingTermsEnum>(in_->begin(), docMap, indexOptions,
                                       hasPositions());
}

shared_ptr<TermsEnum> SortingLeafReader::SortingTerms::intersect(
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
{
  return make_shared<SortingTermsEnum>(in_->intersect(compiled, startTerm),
                                       docMap, indexOptions, hasPositions());
}

SortingLeafReader::SortingTermsEnum::SortingTermsEnum(
    shared_ptr<TermsEnum> in_, shared_ptr<Sorter::DocMap> docMap,
    IndexOptions indexOptions, bool hasPositions)
    : FilterTermsEnum(in_), docMap(docMap), indexOptions(indexOptions),
      hasPositions(hasPositions)
{
}

shared_ptr<PostingsEnum> SortingLeafReader::SortingTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int const flags) 
{

  if (hasPositions &&
      PostingsEnum::featureRequested(flags, PostingsEnum::POSITIONS)) {
    shared_ptr<PostingsEnum> *const inReuse;
    shared_ptr<SortingPostingsEnum> *const wrapReuse;
    if (reuse != nullptr &&
        std::dynamic_pointer_cast<SortingPostingsEnum>(reuse) != nullptr) {
      // if we're asked to reuse the given DocsEnum and it is Sorting, return
      // the wrapped one, since some Codecs expect it.
      wrapReuse = std::static_pointer_cast<SortingPostingsEnum>(reuse);
      inReuse = wrapReuse->getWrapped();
    } else {
      wrapReuse.reset();
      inReuse = reuse;
    }

    shared_ptr<PostingsEnum> *const inDocsAndPositions =
        in_->postings(inReuse, flags);
    // we ignore the fact that offsets may be stored but not asked for,
    // since this code is expected to be used during addIndexes which will
    // ask for everything. if that assumption changes in the future, we can
    // factor in whether 'flags' says offsets are not required.
    constexpr bool storeOffsets =
        indexOptions.compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
    return make_shared<SortingPostingsEnum>(
        docMap->size(), wrapReuse, inDocsAndPositions, docMap, storeOffsets);
  }

  shared_ptr<PostingsEnum> *const inReuse;
  shared_ptr<SortingDocsEnum> *const wrapReuse;
  if (reuse != nullptr &&
      std::dynamic_pointer_cast<SortingDocsEnum>(reuse) != nullptr) {
    // if we're asked to reuse the given DocsEnum and it is Sorting, return
    // the wrapped one, since some Codecs expect it.
    wrapReuse = std::static_pointer_cast<SortingDocsEnum>(reuse);
    inReuse = wrapReuse->getWrapped();
  } else {
    wrapReuse.reset();
    inReuse = reuse;
  }

  shared_ptr<PostingsEnum> *const inDocs = in_->postings(inReuse, flags);
  constexpr bool withFreqs =
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS) >= 0 &&
      PostingsEnum::featureRequested(flags, PostingsEnum::FREQS);
  return make_shared<SortingDocsEnum>(docMap->size(), wrapReuse, inDocs,
                                      withFreqs, docMap);
}

SortingLeafReader::SortingBinaryDocValues::SortingBinaryDocValues(
    shared_ptr<CachedBinaryDVs> dvs)
    : dvs(dvs)
{
}

int SortingLeafReader::SortingBinaryDocValues::nextDoc()
{
  if (docID_ + 1 == dvs->docsWithField->length()) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = dvs->docsWithField.nextSetBit(docID_ + 1);
  }

  return docID_;
}

int SortingLeafReader::SortingBinaryDocValues::docID() { return docID_; }

int SortingLeafReader::SortingBinaryDocValues::advance(int target)
{
  docID_ = dvs->docsWithField.nextSetBit(target);
  return docID_;
}

bool SortingLeafReader::SortingBinaryDocValues::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  return dvs->docsWithField->get(target);
}

shared_ptr<BytesRef> SortingLeafReader::SortingBinaryDocValues::binaryValue()
{
  return dvs->values[docID_];
}

int64_t SortingLeafReader::SortingBinaryDocValues::cost()
{
  return dvs->docsWithField.cardinality();
}

SortingLeafReader::CachedNumericDVs::CachedNumericDVs(
    std::deque<int64_t> &values, shared_ptr<BitSet> docsWithField)
    : values(values), docsWithField(docsWithField)
{
}

SortingLeafReader::CachedBinaryDVs::CachedBinaryDVs(
    std::deque<std::shared_ptr<BytesRef>> &values,
    shared_ptr<BitSet> docsWithField)
    : values(values), docsWithField(docsWithField)
{
}

SortingLeafReader::SortingNumericDocValues::SortingNumericDocValues(
    shared_ptr<CachedNumericDVs> dvs)
    : dvs(dvs)
{
}

int SortingLeafReader::SortingNumericDocValues::docID() { return docID_; }

int SortingLeafReader::SortingNumericDocValues::nextDoc()
{
  if (docID_ + 1 == dvs->docsWithField->length()) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = dvs->docsWithField.nextSetBit(docID_ + 1);
  }

  return docID_;
}

int SortingLeafReader::SortingNumericDocValues::advance(int target)
{
  docID_ = dvs->docsWithField.nextSetBit(target);
  return docID_;
}

bool SortingLeafReader::SortingNumericDocValues::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  return dvs->docsWithField->get(target);
}

int64_t SortingLeafReader::SortingNumericDocValues::longValue()
{
  return dvs->values[docID_];
}

int64_t SortingLeafReader::SortingNumericDocValues::cost()
{
  return dvs->docsWithField.cardinality();
}

SortingLeafReader::SortingBits::SortingBits(shared_ptr<Bits> in_,
                                            shared_ptr<Sorter::DocMap> docMap)
    : in_(in_), docMap(docMap)
{
}

bool SortingLeafReader::SortingBits::get(int index)
{
  return in_->get(docMap->newToOld(index));
}

int SortingLeafReader::SortingBits::length() { return in_->length(); }

SortingLeafReader::SortingPointValues::SortingPointValues(
    shared_ptr<PointValues> in_, shared_ptr<Sorter::DocMap> docMap)
    : in_(in_), docMap(docMap)
{
}

void SortingLeafReader::SortingPointValues::intersect(
    shared_ptr<IntersectVisitor> visitor) 
{
  in_->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
      shared_from_this(), visitor));
}

SortingLeafReader::SortingPointValues::IntersectVisitorAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass(
        shared_ptr<SortingPointValues> outerInstance,
        shared_ptr<org::apache::lucene::index::PointValues::IntersectVisitor>
            visitor)
{
  this->outerInstance = outerInstance;
  this->visitor = visitor;
}

void SortingLeafReader::SortingPointValues::
    IntersectVisitorAnonymousInnerClass::visit(int docID) 
{
  visitor->visit(outerInstance->docMap->oldToNew(docID));
}

void SortingLeafReader::SortingPointValues::
    IntersectVisitorAnonymousInnerClass::visit(
        int docID, std::deque<char> &packedValue) 
{
  visitor->visit(outerInstance->docMap->oldToNew(docID), packedValue);
}

Relation
SortingLeafReader::SortingPointValues::IntersectVisitorAnonymousInnerClass::
    compare(std::deque<char> &minPackedValue,
            std::deque<char> &maxPackedValue)
{
  return visitor->compare(minPackedValue, maxPackedValue);
}

int64_t SortingLeafReader::SortingPointValues::estimatePointCount(
    shared_ptr<IntersectVisitor> visitor)
{
  return in_->estimatePointCount(visitor);
}

std::deque<char>
SortingLeafReader::SortingPointValues::getMinPackedValue() 
{
  return in_->getMinPackedValue();
}

std::deque<char>
SortingLeafReader::SortingPointValues::getMaxPackedValue() 
{
  return in_->getMaxPackedValue();
}

int SortingLeafReader::SortingPointValues::getNumDimensions() 
{
  return in_->getNumDimensions();
}

int SortingLeafReader::SortingPointValues::getBytesPerDimension() throw(
    IOException)
{
  return in_->getBytesPerDimension();
}

int64_t SortingLeafReader::SortingPointValues::size() { return in_->size(); }

int SortingLeafReader::SortingPointValues::getDocCount()
{
  return in_->getDocCount();
}

SortingLeafReader::SortingSortedDocValues::SortingSortedDocValues(
    shared_ptr<SortedDocValues> in_, std::deque<int> &ords)
    : in_(in_), ords(ords)
{
  assert(ords.size() > 0);
}

int SortingLeafReader::SortingSortedDocValues::docID() { return docID_; }

int SortingLeafReader::SortingSortedDocValues::nextDoc()
{
  while (true) {
    docID_++;
    if (docID_ == ords.size()) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    if (ords[docID_] != -1) {
      break;
    }
    // skip missing docs
  }

  return docID_;
}

int SortingLeafReader::SortingSortedDocValues::advance(int target)
{
  if (target >= ords.size()) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = target;
    if (ords[docID_] == -1) {
      nextDoc();
    }
  }
  return docID_;
}

bool SortingLeafReader::SortingSortedDocValues::advanceExact(int target) throw(
    IOException)
{
  docID_ = target;
  return ords[target] != -1;
}

int SortingLeafReader::SortingSortedDocValues::ordValue()
{
  return ords[docID_];
}

int64_t SortingLeafReader::SortingSortedDocValues::cost()
{
  return in_->cost();
}

shared_ptr<BytesRef>
SortingLeafReader::SortingSortedDocValues::lookupOrd(int ord) 
{
  return in_->lookupOrd(ord);
}

int SortingLeafReader::SortingSortedDocValues::getValueCount()
{
  return in_->getValueCount();
}

SortingLeafReader::SortingSortedSetDocValues::SortingSortedSetDocValues(
    shared_ptr<SortedSetDocValues> in_,
    std::deque<std::deque<int64_t>> &ords)
    : in_(in_), ords(ords)
{
}

int SortingLeafReader::SortingSortedSetDocValues::docID() { return docID_; }

int SortingLeafReader::SortingSortedSetDocValues::nextDoc()
{
  while (true) {
    docID_++;
    if (docID_ == ords.size()) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    if (ords[docID_] != nullptr) {
      break;
    }
    // skip missing docs
  }
  ordUpto = 0;
  return docID_;
}

int SortingLeafReader::SortingSortedSetDocValues::advance(int target)
{
  if (target >= ords.size()) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = target;
    if (ords[docID_] == nullptr) {
      nextDoc();
    } else {
      ordUpto = 0;
    }
  }
  return docID_;
}

bool SortingLeafReader::SortingSortedSetDocValues::advanceExact(
    int target) 
{
  docID_ = target;
  ordUpto = 0;
  return ords[docID_] != nullptr;
}

int64_t SortingLeafReader::SortingSortedSetDocValues::nextOrd()
{
  if (ordUpto == ords[docID_].length) {
    return NO_MORE_ORDS;
  } else {
    return ords[docID_][ordUpto++];
  }
}

int64_t SortingLeafReader::SortingSortedSetDocValues::cost()
{
  return in_->cost();
}

shared_ptr<BytesRef> SortingLeafReader::SortingSortedSetDocValues::lookupOrd(
    int64_t ord) 
{
  return in_->lookupOrd(ord);
}

int64_t SortingLeafReader::SortingSortedSetDocValues::getValueCount()
{
  return in_->getValueCount();
}

SortingLeafReader::SortingSortedNumericDocValues::SortingSortedNumericDocValues(
    shared_ptr<SortedNumericDocValues> in_,
    std::deque<std::deque<int64_t>> &values)
    : in_(in_), values(values)
{
}

int SortingLeafReader::SortingSortedNumericDocValues::docID() { return docID_; }

int SortingLeafReader::SortingSortedNumericDocValues::nextDoc()
{
  while (true) {
    docID_++;
    if (docID_ == values.size()) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    if (values[docID_] != nullptr) {
      break;
    }
    // skip missing docs
  }
  upto = 0;
  return docID_;
}

int SortingLeafReader::SortingSortedNumericDocValues::advance(int target)
{
  if (target >= values.size()) {
    docID_ = NO_MORE_DOCS;
    return docID_;
  } else {
    docID_ = target - 1;
    return nextDoc();
  }
}

bool SortingLeafReader::SortingSortedNumericDocValues::advanceExact(
    int target) 
{
  docID_ = target;
  upto = 0;
  return values[docID_] != nullptr;
}

int64_t SortingLeafReader::SortingSortedNumericDocValues::nextValue()
{
  if (upto == values[docID_].length) {
    throw make_shared<AssertionError>();
  } else {
    return values[docID_][upto++];
  }
}

int64_t SortingLeafReader::SortingSortedNumericDocValues::cost()
{
  return in_->cost();
}

int SortingLeafReader::SortingSortedNumericDocValues::docValueCount()
{
  return values[docID_].length;
}

SortingLeafReader::SortingDocsEnum::DocFreqSorter::DocFreqSorter(int maxDoc)
    : org::apache::lucene::util::TimSorter(maxDoc / 64),
      tmpDocs(std::deque<int>(maxDoc / 64))
{
}

void SortingLeafReader::SortingDocsEnum::DocFreqSorter::reset(
    std::deque<int> &docs, std::deque<int> &freqs)
{
  this->docs = docs;
  this->freqs = freqs;
  if (freqs.size() > 0 && tmpFreqs.empty()) {
    tmpFreqs = std::deque<int>(tmpDocs.size());
  }
}

int SortingLeafReader::SortingDocsEnum::DocFreqSorter::compare(int i, int j)
{
  return docs[i] - docs[j];
}

void SortingLeafReader::SortingDocsEnum::DocFreqSorter::swap(int i, int j)
{
  int tmpDoc = docs[i];
  docs[i] = docs[j];
  docs[j] = tmpDoc;

  if (freqs.size() > 0) {
    int tmpFreq = freqs[i];
    freqs[i] = freqs[j];
    freqs[j] = tmpFreq;
  }
}

void SortingLeafReader::SortingDocsEnum::DocFreqSorter::copy(int src, int dest)
{
  docs[dest] = docs[src];
  if (freqs.size() > 0) {
    freqs[dest] = freqs[src];
  }
}

void SortingLeafReader::SortingDocsEnum::DocFreqSorter::save(int i, int len)
{
  System::arraycopy(docs, i, tmpDocs, 0, len);
  if (freqs.size() > 0) {
    System::arraycopy(freqs, i, tmpFreqs, 0, len);
  }
}

void SortingLeafReader::SortingDocsEnum::DocFreqSorter::restore(int i, int j)
{
  docs[j] = tmpDocs[i];
  if (freqs.size() > 0) {
    freqs[j] = tmpFreqs[i];
  }
}

int SortingLeafReader::SortingDocsEnum::DocFreqSorter::compareSaved(int i,
                                                                    int j)
{
  return tmpDocs[i] - docs[j];
}

SortingLeafReader::SortingDocsEnum::SortingDocsEnum(
    int maxDoc, shared_ptr<SortingDocsEnum> reuse, shared_ptr<PostingsEnum> in_,
    bool withFreqs, shared_ptr<Sorter::DocMap> docMap) 
    : FilterPostingsEnum(in_), maxDoc(maxDoc), upto(i), withFreqs(withFreqs)
{
  if (reuse != nullptr) {
    if (reuse->maxDoc == maxDoc) {
      sorter = reuse->sorter;
    } else {
      sorter = make_shared<DocFreqSorter>(maxDoc);
    }
    docs = reuse->docs;
    freqs = reuse->freqs; // maybe null
  } else {
    docs = std::deque<int>(64);
    sorter = make_shared<DocFreqSorter>(maxDoc);
  }
  docIt = -1;
  int i = 0;
  int doc;
  if (withFreqs) {
    if (freqs.empty() || freqs.size() < docs.size()) {
      freqs = std::deque<int>(docs.size());
    }
    while ((doc = in_->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      if (i >= docs.size()) {
        docs = ArrayUtil::grow(docs, docs.size() + 1);
        freqs = ArrayUtil::grow(freqs, freqs.size() + 1);
      }
      docs[i] = docMap->oldToNew(doc);
      freqs[i] = in_->freq();
      ++i;
    }
  } else {
    freqs.clear();
    while ((doc = in_->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      if (i >= docs.size()) {
        docs = ArrayUtil::grow(docs, docs.size() + 1);
      }
      docs[i++] = docMap->oldToNew(doc);
    }
  }
  // TimSort can save much time compared to other sorts in case of
  // reverse sorting, or when sorting a concatenation of sorted readers
  sorter->reset(docs, freqs);
  sorter->sort(0, i);
}

bool SortingLeafReader::SortingDocsEnum::reused(shared_ptr<PostingsEnum> other)
{
  if (other == nullptr ||
      !(std::dynamic_pointer_cast<SortingDocsEnum>(other) != nullptr)) {
    return false;
  }
  return docs == (std::static_pointer_cast<SortingDocsEnum>(other))->docs;
}

int SortingLeafReader::SortingDocsEnum::advance(int const target) throw(
    IOException)
{
  // need to support it for checkIndex, but in practice it won't be called, so
  // don't bother to implement efficiently for now.
  return slowAdvance(target);
}

int SortingLeafReader::SortingDocsEnum::docID()
{
  return docIt < 0 ? -1 : docIt >= upto ? NO_MORE_DOCS : docs[docIt];
}

int SortingLeafReader::SortingDocsEnum::freq() 
{
  return withFreqs && docIt < upto ? freqs[docIt] : 1;
}

int SortingLeafReader::SortingDocsEnum::nextDoc() 
{
  if (++docIt >= upto) {
    return NO_MORE_DOCS;
  }
  return docs[docIt];
}

shared_ptr<PostingsEnum> SortingLeafReader::SortingDocsEnum::getWrapped()
{
  return in_;
}

int SortingLeafReader::SortingDocsEnum::nextPosition() 
{
  return -1;
}

int SortingLeafReader::SortingDocsEnum::startOffset() 
{
  return -1;
}

int SortingLeafReader::SortingDocsEnum::endOffset() 
{
  return -1;
}

shared_ptr<BytesRef>
SortingLeafReader::SortingDocsEnum::getPayload() 
{
  return nullptr;
}

SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::DocOffsetSorter(
    int maxDoc)
    : org::apache::lucene::util::TimSorter(maxDoc / 64),
      tmpDocs(std::deque<int>(maxDoc / 64)),
      tmpOffsets(std::deque<int64_t>(maxDoc / 64))
{
}

void SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::reset(
    std::deque<int> &docs, std::deque<int64_t> &offsets)
{
  this->docs = docs;
  this->offsets = offsets;
}

int SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::compare(int i,
                                                                     int j)
{
  return docs[i] - docs[j];
}

void SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::swap(int i, int j)
{
  int tmpDoc = docs[i];
  docs[i] = docs[j];
  docs[j] = tmpDoc;

  int64_t tmpOffset = offsets[i];
  offsets[i] = offsets[j];
  offsets[j] = tmpOffset;
}

void SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::copy(int src,
                                                                   int dest)
{
  docs[dest] = docs[src];
  offsets[dest] = offsets[src];
}

void SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::save(int i,
                                                                   int len)
{
  System::arraycopy(docs, i, tmpDocs, 0, len);
  System::arraycopy(offsets, i, tmpOffsets, 0, len);
}

void SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::restore(int i,
                                                                      int j)
{
  docs[j] = tmpDocs[i];
  offsets[j] = tmpOffsets[i];
}

int SortingLeafReader::SortingPostingsEnum::DocOffsetSorter::compareSaved(int i,
                                                                          int j)
{
  return tmpDocs[i] - docs[j];
}

SortingLeafReader::SortingPostingsEnum::SortingPostingsEnum(
    int maxDoc, shared_ptr<SortingPostingsEnum> reuse,
    shared_ptr<PostingsEnum> in_, shared_ptr<Sorter::DocMap> docMap,
    bool storeOffsets) 
    : FilterPostingsEnum(in_), maxDoc(maxDoc), upto(i),
      postingInput(make_shared<RAMInputStream>(L"", file)),
      storeOffsets(storeOffsets)
{
  if (reuse != nullptr) {
    docs = reuse->docs;
    offsets = reuse->offsets;
    payload = reuse->payload;
    file = reuse->file;
    if (reuse->maxDoc == maxDoc) {
      sorter = reuse->sorter;
    } else {
      sorter = make_shared<DocOffsetSorter>(maxDoc);
    }
  } else {
    docs = std::deque<int>(32);
    offsets = std::deque<int64_t>(32);
    payload = make_shared<BytesRef>(32);
    file = make_shared<RAMFile>();
    sorter = make_shared<DocOffsetSorter>(maxDoc);
  }
  shared_ptr<IndexOutput> *const out =
      make_shared<RAMOutputStream>(file, false);
  int doc;
  int i = 0;
  while ((doc = in_->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
    if (i == docs.size()) {
      constexpr int newLength = ArrayUtil::oversize(i + 1, 4);
      docs = Arrays::copyOf(docs, newLength);
      offsets = Arrays::copyOf(offsets, newLength);
    }
    docs[i] = docMap->oldToNew(doc);
    offsets[i] = out->getFilePointer();
    addPositions(in_, out);
    i++;
  }
  sorter->reset(docs, offsets);
  sorter->sort(0, upto);
  delete out;
}

bool SortingLeafReader::SortingPostingsEnum::reused(
    shared_ptr<PostingsEnum> other)
{
  if (other == nullptr ||
      !(std::dynamic_pointer_cast<SortingPostingsEnum>(other) != nullptr)) {
    return false;
  }
  return docs == (std::static_pointer_cast<SortingPostingsEnum>(other))->docs;
}

void SortingLeafReader::SortingPostingsEnum::addPositions(
    shared_ptr<PostingsEnum> in_,
    shared_ptr<IndexOutput> out) 
{
  int freq = in_->freq();
  out->writeVInt(freq);
  int previousPosition = 0;
  int previousEndOffset = 0;
  for (int i = 0; i < freq; i++) {
    constexpr int pos = in_->nextPosition();
    shared_ptr<BytesRef> *const payload = in_->getPayload();
    // The low-order bit of token is set only if there is a payload, the
    // previous bits are the delta-encoded position.
    constexpr int token =
        (pos - previousPosition) << 1 | (payload == nullptr ? 0 : 1);
    out->writeVInt(token);
    previousPosition = pos;
    if (storeOffsets) { // don't encode offsets if they are not stored
      constexpr int startOffset = in_->startOffset();
      constexpr int endOffset = in_->endOffset();
      out->writeVInt(startOffset - previousEndOffset);
      out->writeVInt(endOffset - startOffset);
      previousEndOffset = endOffset;
    }
    if (payload != nullptr) {
      out->writeVInt(payload->length);
      out->writeBytes(payload->bytes, payload->offset, payload->length);
    }
  }
}

int SortingLeafReader::SortingPostingsEnum::advance(int const target) throw(
    IOException)
{
  // need to support it for checkIndex, but in practice it won't be called, so
  // don't bother to implement efficiently for now.
  return slowAdvance(target);
}

int SortingLeafReader::SortingPostingsEnum::docID()
{
  return docIt < 0 ? -1 : docIt >= upto ? NO_MORE_DOCS : docs[docIt];
}

int SortingLeafReader::SortingPostingsEnum::endOffset() 
{
  return endOffset_;
}

int SortingLeafReader::SortingPostingsEnum::freq() 
{
  return currFreq;
}

shared_ptr<BytesRef>
SortingLeafReader::SortingPostingsEnum::getPayload() 
{
  return payload->length == 0 ? nullptr : payload;
}

int SortingLeafReader::SortingPostingsEnum::nextDoc() 
{
  if (++docIt >= upto) {
    return DocIdSetIterator::NO_MORE_DOCS;
  }
  postingInput->seek(offsets[docIt]);
  currFreq = postingInput->readVInt();
  // reset variables used in nextPosition
  pos = 0;
  endOffset_ = 0;
  return docs[docIt];
}

int SortingLeafReader::SortingPostingsEnum::nextPosition() 
{
  constexpr int token = postingInput->readVInt();
  pos += static_cast<int>(static_cast<unsigned int>(token) >> 1);
  if (storeOffsets) {
    startOffset_ = endOffset_ + postingInput->readVInt();
    endOffset_ = startOffset_ + postingInput->readVInt();
  }
  if ((token & 1) != 0) {
    payload->offset = 0;
    payload->length = postingInput->readVInt();
    if (payload->length > payload->bytes.size()) {
      payload->bytes =
          std::deque<char>(ArrayUtil::oversize(payload->length, 1));
    }
    postingInput->readBytes(payload->bytes, 0, payload->length);
  } else {
    payload->length = 0;
  }
  return pos;
}

int SortingLeafReader::SortingPostingsEnum::startOffset() 
{
  return startOffset_;
}

shared_ptr<PostingsEnum> SortingLeafReader::SortingPostingsEnum::getWrapped()
{
  return in_;
}

shared_ptr<LeafReader>
SortingLeafReader::wrap(shared_ptr<LeafReader> reader,
                        shared_ptr<Sort> sort) 
{
  return wrap(reader, (make_shared<Sorter>(sort))->sort(reader));
}

shared_ptr<LeafReader>
SortingLeafReader::wrap(shared_ptr<LeafReader> reader,
                        shared_ptr<Sorter::DocMap> docMap)
{
  if (docMap == nullptr) {
    // the reader is already sorted
    return reader;
  }
  if (reader->maxDoc() != docMap->size()) {
    throw invalid_argument(
        L"reader.maxDoc() should be equal to docMap.size(), got" +
        to_wstring(reader->maxDoc()) + L" != " + to_wstring(docMap->size()));
  }
  assert(Sorter::isConsistent(docMap));
  return make_shared<SortingLeafReader>(reader, docMap);
}

SortingLeafReader::SortingLeafReader(shared_ptr<LeafReader> in_,
                                     shared_ptr<Sorter::DocMap> docMap)
    : FilterLeafReader(in_), docMap(docMap)
{
}

void SortingLeafReader::document(
    int const docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  in_->document(docMap->newToOld(docID), visitor);
}

shared_ptr<Terms>
SortingLeafReader::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = FilterLeafReader::terms(field);
  return terms == nullptr
             ? nullptr
             : make_shared<SortingTerms>(
                   terms,
                   in_->getFieldInfos()->fieldInfo(field)->getIndexOptions(),
                   docMap);
}

shared_ptr<BinaryDocValues>
SortingLeafReader::getBinaryDocValues(const wstring &field) 
{
  shared_ptr<BinaryDocValues> *const oldDocValues =
      in_->getBinaryDocValues(field);
  if (oldDocValues == nullptr) {
    return nullptr;
  }
  shared_ptr<CachedBinaryDVs> dvs;
  {
    lock_guard<mutex> lock(cachedBinaryDVs);
    dvs = cachedBinaryDVs[field];
    if (dvs == nullptr) {
      shared_ptr<FixedBitSet> docsWithField =
          make_shared<FixedBitSet>(maxDoc());
      std::deque<std::shared_ptr<BytesRef>> values(maxDoc());
      while (true) {
        int docID = oldDocValues->nextDoc();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        int newDocID = docMap->oldToNew(docID);
        docsWithField->set(newDocID);
        values[newDocID] = BytesRef::deepCopyOf(oldDocValues->binaryValue());
      }
      dvs = make_shared<CachedBinaryDVs>(values, docsWithField);
      cachedBinaryDVs.emplace(field, dvs);
    }
  }
  return make_shared<SortingBinaryDocValues>(dvs);
}

shared_ptr<Bits> SortingLeafReader::getLiveDocs()
{
  shared_ptr<Bits> *const inLiveDocs = in_->getLiveDocs();
  if (inLiveDocs == nullptr) {
    return nullptr;
  } else {
    return make_shared<SortingBits>(inLiveDocs, docMap);
  }
}

shared_ptr<PointValues>
SortingLeafReader::getPointValues(const wstring &fieldName) 
{
  shared_ptr<PointValues> *const inPointValues = in_->getPointValues(fieldName);
  if (inPointValues == nullptr) {
    return nullptr;
  } else {
    return make_shared<SortingPointValues>(inPointValues, docMap);
  }
}

shared_ptr<NumericDocValues>
SortingLeafReader::getNormValues(const wstring &field) 
{
  shared_ptr<NumericDocValues> *const oldNorms = in_->getNormValues(field);
  if (oldNorms == nullptr) {
    return nullptr;
  }
  shared_ptr<CachedNumericDVs> norms;
  {
    lock_guard<mutex> lock(cachedNorms);
    norms = cachedNorms[field];
    if (norms == nullptr) {
      shared_ptr<FixedBitSet> docsWithField =
          make_shared<FixedBitSet>(maxDoc());
      std::deque<int64_t> values(maxDoc());
      while (true) {
        int docID = oldNorms->nextDoc();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        int newDocID = docMap->oldToNew(docID);
        docsWithField->set(newDocID);
        values[newDocID] = oldNorms->longValue();
      }
      norms = make_shared<CachedNumericDVs>(values, docsWithField);
      cachedNorms.emplace(field, norms);
    }
  }
  return make_shared<SortingNumericDocValues>(norms);
}

shared_ptr<NumericDocValues>
SortingLeafReader::getNumericDocValues(const wstring &field) 
{
  shared_ptr<NumericDocValues> *const oldDocValues =
      in_->getNumericDocValues(field);
  if (oldDocValues == nullptr) {
    return nullptr;
  }
  shared_ptr<CachedNumericDVs> dvs;
  {
    lock_guard<mutex> lock(cachedNumericDVs);
    dvs = cachedNumericDVs[field];
    if (dvs == nullptr) {
      shared_ptr<FixedBitSet> docsWithField =
          make_shared<FixedBitSet>(maxDoc());
      std::deque<int64_t> values(maxDoc());
      while (true) {
        int docID = oldDocValues->nextDoc();
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        int newDocID = docMap->oldToNew(docID);
        docsWithField->set(newDocID);
        values[newDocID] = oldDocValues->longValue();
      }
      dvs = make_shared<CachedNumericDVs>(values, docsWithField);
      cachedNumericDVs.emplace(field, dvs);
    }
  }
  return make_shared<SortingNumericDocValues>(dvs);
}

shared_ptr<SortedNumericDocValues> SortingLeafReader::getSortedNumericDocValues(
    const wstring &field) 
{
  shared_ptr<SortedNumericDocValues> *const oldDocValues =
      in_->getSortedNumericDocValues(field);
  if (oldDocValues == nullptr) {
    return nullptr;
  }

  std::deque<std::deque<int64_t>> values;
  {
    lock_guard<mutex> lock(cachedSortedNumericDVs);
    values = cachedSortedNumericDVs[field];
    if (values.empty()) {
      values = std::deque<std::deque<int64_t>>(maxDoc());
      int docID;
      while ((docID = oldDocValues->nextDoc()) !=
             DocIdSetIterator::NO_MORE_DOCS) {
        int newDocID = docMap->oldToNew(docID);
        std::deque<int64_t> docValues(oldDocValues->docValueCount());
        for (int i = 0; i < docValues.size(); i++) {
          docValues[i] = oldDocValues->nextValue();
        }
        values[newDocID] = docValues;
      }
      cachedSortedNumericDVs.emplace(field, values);
    }
  }

  return make_shared<SortingSortedNumericDocValues>(oldDocValues, values);
}

shared_ptr<SortedDocValues>
SortingLeafReader::getSortedDocValues(const wstring &field) 
{
  shared_ptr<SortedDocValues> oldDocValues = in_->getSortedDocValues(field);
  if (oldDocValues == nullptr) {
    return nullptr;
  }

  std::deque<int> ords;
  {
    lock_guard<mutex> lock(cachedSortedDVs);
    ords = cachedSortedDVs[field];
    if (ords.empty()) {
      ords = std::deque<int>(maxDoc());
      Arrays::fill(ords, -1);
      int docID;
      while ((docID = oldDocValues->nextDoc()) !=
             DocIdSetIterator::NO_MORE_DOCS) {
        int newDocID = docMap->oldToNew(docID);
        ords[newDocID] = oldDocValues->ordValue();
      }
      cachedSortedDVs.emplace(field, ords);
    }
  }

  return make_shared<SortingSortedDocValues>(oldDocValues, ords);
}

shared_ptr<SortedSetDocValues> SortingLeafReader::getSortedSetDocValues(
    const wstring &field) 
{
  shared_ptr<SortedSetDocValues> oldDocValues =
      in_->getSortedSetDocValues(field);
  if (oldDocValues == nullptr) {
    return nullptr;
  }

  std::deque<std::deque<int64_t>> ords;
  {
    lock_guard<mutex> lock(cachedSortedSetDVs);
    ords = cachedSortedSetDVs[field];
    if (ords.empty()) {
      ords = std::deque<std::deque<int64_t>>(maxDoc());
      int docID;
      while ((docID = oldDocValues->nextDoc()) !=
             DocIdSetIterator::NO_MORE_DOCS) {
        int newDocID = docMap->oldToNew(docID);
        std::deque<int64_t> docOrds(1);
        int upto = 0;
        while (true) {
          int64_t ord = oldDocValues->nextOrd();
          if (ord == NO_MORE_ORDS) {
            break;
          }
          if (upto == docOrds.size()) {
            docOrds = ArrayUtil::grow(docOrds);
          }
          docOrds[upto++] = ord;
        }
        ords[newDocID] = Arrays::copyOfRange(docOrds, 0, upto);
      }
      cachedSortedSetDVs.emplace(field, ords);
    }
  }

  return make_shared<SortingSortedSetDocValues>(oldDocValues, ords);
}

shared_ptr<Fields>
SortingLeafReader::getTermVectors(int const docID) 
{
  return in_->getTermVectors(docMap->newToOld(docID));
}

wstring SortingLeafReader::toString()
{
  return L"SortingLeafReader(" + in_ + L")";
}

shared_ptr<CacheHelper> SortingLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> SortingLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index