using namespace std;

#include "FilterLeafReader.h"

namespace org::apache::lucene::index
{
using AttributeSource = org::apache::lucene::util::AttributeSource;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

shared_ptr<LeafReader> FilterLeafReader::unwrap(shared_ptr<LeafReader> reader)
{
  while (std::dynamic_pointer_cast<FilterLeafReader>(reader) != nullptr) {
    reader = (std::static_pointer_cast<FilterLeafReader>(reader))->in_;
  }
  return reader;
}

FilterLeafReader::FilterFields::FilterFields(shared_ptr<Fields> in_) : in_(in_)
{
  if (in_->empty()) {
    throw make_shared<NullPointerException>(
        L"incoming Fields must not be null");
  }
}

shared_ptr<Iterator<wstring>> FilterLeafReader::FilterFields::iterator()
{
  return in_->begin();
}

shared_ptr<Terms>
FilterLeafReader::FilterFields::terms(const wstring &field) 
{
  return in_->terms(field);
}

int FilterLeafReader::FilterFields::size() { return in_->size(); }

FilterLeafReader::FilterTerms::FilterTerms(shared_ptr<Terms> in_) : in_(in_)
{
  if (in_ == nullptr) {
    throw make_shared<NullPointerException>(L"incoming Terms must not be null");
  }
}

shared_ptr<TermsEnum>
FilterLeafReader::FilterTerms::iterator() 
{
  return in_->begin();
}

int64_t FilterLeafReader::FilterTerms::size() 
{
  return in_->size();
}

int64_t
FilterLeafReader::FilterTerms::getSumTotalTermFreq() 
{
  return in_->getSumTotalTermFreq();
}

int64_t FilterLeafReader::FilterTerms::getSumDocFreq() 
{
  return in_->getSumDocFreq();
}

int FilterLeafReader::FilterTerms::getDocCount() 
{
  return in_->getDocCount();
}

bool FilterLeafReader::FilterTerms::hasFreqs() { return in_->hasFreqs(); }

bool FilterLeafReader::FilterTerms::hasOffsets() { return in_->hasOffsets(); }

bool FilterLeafReader::FilterTerms::hasPositions()
{
  return in_->hasPositions();
}

bool FilterLeafReader::FilterTerms::hasPayloads() { return in_->hasPayloads(); }

any FilterLeafReader::FilterTerms::getStats() 
{
  return in_->getStats();
}

FilterLeafReader::FilterTermsEnum::FilterTermsEnum(shared_ptr<TermsEnum> in_)
    : in_(in_)
{
  if (in_ == nullptr) {
    throw make_shared<NullPointerException>(
        L"incoming TermsEnum must not be null");
  }
}

shared_ptr<AttributeSource> FilterLeafReader::FilterTermsEnum::attributes()
{
  return in_->attributes();
}

SeekStatus FilterLeafReader::FilterTermsEnum::seekCeil(
    shared_ptr<BytesRef> text) 
{
  return in_->seekCeil(text);
}

void FilterLeafReader::FilterTermsEnum::seekExact(int64_t ord) throw(
    IOException)
{
  in_->seekExact(ord);
}

shared_ptr<BytesRef>
FilterLeafReader::FilterTermsEnum::next() 
{
  return in_->next();
}

shared_ptr<BytesRef>
FilterLeafReader::FilterTermsEnum::term() 
{
  return in_->term();
}

int64_t FilterLeafReader::FilterTermsEnum::ord() 
{
  return in_->ord();
}

int FilterLeafReader::FilterTermsEnum::docFreq() 
{
  return in_->docFreq();
}

int64_t FilterLeafReader::FilterTermsEnum::totalTermFreq() 
{
  return in_->totalTermFreq();
}

shared_ptr<PostingsEnum>
FilterLeafReader::FilterTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                            int flags) 
{
  return in_->postings(reuse, flags);
}

FilterLeafReader::FilterPostingsEnum::FilterPostingsEnum(
    shared_ptr<PostingsEnum> in_)
    : in_(in_)
{
  if (in_ == nullptr) {
    throw make_shared<NullPointerException>(
        L"incoming PostingsEnum must not be null");
  }
}

shared_ptr<AttributeSource> FilterLeafReader::FilterPostingsEnum::attributes()
{
  return in_->attributes();
}

int FilterLeafReader::FilterPostingsEnum::docID() { return in_->docID(); }

int FilterLeafReader::FilterPostingsEnum::freq() 
{
  return in_->freq();
}

int FilterLeafReader::FilterPostingsEnum::nextDoc() 
{
  return in_->nextDoc();
}

int FilterLeafReader::FilterPostingsEnum::advance(int target) 
{
  return in_->advance(target);
}

int FilterLeafReader::FilterPostingsEnum::nextPosition() 
{
  return in_->nextPosition();
}

int FilterLeafReader::FilterPostingsEnum::startOffset() 
{
  return in_->startOffset();
}

int FilterLeafReader::FilterPostingsEnum::endOffset() 
{
  return in_->endOffset();
}

shared_ptr<BytesRef>
FilterLeafReader::FilterPostingsEnum::getPayload() 
{
  return in_->getPayload();
}

int64_t FilterLeafReader::FilterPostingsEnum::cost() { return in_->cost(); }

FilterLeafReader::FilterLeafReader(shared_ptr<LeafReader> in_)
    : LeafReader(), in_(in_)
{
  if (in_ == nullptr) {
    throw make_shared<NullPointerException>(
        L"incoming LeafReader must not be null");
  }
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  in_->registerParentReader(shared_from_this());
}

shared_ptr<Bits> FilterLeafReader::getLiveDocs()
{
  ensureOpen();
  return in_->getLiveDocs();
}

shared_ptr<FieldInfos> FilterLeafReader::getFieldInfos()
{
  return in_->getFieldInfos();
}

shared_ptr<PointValues>
FilterLeafReader::getPointValues(const wstring &field) 
{
  return in_->getPointValues(field);
}

shared_ptr<Fields>
FilterLeafReader::getTermVectors(int docID) 
{
  ensureOpen();
  return in_->getTermVectors(docID);
}

int FilterLeafReader::numDocs()
{
  // Don't call ensureOpen() here (it could affect performance)
  return in_->numDocs();
}

int FilterLeafReader::maxDoc()
{
  // Don't call ensureOpen() here (it could affect performance)
  return in_->maxDoc();
}

void FilterLeafReader::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
  ensureOpen();
  in_->document(docID, visitor);
}

void FilterLeafReader::doClose()  { delete in_; }

shared_ptr<Terms>
FilterLeafReader::terms(const wstring &field) 
{
  ensureOpen();
  return in_->terms(field);
}

wstring FilterLeafReader::toString()
{
  shared_ptr<StringBuilder> *const buffer =
      make_shared<StringBuilder>(L"FilterLeafReader(");
  buffer->append(in_);
  buffer->append(L')');
  return buffer->toString();
}

shared_ptr<NumericDocValues>
FilterLeafReader::getNumericDocValues(const wstring &field) 
{
  ensureOpen();
  return in_->getNumericDocValues(field);
}

shared_ptr<BinaryDocValues>
FilterLeafReader::getBinaryDocValues(const wstring &field) 
{
  ensureOpen();
  return in_->getBinaryDocValues(field);
}

shared_ptr<SortedDocValues>
FilterLeafReader::getSortedDocValues(const wstring &field) 
{
  ensureOpen();
  return in_->getSortedDocValues(field);
}

shared_ptr<SortedNumericDocValues> FilterLeafReader::getSortedNumericDocValues(
    const wstring &field) 
{
  ensureOpen();
  return in_->getSortedNumericDocValues(field);
}

shared_ptr<SortedSetDocValues>
FilterLeafReader::getSortedSetDocValues(const wstring &field) 
{
  ensureOpen();
  return in_->getSortedSetDocValues(field);
}

shared_ptr<NumericDocValues>
FilterLeafReader::getNormValues(const wstring &field) 
{
  ensureOpen();
  return in_->getNormValues(field);
}

shared_ptr<LeafMetaData> FilterLeafReader::getMetaData()
{
  ensureOpen();
  return in_->getMetaData();
}

void FilterLeafReader::checkIntegrity() 
{
  ensureOpen();
  in_->checkIntegrity();
}

shared_ptr<LeafReader> FilterLeafReader::getDelegate() { return in_; }
} // namespace org::apache::lucene::index