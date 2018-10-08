using namespace std;

#include "SpanContainQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

SpanContainQuery::SpanContainQuery(shared_ptr<SpanQuery> big,
                                   shared_ptr<SpanQuery> little)
{
  this->big = Objects::requireNonNull(big);
  this->little = Objects::requireNonNull(little);
  Objects::requireNonNull(big->getField());
  Objects::requireNonNull(little->getField());
  if (big->getField() != little->getField()) {
    throw invalid_argument(L"big and little not same field");
  }
}

wstring SpanContainQuery::getField() { return big->getField(); }

shared_ptr<SpanQuery> SpanContainQuery::getBig() { return big; }

shared_ptr<SpanQuery> SpanContainQuery::getLittle() { return little; }

SpanContainQuery::SpanContainWeight::SpanContainWeight(
    shared_ptr<SpanContainQuery> outerInstance,
    shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    shared_ptr<SpanWeight> bigWeight, shared_ptr<SpanWeight> littleWeight,
    float boost) 
    : SpanWeight(SpanContainQuery::this, searcher, terms, boost),
      bigWeight(bigWeight), littleWeight(littleWeight),
      outerInstance(outerInstance)
{
}

void SpanContainQuery::SpanContainWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  bigWeight->extractTerms(terms);
  littleWeight->extractTerms(terms);
}

deque<std::shared_ptr<Spans>>
SpanContainQuery::SpanContainWeight::prepareConjunction(
    shared_ptr<LeafReaderContext> context, Postings postings) 
{
  shared_ptr<Spans> bigSpans = bigWeight->getSpans(context, postings);
  if (bigSpans == nullptr) {
    return nullptr;
  }
  shared_ptr<Spans> littleSpans = littleWeight->getSpans(context, postings);
  if (littleSpans == nullptr) {
    return nullptr;
  }
  deque<std::shared_ptr<Spans>> bigAndLittle =
      deque<std::shared_ptr<Spans>>();
  bigAndLittle.push_back(bigSpans);
  bigAndLittle.push_back(littleSpans);
  return bigAndLittle;
}

void SpanContainQuery::SpanContainWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  bigWeight->extractTermContexts(contexts);
  littleWeight->extractTermContexts(contexts);
}

wstring SpanContainQuery::toString(const wstring &field, const wstring &name)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  buffer->append(name);
  buffer->append(L"(");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(big->toString(field));
  buffer->append(L", ");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(little->toString(field));
  buffer->append(L")");
  return buffer->toString();
}

shared_ptr<Query>
SpanContainQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<SpanQuery> rewrittenBig =
      std::static_pointer_cast<SpanQuery>(big->rewrite(reader));
  shared_ptr<SpanQuery> rewrittenLittle =
      std::static_pointer_cast<SpanQuery>(little->rewrite(reader));
  if (big != rewrittenBig || little != rewrittenLittle) {
    try {
      shared_ptr<SpanContainQuery> clone =
          std::static_pointer_cast<SpanContainQuery>(SpanQuery::clone());
      clone->big = rewrittenBig;
      clone->little = rewrittenLittle;
      return clone;
    } catch (const CloneNotSupportedException &e) {
      throw make_shared<AssertionError>(e);
    }
  }
  return SpanQuery::rewrite(reader);
}

bool SpanContainQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool SpanContainQuery::equalsTo(shared_ptr<SpanContainQuery> other)
{
  return big->equals(other->big) && little->equals(other->little);
}

int SpanContainQuery::hashCode()
{
  int h = Integer::rotateLeft(classHash(), 1);
  h ^= big->hashCode();
  h = Integer::rotateLeft(h, 1);
  h ^= little->hashCode();
  return h;
}
} // namespace org::apache::lucene::search::spans