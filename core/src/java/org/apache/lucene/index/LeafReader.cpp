using namespace std;

#include "LeafReader.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;

LeafReader::LeafReader() : IndexReader() {}

shared_ptr<LeafReaderContext> LeafReader::getContext()
{
  ensureOpen();
  return readerContext;
}

int LeafReader::docFreq(shared_ptr<Term> term) 
{
  shared_ptr<Terms> *const terms = this->terms(term->field());
  if (terms == nullptr) {
    return 0;
  }
  shared_ptr<TermsEnum> *const termsEnum = terms->begin();
  if (termsEnum->seekExact(term->bytes())) {
    return termsEnum->docFreq();
  } else {
    return 0;
  }
}

int64_t LeafReader::totalTermFreq(shared_ptr<Term> term) 
{
  shared_ptr<Terms> *const terms = this->terms(term->field());
  if (terms == nullptr) {
    return 0;
  }
  shared_ptr<TermsEnum> *const termsEnum = terms->begin();
  if (termsEnum->seekExact(term->bytes())) {
    return termsEnum->totalTermFreq();
  } else {
    return 0;
  }
}

int64_t LeafReader::getSumDocFreq(const wstring &field) 
{
  shared_ptr<Terms> *const terms = this->terms(field);
  if (terms == nullptr) {
    return 0;
  }
  return terms->getSumDocFreq();
}

int LeafReader::getDocCount(const wstring &field) 
{
  shared_ptr<Terms> *const terms = this->terms(field);
  if (terms == nullptr) {
    return 0;
  }
  return terms->getDocCount();
}

int64_t
LeafReader::getSumTotalTermFreq(const wstring &field) 
{
  shared_ptr<Terms> *const terms = this->terms(field);
  if (terms == nullptr) {
    return 0;
  }
  return terms->getSumTotalTermFreq();
}

shared_ptr<PostingsEnum> LeafReader::postings(shared_ptr<Term> term,
                                              int flags) 
{
  assert(term->field() != L"");
  assert(term->bytes() != nullptr);
  shared_ptr<Terms> *const terms = this->terms(term->field());
  if (terms != nullptr) {
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (termsEnum->seekExact(term->bytes())) {
      return termsEnum->postings(nullptr, flags);
    }
  }
  return nullptr;
}

shared_ptr<PostingsEnum>
LeafReader::postings(shared_ptr<Term> term) 
{
  return postings(term, PostingsEnum::FREQS);
}
} // namespace org::apache::lucene::index