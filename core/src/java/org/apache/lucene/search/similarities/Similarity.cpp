using namespace std;

#include "Similarity.h"

namespace org::apache::lucene::search::similarities
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using SmallFloat = org::apache::lucene::util::SmallFloat;

Similarity::Similarity() {}

Similarity::SimScorer::SimScorer() {}

shared_ptr<Explanation>
Similarity::SimScorer::explain(int doc,
                               shared_ptr<Explanation> freq) 
{
  return Explanation::match(score(doc, freq->getValue()),
                            L"score(doc=" + to_wstring(doc) + L",freq=" +
                                to_wstring(freq->getValue()) +
                                L"), with freq of:",
                            Collections::singleton(freq));
}

Similarity::SimWeight::SimWeight() {}
} // namespace org::apache::lucene::search::similarities