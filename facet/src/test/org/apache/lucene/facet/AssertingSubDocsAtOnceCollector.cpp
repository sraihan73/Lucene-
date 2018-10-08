using namespace std;

#include "AssertingSubDocsAtOnceCollector.h"

namespace org::apache::lucene::facet
{
using Scorer = org::apache::lucene::search::Scorer;
using ChildScorer = org::apache::lucene::search::Scorer::ChildScorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;

void AssertingSubDocsAtOnceCollector::setScorer(shared_ptr<Scorer> s) throw(
    IOException)
{
  // Gathers all scorers, including s and "under":
  allScorers = deque<>();
  allScorers.push_back(s);
  int upto = 0;
  while (upto < allScorers.size()) {
    s = allScorers[upto++];
    for (auto sub : s->getChildren()) {
      allScorers.push_back(sub->child);
    }
  }
}

void AssertingSubDocsAtOnceCollector::collect(int docID)
{
  for (auto s : allScorers) {
    if (docID != s->docID()) {
      throw make_shared<IllegalStateException>(
          L"subScorer=" + s + L" has docID=" + to_wstring(s->docID()) +
          L" != collected docID=" + to_wstring(docID));
    }
  }
}

bool AssertingSubDocsAtOnceCollector::needsScores() { return false; }
} // namespace org::apache::lucene::facet