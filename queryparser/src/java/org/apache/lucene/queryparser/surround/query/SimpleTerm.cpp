using namespace std;

#include "SimpleTerm.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;

SimpleTerm::SimpleTerm(bool q) { quoted = q; }

bool SimpleTerm::isQuoted() { return quoted; }

wstring SimpleTerm::getQuote() { return L"\""; }

wstring SimpleTerm::getFieldOperator() { return L"/"; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Deprecated public int compareTo(SimpleTerm ost)
int SimpleTerm::compareTo(shared_ptr<SimpleTerm> ost)
{
  /* for ordering terms and prefixes before using an index, not used */
  return this->toStringUnquoted().compare(ost->toStringUnquoted());
}

void SimpleTerm::suffixToString(shared_ptr<StringBuilder> r) {
} // override for prefix query

wstring SimpleTerm::toString()
{
  shared_ptr<StringBuilder> r = make_shared<StringBuilder>();
  if (isQuoted()) {
    r->append(getQuote());
  }
  r->append(toStringUnquoted());
  if (isQuoted()) {
    r->append(getQuote());
  }
  suffixToString(r);
  weightToString(r);
  return r->toString();
}

wstring SimpleTerm::distanceSubQueryNotAllowed() { return L""; }

void SimpleTerm::addSpanQueries(shared_ptr<SpanNearClauseFactory> sncf) throw(
    IOException)
{
  visitMatchingTerms(sncf->getIndexReader(), sncf->getFieldName(),
                     make_shared<MatchingTermVisitorAnonymousInnerClass>(
                         shared_from_this(), sncf));
}

SimpleTerm::MatchingTermVisitorAnonymousInnerClass::
    MatchingTermVisitorAnonymousInnerClass(
        shared_ptr<SimpleTerm> outerInstance,
        shared_ptr<org::apache::lucene::queryparser::surround::query::
                       SpanNearClauseFactory>
            sncf)
{
  this->outerInstance = outerInstance;
  this->sncf = sncf;
}

void SimpleTerm::MatchingTermVisitorAnonymousInnerClass::visitMatchingTerm(
    shared_ptr<Term> term) 
{
  sncf->addTermWeighted(term, outerInstance->getWeight());
}

shared_ptr<Query>
SimpleTerm::makeLuceneQueryFieldNoBoost(const wstring &fieldName,
                                        shared_ptr<BasicQueryFactory> qf)
{
  return make_shared<SimpleTermRewriteQuery>(shared_from_this(), fieldName, qf);
}
} // namespace org::apache::lucene::queryparser::surround::query