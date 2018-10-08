using namespace std;

#include "JustCompileSearch.h"

namespace org::apache::lucene::search
{
using FieldInvertState = org::apache::lucene::index::FieldInvertState;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const wstring JustCompileSearch::UNSUPPORTED_MSG =
    L"unsupported: used for back-compat testing only !";

void JustCompileSearch::JustCompileCollector::collect(int doc)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

void JustCompileSearch::JustCompileCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

void JustCompileSearch::JustCompileCollector::setScorer(
    shared_ptr<Scorer> scorer)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

bool JustCompileSearch::JustCompileCollector::needsScores()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<DocIdSetIterator> JustCompileSearch::JustCompileDocIdSet::iterator()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int64_t JustCompileSearch::JustCompileDocIdSet::ramBytesUsed() { return 0LL; }

int JustCompileSearch::JustCompileDocIdSetIterator::docID()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearch::JustCompileDocIdSetIterator::nextDoc()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearch::JustCompileDocIdSetIterator::advance(int target)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int64_t JustCompileSearch::JustCompileDocIdSetIterator::cost()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

void JustCompileSearch::JustCompileFieldComparator::setTopValue(any value)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

any JustCompileSearch::JustCompileFieldComparator::value(int slot)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<LeafFieldComparator>
JustCompileSearch::JustCompileFieldComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearch::JustCompileFieldComparator::compare(int slot1, int slot2)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: @Override public FieldComparator<?> newComparator(std::wstring
// fieldname, int numHits, int sortPos, bool reversed)
shared_ptr < FieldComparator <
    ? >> JustCompileSearch::JustCompileFieldComparatorSource::newComparator(
             const wstring &fieldname, int numHits, int sortPos, bool reversed)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

JustCompileSearch::JustCompileFilteredDocIdSetIterator::
    JustCompileFilteredDocIdSetIterator(shared_ptr<DocIdSetIterator> innerIter)
    : FilteredDocIdSetIterator(innerIter)
{
}

bool JustCompileSearch::JustCompileFilteredDocIdSetIterator::match(int doc)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int64_t JustCompileSearch::JustCompileFilteredDocIdSetIterator::cost()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

wstring JustCompileSearch::JustCompileQuery::toString(const wstring &field)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

bool JustCompileSearch::JustCompileQuery::equals(any obj)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearch::JustCompileQuery::hashCode()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

JustCompileSearch::JustCompileScorer::JustCompileScorer(
    shared_ptr<Weight> weight)
    : Scorer(weight)
{
}

float JustCompileSearch::JustCompileScorer::score()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearch::JustCompileScorer::docID()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<DocIdSetIterator> JustCompileSearch::JustCompileScorer::iterator()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<Similarity::SimWeight>
JustCompileSearch::JustCompileSimilarity::computeWeight(
    float boost, shared_ptr<CollectionStatistics> collectionStats,
    deque<TermStatistics> &termStats)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<Similarity::SimScorer>
JustCompileSearch::JustCompileSimilarity::simScorer(
    shared_ptr<Similarity::SimWeight> stats,
    shared_ptr<LeafReaderContext> context)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int64_t JustCompileSearch::JustCompileSimilarity::computeNorm(
    shared_ptr<FieldInvertState> state)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

JustCompileSearch::JustCompileTopDocsCollector::JustCompileTopDocsCollector(
    shared_ptr<PriorityQueue<std::shared_ptr<ScoreDoc>>> pq)
    : TopDocsCollector<ScoreDoc>(pq)
{
}

shared_ptr<LeafCollector>
JustCompileSearch::JustCompileTopDocsCollector::getLeafCollector(
    shared_ptr<LeafReaderContext> context) 
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<TopDocs> JustCompileSearch::JustCompileTopDocsCollector::topDocs()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<TopDocs>
JustCompileSearch::JustCompileTopDocsCollector::topDocs(int start)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<TopDocs>
JustCompileSearch::JustCompileTopDocsCollector::topDocs(int start, int end)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

bool JustCompileSearch::JustCompileTopDocsCollector::needsScores()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

JustCompileSearch::JustCompileWeight::JustCompileWeight() : Weight(nullptr) {}

void JustCompileSearch::JustCompileWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<Explanation> JustCompileSearch::JustCompileWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<Scorer> JustCompileSearch::JustCompileWeight::scorer(
    shared_ptr<LeafReaderContext> context)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

bool JustCompileSearch::JustCompileWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}
} // namespace org::apache::lucene::search