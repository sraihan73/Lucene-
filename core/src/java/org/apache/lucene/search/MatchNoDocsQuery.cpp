using namespace std;

#include "MatchNoDocsQuery.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

MatchNoDocsQuery::MatchNoDocsQuery() : MatchNoDocsQuery(L"") {}

MatchNoDocsQuery::MatchNoDocsQuery(const wstring &reason) : reason(reason) {}

shared_ptr<Weight>
MatchNoDocsQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                               bool needsScores, float boost) 
{
  return make_shared<WeightAnonymousInnerClass>(shared_from_this());
}

MatchNoDocsQuery::WeightAnonymousInnerClass::WeightAnonymousInnerClass(
    shared_ptr<MatchNoDocsQuery> outerInstance)
    : Weight(outerInstance)
{
  this->outerInstance = outerInstance;
}

void MatchNoDocsQuery::WeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
}

shared_ptr<Explanation> MatchNoDocsQuery::WeightAnonymousInnerClass::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  return Explanation::noMatch(outerInstance->reason);
}

shared_ptr<Scorer> MatchNoDocsQuery::WeightAnonymousInnerClass::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  return nullptr;
}

bool MatchNoDocsQuery::WeightAnonymousInnerClass::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

wstring MatchNoDocsQuery::toString(const wstring &field)
{
  return L"MatchNoDocsQuery(\"" + reason + L"\")";
}

bool MatchNoDocsQuery::equals(any o) { return sameClassAs(o); }

int MatchNoDocsQuery::hashCode() { return classHash(); }
} // namespace org::apache::lucene::search