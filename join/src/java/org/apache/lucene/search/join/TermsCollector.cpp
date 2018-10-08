using namespace std;

#include "TermsCollector.h"

namespace org::apache::lucene::search::join
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

TermsCollector<DV>::MV::MV(
    Function<std::shared_ptr<SortedSetDocValues>> docValuesCall)
    : TermsCollector<org::apache::lucene::index::SortedSetDocValues>(
          docValuesCall)
{
}

void TermsCollector<DV>::MV::collect(int doc) 
{
  int64_t ord;
  if (doc > docValues->docID()) {
    docValues->advance(doc);
  }
  if (doc == docValues->docID()) {
    while ((ord = docValues->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
      shared_ptr<BytesRef> *const term = docValues->lookupOrd(ord);
      collectorTerms->add(term);
    }
  }
}

TermsCollector<DV>::SV::SV(
    Function<std::shared_ptr<BinaryDocValues>> docValuesCall)
    : TermsCollector<org::apache::lucene::index::BinaryDocValues>(docValuesCall)
{
}

void TermsCollector<DV>::SV::collect(int doc) 
{
  shared_ptr<BytesRef> term;
  if (docValues->advanceExact(doc)) {
    term = docValues->binaryValue();
  } else {
    term = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
  }
  collectorTerms->add(term);
}
} // namespace org::apache::lucene::search::join