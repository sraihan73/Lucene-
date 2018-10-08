using namespace std;

#include "IDFValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using namespace org::apache::lucene::index;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PerFieldSimilarityWrapper =
    org::apache::lucene::search::similarities::PerFieldSimilarityWrapper;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using BytesRef = org::apache::lucene::util::BytesRef;

IDFValueSource::IDFValueSource(const wstring &field, const wstring &val,
                               const wstring &indexedField,
                               shared_ptr<BytesRef> indexedBytes)
    : DocFreqValueSource(field, val, indexedField, indexedBytes)
{
}

wstring IDFValueSource::name() { return L"idf"; }

shared_ptr<FunctionValues> IDFValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  shared_ptr<TFIDFSimilarity> sim =
      asTFIDF(searcher->getSimilarity(true), field);
  if (sim == nullptr) {
    throw make_shared<UnsupportedOperationException>(
        L"requires a TFIDFSimilarity (such as ClassicSimilarity)");
  }
  int docfreq = searcher->getIndexReader()->docFreq(
      make_shared<Term>(indexedField, indexedBytes));
  float idf = sim->idf(docfreq, searcher->getIndexReader()->maxDoc());
  return make_shared<ConstDoubleDocValues>(idf, shared_from_this());
}

shared_ptr<TFIDFSimilarity> IDFValueSource::asTFIDF(shared_ptr<Similarity> sim,
                                                    const wstring &field)
{
  while (std::dynamic_pointer_cast<PerFieldSimilarityWrapper>(sim) != nullptr) {
    sim =
        (std::static_pointer_cast<PerFieldSimilarityWrapper>(sim))->get(field);
  }
  if (std::dynamic_pointer_cast<TFIDFSimilarity>(sim) != nullptr) {
    return std::static_pointer_cast<TFIDFSimilarity>(sim);
  } else {
    return nullptr;
  }
}
} // namespace org::apache::lucene::queries::function::valuesource