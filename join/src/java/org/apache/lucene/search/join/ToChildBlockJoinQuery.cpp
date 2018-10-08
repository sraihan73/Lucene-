using namespace std;

#include "ToChildBlockJoinQuery.h"

namespace org::apache::lucene::search::join
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FilterWeight = org::apache::lucene::search::FilterWeight;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitSet = org::apache::lucene::util::BitSet;
const wstring ToChildBlockJoinQuery::INVALID_QUERY_MESSAGE =
    wstring(L"Parent query must not match any docs besides parent filter. ") +
    L"Combine them as must (+) and must-not (-) clauses to find a problem doc. "
    L"docID=";
const wstring ToChildBlockJoinQuery::ILLEGAL_ADVANCE_ON_PARENT =
    L"Expect to be advanced on child docs only. got docID=";

ToChildBlockJoinQuery::ToChildBlockJoinQuery(
    shared_ptr<Query> parentQuery, shared_ptr<BitSetProducer> parentsFilter)
    : org::apache::lucene::search::Query(), parentsFilter(parentsFilter),
      parentQuery(parentQuery)
{
}

shared_ptr<Weight>
ToChildBlockJoinQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                    bool needsScores,
                                    float boost) 
{
  return make_shared<ToChildBlockJoinWeight>(
      shared_from_this(),
      parentQuery->createWeight(searcher, needsScores, boost), parentsFilter,
      needsScores);
}

shared_ptr<Query> ToChildBlockJoinQuery::getParentQuery()
{
  return parentQuery;
}

ToChildBlockJoinQuery::ToChildBlockJoinWeight::ToChildBlockJoinWeight(
    shared_ptr<Query> joinQuery, shared_ptr<Weight> parentWeight,
    shared_ptr<BitSetProducer> parentsFilter, bool doScores)
    : org::apache::lucene::search::FilterWeight(joinQuery, parentWeight),
      parentsFilter(parentsFilter), doScores(doScores)
{
}

shared_ptr<Scorer> ToChildBlockJoinQuery::ToChildBlockJoinWeight::scorer(
    shared_ptr<LeafReaderContext> readerContext) 
{

  shared_ptr<Scorer> *const parentScorer = in_->scorer(readerContext);

  if (parentScorer == nullptr) {
    // No matches
    return nullptr;
  }

  // NOTE: this doesn't take acceptDocs into account, the responsibility
  // to not match deleted docs is on the scorer
  shared_ptr<BitSet> *const parents = parentsFilter->getBitSet(readerContext);
  if (parents == nullptr) {
    // No parents
    return nullptr;
  }

  return make_shared<ToChildBlockJoinScorer>(shared_from_this(), parentScorer,
                                             parents, doScores);
}

shared_ptr<Explanation> ToChildBlockJoinQuery::ToChildBlockJoinWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<ToChildBlockJoinScorer> scorer =
      std::static_pointer_cast<ToChildBlockJoinScorer>(this->scorer(context));
  if (scorer != nullptr && scorer->begin().advance(doc) == doc) {
    int parentDoc = scorer->getParentDoc();
    return Explanation::match(
        scorer->score(),
        wstring::format(Locale::ROOT, L"Score based on parent document %d",
                        parentDoc + context->docBase),
        in_->explain(context, parentDoc));
  }
  return Explanation::noMatch(L"Not a match");
}

ToChildBlockJoinQuery::ToChildBlockJoinScorer::ToChildBlockJoinScorer(
    shared_ptr<Weight> weight, shared_ptr<Scorer> parentScorer,
    shared_ptr<BitSet> parentBits, bool doScores)
    : org::apache::lucene::search::Scorer(weight), parentScorer(parentScorer),
      parentIt(parentScorer->begin()), parentBits(parentBits),
      doScores(doScores)
{
}

shared_ptr<deque<std::shared_ptr<Scorer::ChildScorer>>>
ToChildBlockJoinQuery::ToChildBlockJoinScorer::getChildren()
{
  return Collections::singleton(
      make_shared<Scorer::ChildScorer>(parentScorer, L"BLOCK_JOIN"));
}

shared_ptr<DocIdSetIterator>
ToChildBlockJoinQuery::ToChildBlockJoinScorer::iterator()
{
  return make_shared<DocIdSetIteratorAnonymousInnerClass>(shared_from_this());
}

ToChildBlockJoinQuery::ToChildBlockJoinScorer::
    DocIdSetIteratorAnonymousInnerClass::DocIdSetIteratorAnonymousInnerClass(
        shared_ptr<ToChildBlockJoinScorer> outerInstance)
{
  this->outerInstance = outerInstance;
}

int ToChildBlockJoinQuery::ToChildBlockJoinScorer::
    DocIdSetIteratorAnonymousInnerClass::docID()
{
  return outerInstance->childDoc;
}

int ToChildBlockJoinQuery::ToChildBlockJoinScorer::
    DocIdSetIteratorAnonymousInnerClass::nextDoc() 
{
  // System.out.println("Q.nextDoc() parentDoc=" + parentDoc + " childDoc=" +
  // childDoc);

  while (true) {
    if (outerInstance->childDoc + 1 == outerInstance->parentDoc) {
      // OK, we are done iterating through all children
      // matching this one parent doc, so we now nextDoc()
      // the parent.  Use a while loop because we may have
      // to skip over some number of parents w/ no
      // children:
      while (true) {
        outerInstance->parentDoc = outerInstance->parentIt->nextDoc();
        outerInstance->validateParentDoc();

        if (outerInstance->parentDoc == 0) {
          // Degenerate but allowed: first parent doc has no children
          // TODO: would be nice to pull initial parent
          // into ctor so we can skip this if... but it's
          // tricky because scorer must return -1 for
          // .doc() on init...
          outerInstance->parentDoc = outerInstance->parentIt->nextDoc();
          outerInstance->validateParentDoc();
        }

        if (outerInstance->parentDoc == DocIdSetIterator::NO_MORE_DOCS) {
          outerInstance->childDoc = DocIdSetIterator::NO_MORE_DOCS;
          // System.out.println("  END");
          return outerInstance->childDoc;
        }

        // Go to first child for this next parentDoc:
        outerInstance->childDoc = 1 + outerInstance->parentBits->prevSetBit(
                                          outerInstance->parentDoc - 1);

        if (outerInstance->childDoc == outerInstance->parentDoc) {
          // This parent has no children; continue
          // parent loop so we move to next parent
          continue;
        }

        if (outerInstance->childDoc < outerInstance->parentDoc) {
          if (outerInstance->doScores) {
            outerInstance->parentScore = outerInstance->parentScorer->score();
          }
          // System.out.println("  " + childDoc);
          return outerInstance->childDoc;
        } else {
          // Degenerate but allowed: parent has no children
        }
      }
    } else {
      assert((outerInstance->childDoc < outerInstance->parentDoc,
              L"childDoc=" + to_wstring(outerInstance->childDoc) +
                  L" parentDoc=" + to_wstring(outerInstance->parentDoc)));
      outerInstance->childDoc++;
      // System.out.println("  " + childDoc);
      return outerInstance->childDoc;
    }
  }
}

int ToChildBlockJoinQuery::ToChildBlockJoinScorer::
    DocIdSetIteratorAnonymousInnerClass::advance(int childTarget) throw(
        IOException)
{
  if (childTarget >= outerInstance->parentDoc) {
    if (childTarget == DocIdSetIterator::NO_MORE_DOCS) {
      return outerInstance->childDoc = outerInstance->parentDoc =
                 DocIdSetIterator::NO_MORE_DOCS;
    }
    outerInstance->parentDoc =
        outerInstance->parentIt->advance(childTarget + 1);
    outerInstance->validateParentDoc();

    if (outerInstance->parentDoc == DocIdSetIterator::NO_MORE_DOCS) {
      return outerInstance->childDoc = DocIdSetIterator::NO_MORE_DOCS;
    }

    // scan to the first parent that has children
    while (true) {
      constexpr int firstChild =
          outerInstance->parentBits->prevSetBit(outerInstance->parentDoc - 1) +
          1;
      if (firstChild != outerInstance->parentDoc) {
        // this parent has children
        childTarget = max(childTarget, firstChild);
        break;
      }
      // parent with no children, move to the next one
      outerInstance->parentDoc = outerInstance->parentIt->nextDoc();
      outerInstance->validateParentDoc();
      if (outerInstance->parentDoc == DocIdSetIterator::NO_MORE_DOCS) {
        return outerInstance->childDoc = DocIdSetIterator::NO_MORE_DOCS;
      }
    }

    if (outerInstance->doScores) {
      outerInstance->parentScore = outerInstance->parentScorer->score();
    }
  }

  assert(childTarget < outerInstance->parentDoc);
  assert(!outerInstance->parentBits->get(childTarget));
  outerInstance->childDoc = childTarget;
  // System.out.println("  " + childDoc);
  return outerInstance->childDoc;
}

int64_t ToChildBlockJoinQuery::ToChildBlockJoinScorer::
    DocIdSetIteratorAnonymousInnerClass::cost()
{
  return outerInstance->parentIt->cost();
}

void ToChildBlockJoinQuery::ToChildBlockJoinScorer::validateParentDoc()
{
  if (parentDoc != DocIdSetIterator::NO_MORE_DOCS &&
      !parentBits->get(parentDoc)) {
    throw make_shared<IllegalStateException>(INVALID_QUERY_MESSAGE +
                                             to_wstring(parentDoc));
  }
}

int ToChildBlockJoinQuery::ToChildBlockJoinScorer::docID() { return childDoc; }

float ToChildBlockJoinQuery::ToChildBlockJoinScorer::score() 
{
  return parentScore;
}

int ToChildBlockJoinQuery::ToChildBlockJoinScorer::getParentDoc()
{
  return parentDoc;
}

shared_ptr<Query> ToChildBlockJoinQuery::rewrite(
    shared_ptr<IndexReader> reader) 
{
  shared_ptr<Query> *const parentRewrite = parentQuery->rewrite(reader);
  if (parentRewrite != parentQuery) {
    return make_shared<ToChildBlockJoinQuery>(parentRewrite, parentsFilter);
  } else {
    return Query::rewrite(reader);
  }
}

wstring ToChildBlockJoinQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"ToChildBlockJoinQuery (" + parentQuery->toString() + L")";
}

bool ToChildBlockJoinQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool ToChildBlockJoinQuery::equalsTo(shared_ptr<ToChildBlockJoinQuery> other)
{
  return parentQuery->equals(other->parentQuery) &&
         parentsFilter->equals(other->parentsFilter);
}

int ToChildBlockJoinQuery::hashCode()
{
  constexpr int prime = 31;
  int hash = classHash();
  hash = prime * hash + parentQuery->hashCode();
  hash = prime * hash + parentsFilter->hashCode();
  return hash;
}
} // namespace org::apache::lucene::search::join