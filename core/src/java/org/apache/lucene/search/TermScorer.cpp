using namespace std;

#include "TermScorer.h"

namespace org::apache::lucene::search
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;

TermScorer::TermScorer(shared_ptr<Weight> weight, shared_ptr<PostingsEnum> td,
                       shared_ptr<Similarity::SimScorer> docScorer)
    : Scorer(weight), postingsEnum(td), docScorer(docScorer)
{
}

int TermScorer::docID() { return postingsEnum->docID(); }

int TermScorer::freq()  { return postingsEnum->freq(); }

shared_ptr<DocIdSetIterator> TermScorer::iterator() { return postingsEnum; }

float TermScorer::score() 
{
  assert(docID() != DocIdSetIterator::NO_MORE_DOCS);
  return docScorer->score(postingsEnum->docID(), postingsEnum->freq());
}

wstring TermScorer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"scorer(" + weight + L")[" + Scorer::toString() + L"]";
}
} // namespace org::apache::lucene::search