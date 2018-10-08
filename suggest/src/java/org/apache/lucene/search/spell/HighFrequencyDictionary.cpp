using namespace std;

#include "HighFrequencyDictionary.h"

namespace org::apache::lucene::search::spell
{
using IndexReader = org::apache::lucene::index::IndexReader;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Terms = org::apache::lucene::index::Terms;
using MultiFields = org::apache::lucene::index::MultiFields;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

HighFrequencyDictionary::HighFrequencyDictionary(shared_ptr<IndexReader> reader,
                                                 const wstring &field,
                                                 float thresh)
{
  this->reader = reader;
  this->field = field;
  this->thresh = thresh;
}

shared_ptr<InputIterator>
HighFrequencyDictionary::getEntryIterator() 
{
  return make_shared<HighFrequencyIterator>(shared_from_this());
}

HighFrequencyDictionary::HighFrequencyIterator::HighFrequencyIterator(
    shared_ptr<HighFrequencyDictionary> outerInstance) 
    : outerInstance(outerInstance)
{
  shared_ptr<Terms> terms =
      MultiFields::getTerms(outerInstance->reader, outerInstance->field);
  if (terms != nullptr) {
    termsEnum = terms->begin();
  } else {
    termsEnum.reset();
  }
  minNumDocs =
      static_cast<int>(outerInstance->thresh *
                       static_cast<float>(outerInstance->reader->numDocs()));
}

bool HighFrequencyDictionary::HighFrequencyIterator::isFrequent(int freq)
{
  return freq >= minNumDocs;
}

int64_t HighFrequencyDictionary::HighFrequencyIterator::weight()
{
  return freq;
}

shared_ptr<BytesRef>
HighFrequencyDictionary::HighFrequencyIterator::next() 
{
  if (termsEnum != nullptr) {
    shared_ptr<BytesRef> next;
    while ((next = termsEnum->next()) != nullptr) {
      if (isFrequent(termsEnum->docFreq())) {
        freq = termsEnum->docFreq();
        spare->copyBytes(next);
        return spare->get();
      }
    }
  }
  return nullptr;
}

shared_ptr<BytesRef> HighFrequencyDictionary::HighFrequencyIterator::payload()
{
  return nullptr;
}

bool HighFrequencyDictionary::HighFrequencyIterator::hasPayloads()
{
  return false;
}

shared_ptr<Set<std::shared_ptr<BytesRef>>>
HighFrequencyDictionary::HighFrequencyIterator::contexts()
{
  return nullptr;
}

bool HighFrequencyDictionary::HighFrequencyIterator::hasContexts()
{
  return false;
}
} // namespace org::apache::lucene::search::spell