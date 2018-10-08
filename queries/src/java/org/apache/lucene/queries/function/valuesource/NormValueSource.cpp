using namespace std;

#include "NormValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;
using SimWeight =
    org::apache::lucene::search::similarities::Similarity::SimWeight;

NormValueSource::NormValueSource(const wstring &field) : field(field) {}

wstring NormValueSource::name() { return L"norm"; }

wstring NormValueSource::description()
{
  return name() + StringHelper::toString(L'(') + field +
         StringHelper::toString(L')');
}

void NormValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  context.emplace(L"searcher", searcher);
}

shared_ptr<FunctionValues> NormValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  shared_ptr<TFIDFSimilarity> *const similarity =
      IDFValueSource::asTFIDF(searcher->getSimilarity(true), field);
  if (similarity == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"requires a TFIDFSimilarity (such as ClassicSimilarity)");
  }
  // Only works if the contribution of the tf is 1 when the freq is 1 and
  // contribution of the idf is 1 when docCount == docFreq == 1
  shared_ptr<SimWeight> *const simWeight = similarity->computeWeight(
      1.0f, make_shared<CollectionStatistics>(field, 1, 1, 1, 1),
      {make_shared<TermStatistics>(make_shared<BytesRef>(L"bogus"), 1, 1)});
  shared_ptr<SimScorer> *const simScorer =
      similarity->simScorer(simWeight, readerContext);

  return make_shared<FloatDocValuesAnonymousInnerClass>(shared_from_this(),
                                                        simScorer);
}

NormValueSource::FloatDocValuesAnonymousInnerClass::
    FloatDocValuesAnonymousInnerClass(shared_ptr<NormValueSource> outerInstance,
                                      shared_ptr<SimScorer> simScorer)
    : org::apache::lucene::queries::function::docvalues::FloatDocValues(
          outerInstance)
{
  this->outerInstance = outerInstance;
  this->simScorer = simScorer;
  lastDocID = -1;
}

float NormValueSource::FloatDocValuesAnonymousInnerClass::floatVal(
    int docID) 
{
  if (docID < lastDocID) {
    throw make_shared<AssertionError>(L"docs out of order: lastDocID=" +
                                      lastDocID + L" docID=" +
                                      to_wstring(docID));
  }
  lastDocID = docID;
  return simScorer->score(docID, 1.0f);
}

bool NormValueSource::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  return this->field == (any_cast<std::shared_ptr<NormValueSource>>(o)).field;
}

int NormValueSource::hashCode()
{
  return this->getClass().hashCode() + field.hashCode();
}
} // namespace org::apache::lucene::queries::function::valuesource