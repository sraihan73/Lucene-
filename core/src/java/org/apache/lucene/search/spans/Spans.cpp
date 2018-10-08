using namespace std;

#include "Spans.h"

namespace org::apache::lucene::search::spans
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;
using TwoPhaseIterator = org::apache::lucene::search::TwoPhaseIterator;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

shared_ptr<TwoPhaseIterator> Spans::asTwoPhaseIterator() { return nullptr; }

wstring Spans::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  type_info clazz = getClass();
  sb->append(clazz.isAnonymousClass() ? clazz.getName() : clazz.name());
  sb->append(L"(doc=")->append(docID());
  sb->append(L",start=")->append(startPosition());
  sb->append(L",end=")->append(endPosition());
  sb->append(L")");
  return sb->toString();
}

void Spans::doStartCurrentDoc()  {}

void Spans::doCurrentSpans()  {}
} // namespace org::apache::lucene::search::spans