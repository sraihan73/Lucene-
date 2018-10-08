using namespace std;

#include "SpanNearQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

SpanNearQuery::Builder::Builder(const wstring &field, bool ordered)
    : ordered(ordered), field(field)
{
}

shared_ptr<Builder>
SpanNearQuery::Builder::addClause(shared_ptr<SpanQuery> clause)
{
  if (Objects::equals(clause->getField(), field) == false) {
    throw invalid_argument(L"Cannot add clause " + clause +
                           L" to SpanNearQuery for field " + field);
  }
  this->clauses.push_back(clause);
  return shared_from_this();
}

shared_ptr<Builder> SpanNearQuery::Builder::addGap(int width)
{
  if (!ordered) {
    throw invalid_argument(L"Gaps can only be added to ordered near queries");
  }
  this->clauses.push_back(make_shared<SpanGapQuery>(field, width));
  return shared_from_this();
}

shared_ptr<Builder> SpanNearQuery::Builder::setSlop(int slop)
{
  this->slop = slop;
  return shared_from_this();
}

shared_ptr<SpanNearQuery> SpanNearQuery::Builder::build()
{
  return make_shared<SpanNearQuery>(
      clauses.toArray(std::deque<std::shared_ptr<SpanQuery>>(clauses.size())),
      slop, ordered);
}

shared_ptr<Builder> SpanNearQuery::newOrderedNearQuery(const wstring &field)
{
  return make_shared<Builder>(field, true);
}

shared_ptr<Builder> SpanNearQuery::newUnorderedNearQuery(const wstring &field)
{
  return make_shared<Builder>(field, false);
}

SpanNearQuery::SpanNearQuery(std::deque<std::shared_ptr<SpanQuery>> &clausesIn,
                             int slop, bool inOrder)
{
  this->clauses = deque<>(clausesIn.size());
  for (auto clause : clausesIn) {
    if (this->field == L"") { // check field
      this->field = clause->getField();
    } else if (clause->getField() != L"" && clause->getField() != field) {
      throw invalid_argument(L"Clauses must have same field.");
    }
    this->clauses.push_back(clause);
  }
  this->slop = slop;
  this->inOrder = inOrder;
}

std::deque<std::shared_ptr<SpanQuery>> SpanNearQuery::getClauses()
{
  return clauses.toArray(
      std::deque<std::shared_ptr<SpanQuery>>(clauses.size()));
}

int SpanNearQuery::getSlop() { return slop; }

bool SpanNearQuery::isInOrder() { return inOrder; }

wstring SpanNearQuery::getField() { return field; }

wstring SpanNearQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(L"spanNear([");
  deque<std::shared_ptr<SpanQuery>>::const_iterator i = clauses.begin();
  while (i != clauses.end()) {
    shared_ptr<SpanQuery> clause = *i;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(clause->toString(field));
    if ((*i)->hasNext()) {
      buffer->append(L", ");
    }
    i++;
  }
  buffer->append(L"], ");
  buffer->append(slop);
  buffer->append(L", ");
  buffer->append(inOrder);
  buffer->append(L")");
  return buffer->toString();
}

shared_ptr<SpanWeight>
SpanNearQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  deque<std::shared_ptr<SpanWeight>> subWeights =
      deque<std::shared_ptr<SpanWeight>>();
  for (auto q : clauses) {
    subWeights.push_back(q->createWeight(searcher, false, boost));
  }
  return make_shared<SpanNearWeight>(
      shared_from_this(), subWeights, searcher,
      needsScores ? getTermContexts(subWeights) : nullptr, boost);
}

SpanNearQuery::SpanNearWeight::SpanNearWeight(
    shared_ptr<SpanNearQuery> outerInstance,
    deque<std::shared_ptr<SpanWeight>> &subWeights,
    shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    float boost) 
    : SpanWeight(SpanNearQuery::this, searcher, terms, boost),
      subWeights(subWeights), outerInstance(outerInstance)
{
}

void SpanNearQuery::SpanNearWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  for (auto w : subWeights) {
    w->extractTermContexts(contexts);
  }
}

shared_ptr<Spans> SpanNearQuery::SpanNearWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{

  shared_ptr<Terms> terms = context->reader()->terms(field);
  if (terms == nullptr) {
    return nullptr; // field does not exist
  }

  deque<std::shared_ptr<Spans>> subSpans =
      deque<std::shared_ptr<Spans>>(outerInstance->clauses.size());
  for (auto w : subWeights) {
    shared_ptr<Spans> subSpan = w->getSpans(context, requiredPostings);
    if (subSpan != nullptr) {
      subSpans.push_back(subSpan);
    } else {
      return nullptr; // all required
    }
  }

  // all NearSpans require at least two subSpans
  return (!outerInstance->inOrder)
             ? make_shared<NearSpansUnordered>(outerInstance->slop, subSpans)
             : make_shared<NearSpansOrdered>(outerInstance->slop, subSpans);
}

void SpanNearQuery::SpanNearWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto w : subWeights) {
    w->extractTerms(terms);
  }
}

bool SpanNearQuery::SpanNearWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  for (auto w : subWeights) {
    if (w->isCacheable(ctx) == false) {
      return false;
    }
  }
  return true;
}

shared_ptr<Query>
SpanNearQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  bool actuallyRewritten = false;
  deque<std::shared_ptr<SpanQuery>> rewrittenClauses =
      deque<std::shared_ptr<SpanQuery>>();
  for (int i = 0; i < clauses.size(); i++) {
    shared_ptr<SpanQuery> c = clauses[i];
    shared_ptr<SpanQuery> query =
        std::static_pointer_cast<SpanQuery>(c->rewrite(reader));
    actuallyRewritten |= query != c;
    rewrittenClauses.push_back(query);
  }
  if (actuallyRewritten) {
    try {
      shared_ptr<SpanNearQuery> rewritten =
          std::static_pointer_cast<SpanNearQuery>(clone());
      rewritten->clauses = rewrittenClauses;
      return rewritten;
    } catch (const CloneNotSupportedException &e) {
      throw make_shared<AssertionError>(e);
    }
  }
  return SpanQuery::rewrite(reader);
}

bool SpanNearQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool SpanNearQuery::equalsTo(shared_ptr<SpanNearQuery> other)
{
  return inOrder == other->inOrder && slop == other->slop &&
         clauses.equals(other->clauses);
}

int SpanNearQuery::hashCode()
{
  int result = classHash();
  result ^= clauses.hashCode();
  result += slop;
  int fac = 1 + (inOrder ? 8 : 4);
  return fac * result;
}

SpanNearQuery::SpanGapQuery::SpanGapQuery(const wstring &field, int width)
    : field(field), width(width)
{
}

wstring SpanNearQuery::SpanGapQuery::getField() { return field; }

wstring SpanNearQuery::SpanGapQuery::toString(const wstring &field)
{
  return L"SpanGap(" + field + L":" + to_wstring(width) + L")";
}

shared_ptr<SpanWeight>
SpanNearQuery::SpanGapQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                          bool needsScores,
                                          float boost) 
{
  return make_shared<SpanGapWeight>(shared_from_this(), searcher, boost);
}

SpanNearQuery::SpanGapQuery::SpanGapWeight::SpanGapWeight(
    shared_ptr<SpanNearQuery::SpanGapQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher, float boost) 
    : SpanWeight(SpanGapQuery::this, searcher, nullptr, boost),
      outerInstance(outerInstance)
{
}

void SpanNearQuery::SpanGapQuery::SpanGapWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
}

shared_ptr<Spans> SpanNearQuery::SpanGapQuery::SpanGapWeight::getSpans(
    shared_ptr<LeafReaderContext> ctx,
    Postings requiredPostings) 
{
  return make_shared<GapSpans>(outerInstance->width);
}

void SpanNearQuery::SpanGapQuery::SpanGapWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

bool SpanNearQuery::SpanGapQuery::SpanGapWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

bool SpanNearQuery::SpanGapQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool SpanNearQuery::SpanGapQuery::equalsTo(shared_ptr<SpanGapQuery> other)
{
  return width == other->width && field == other->field;
}

int SpanNearQuery::SpanGapQuery::hashCode()
{
  int result = classHash();
  result -= 7 * width;
  return result * 15 - field.hashCode();
}

SpanNearQuery::GapSpans::GapSpans(int width) : width(width) {}

int SpanNearQuery::GapSpans::nextStartPosition() 
{
  return ++pos;
}

int SpanNearQuery::GapSpans::skipToPosition(int position) 
{
  return pos = position;
}

int SpanNearQuery::GapSpans::startPosition() { return pos; }

int SpanNearQuery::GapSpans::endPosition() { return pos + width_; }

int SpanNearQuery::GapSpans::width() { return width_; }

void SpanNearQuery::GapSpans::collect(
    shared_ptr<SpanCollector> collector) 
{
}

int SpanNearQuery::GapSpans::docID() { return doc; }

int SpanNearQuery::GapSpans::nextDoc() 
{
  pos = -1;
  return ++doc;
}

int SpanNearQuery::GapSpans::advance(int target) 
{
  pos = -1;
  return doc = target;
}

int64_t SpanNearQuery::GapSpans::cost() { return 0; }

float SpanNearQuery::GapSpans::positionsCost() { return 0; }
} // namespace org::apache::lucene::search::spans