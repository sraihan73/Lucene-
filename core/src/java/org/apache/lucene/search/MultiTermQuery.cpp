using namespace std;

#include "MultiTermQuery.h"

namespace org::apache::lucene::search
{
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using SingleTermsEnum = org::apache::lucene::index::SingleTermsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Builder = org::apache::lucene::search::BooleanQuery::Builder;
using AttributeSource = org::apache::lucene::util::AttributeSource;

shared_ptr<TermsEnum> MultiTermQuery::RewriteMethod::getTermsEnum(
    shared_ptr<MultiTermQuery> query, shared_ptr<Terms> terms,
    shared_ptr<AttributeSource> atts) 
{
  return query->getTermsEnum(
      terms,
      atts); // allow RewriteMethod subclasses to pull a TermsEnum from the MTQ
}

const shared_ptr<RewriteMethod> MultiTermQuery::CONSTANT_SCORE_REWRITE =
    make_shared<RewriteMethodAnonymousInnerClass>();

MultiTermQuery::RewriteMethodAnonymousInnerClass::
    RewriteMethodAnonymousInnerClass()
{
}

shared_ptr<Query> MultiTermQuery::RewriteMethodAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader, shared_ptr<MultiTermQuery> query)
{
  return make_shared<MultiTermQueryConstantScoreWrapper<>>(query);
}

const shared_ptr<RewriteMethod> MultiTermQuery::SCORING_BOOLEAN_REWRITE =
    ScoringRewrite::SCORING_BOOLEAN_REWRITE;
const shared_ptr<RewriteMethod> MultiTermQuery::CONSTANT_SCORE_BOOLEAN_REWRITE =
    ScoringRewrite::CONSTANT_SCORE_BOOLEAN_REWRITE;

MultiTermQuery::TopTermsScoringBooleanQueryRewrite::
    TopTermsScoringBooleanQueryRewrite(int size)
    : TopTermsRewrite<BooleanQuery::Builder>(size)
{
}

int MultiTermQuery::TopTermsScoringBooleanQueryRewrite::getMaxSize()
{
  return BooleanQuery::getMaxClauseCount();
}

shared_ptr<BooleanQuery::Builder>
MultiTermQuery::TopTermsScoringBooleanQueryRewrite::getTopLevelBuilder()
{
  return make_shared<BooleanQuery::Builder>();
}

shared_ptr<Query> MultiTermQuery::TopTermsScoringBooleanQueryRewrite::build(
    shared_ptr<Builder> builder)
{
  return builder->build();
}

void MultiTermQuery::TopTermsScoringBooleanQueryRewrite::addClause(
    shared_ptr<BooleanQuery::Builder> topLevel, shared_ptr<Term> term,
    int docCount, float boost, shared_ptr<TermContext> states)
{
  shared_ptr<TermQuery> *const tq = make_shared<TermQuery>(term, states);
  topLevel->add(make_shared<BoostQuery>(tq, boost),
                BooleanClause::Occur::SHOULD);
}

MultiTermQuery::TopTermsBlendedFreqScoringRewrite::
    TopTermsBlendedFreqScoringRewrite(int size)
    : TopTermsRewrite<BlendedTermQuery::Builder>(size)
{
}

int MultiTermQuery::TopTermsBlendedFreqScoringRewrite::getMaxSize()
{
  return BooleanQuery::getMaxClauseCount();
}

shared_ptr<BlendedTermQuery::Builder>
MultiTermQuery::TopTermsBlendedFreqScoringRewrite::getTopLevelBuilder()
{
  shared_ptr<BlendedTermQuery::Builder> builder =
      make_shared<BlendedTermQuery::Builder>();
  builder->setRewriteMethod(BlendedTermQuery::BOOLEAN_REWRITE);
  return builder;
}

shared_ptr<Query> MultiTermQuery::TopTermsBlendedFreqScoringRewrite::build(
    shared_ptr<BlendedTermQuery::Builder> builder)
{
  return builder->build();
}

void MultiTermQuery::TopTermsBlendedFreqScoringRewrite::addClause(
    shared_ptr<BlendedTermQuery::Builder> topLevel, shared_ptr<Term> term,
    int docCount, float boost, shared_ptr<TermContext> states)
{
  topLevel->add(term, boost, states);
}

MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite::
    TopTermsBoostOnlyBooleanQueryRewrite(int size)
    : TopTermsRewrite<BooleanQuery::Builder>(size)
{
}

int MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite::getMaxSize()
{
  return BooleanQuery::getMaxClauseCount();
}

shared_ptr<BooleanQuery::Builder>
MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite::getTopLevelBuilder()
{
  return make_shared<BooleanQuery::Builder>();
}

shared_ptr<Query> MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite::build(
    shared_ptr<BooleanQuery::Builder> builder)
{
  return builder->build();
}

void MultiTermQuery::TopTermsBoostOnlyBooleanQueryRewrite::addClause(
    shared_ptr<BooleanQuery::Builder> topLevel, shared_ptr<Term> term,
    int docFreq, float boost, shared_ptr<TermContext> states)
{
  shared_ptr<Query> *const q =
      make_shared<ConstantScoreQuery>(make_shared<TermQuery>(term, states));
  topLevel->add(make_shared<BoostQuery>(q, boost),
                BooleanClause::Occur::SHOULD);
}

MultiTermQuery::MultiTermQuery(const wstring &field)
    : field(Objects::requireNonNull(field, L"field must not be null"))
{
}

wstring MultiTermQuery::getField() { return field; }

shared_ptr<TermsEnum>
MultiTermQuery::getTermsEnum(shared_ptr<Terms> terms) 
{
  return getTermsEnum(terms, make_shared<AttributeSource>());
}

shared_ptr<Query>
MultiTermQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  return rewriteMethod->rewrite(reader, shared_from_this());
}

shared_ptr<RewriteMethod> MultiTermQuery::getRewriteMethod()
{
  return rewriteMethod;
}

void MultiTermQuery::setRewriteMethod(shared_ptr<RewriteMethod> method)
{
  rewriteMethod = method;
}

int MultiTermQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + rewriteMethod->hashCode();
  result = prime * result + field.hashCode();
  return result;
}

bool MultiTermQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool MultiTermQuery::equalsTo(shared_ptr<MultiTermQuery> other)
{
  return rewriteMethod->equals(other->rewriteMethod) && field == other->field;
}
} // namespace org::apache::lucene::search