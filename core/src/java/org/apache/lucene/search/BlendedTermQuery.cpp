using namespace std;

#include "BlendedTermQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using InPlaceMergeSorter = org::apache::lucene::util::InPlaceMergeSorter;

BlendedTermQuery::Builder::Builder() {}

shared_ptr<Builder> BlendedTermQuery::Builder::setRewriteMethod(
    shared_ptr<RewriteMethod> rewiteMethod)
{
  this->rewriteMethod = rewiteMethod;
  return shared_from_this();
}

shared_ptr<Builder> BlendedTermQuery::Builder::add(shared_ptr<Term> term)
{
  return add(term, 1.0f);
}

shared_ptr<Builder> BlendedTermQuery::Builder::add(shared_ptr<Term> term,
                                                   float boost)
{
  return add(term, boost, nullptr);
}

shared_ptr<Builder>
BlendedTermQuery::Builder::add(shared_ptr<Term> term, float boost,
                               shared_ptr<TermContext> context)
{
  if (numTerms >= BooleanQuery::getMaxClauseCount()) {
    throw make_shared<BooleanQuery::TooManyClauses>();
  }
  terms = ArrayUtil::grow(terms, numTerms + 1);
  boosts = ArrayUtil::grow(boosts, numTerms + 1);
  contexts = ArrayUtil::grow(contexts, numTerms + 1);
  terms[numTerms] = term;
  boosts[numTerms] = boost;
  contexts[numTerms] = context;
  numTerms += 1;
  return shared_from_this();
}

shared_ptr<BlendedTermQuery> BlendedTermQuery::Builder::build()
{
  return make_shared<BlendedTermQuery>(
      Arrays::copyOf(terms, numTerms), Arrays::copyOf(boosts, numTerms),
      Arrays::copyOf(contexts, numTerms), rewriteMethod);
}

BlendedTermQuery::RewriteMethod::RewriteMethod() {}

const shared_ptr<RewriteMethod> BlendedTermQuery::BOOLEAN_REWRITE =
    make_shared<RewriteMethodAnonymousInnerClass>();

BlendedTermQuery::RewriteMethodAnonymousInnerClass::
    RewriteMethodAnonymousInnerClass()
{
}

shared_ptr<Query> BlendedTermQuery::RewriteMethodAnonymousInnerClass::rewrite(
    std::deque<std::shared_ptr<Query>> &subQueries)
{
  shared_ptr<BooleanQuery::Builder> merged =
      make_shared<BooleanQuery::Builder>();
  for (auto query : subQueries) {
    merged->add(query, Occur::SHOULD);
  }
  return merged->build();
}

BlendedTermQuery::DisjunctionMaxRewrite::DisjunctionMaxRewrite(
    float tieBreakerMultiplier)
    : tieBreakerMultiplier(tieBreakerMultiplier)
{
}

shared_ptr<Query> BlendedTermQuery::DisjunctionMaxRewrite::rewrite(
    std::deque<std::shared_ptr<Query>> &subQueries)
{
  return make_shared<DisjunctionMaxQuery>(Arrays::asList(subQueries),
                                          tieBreakerMultiplier);
}

bool BlendedTermQuery::DisjunctionMaxRewrite::equals(any obj)
{
  if (obj == nullptr || getClass() != obj.type()) {
    return false;
  }
  shared_ptr<DisjunctionMaxRewrite> that =
      any_cast<std::shared_ptr<DisjunctionMaxRewrite>>(obj);
  return tieBreakerMultiplier == that->tieBreakerMultiplier;
}

int BlendedTermQuery::DisjunctionMaxRewrite::hashCode()
{
  return 31 * getClass().hashCode() +
         Float::floatToIntBits(tieBreakerMultiplier);
}

const shared_ptr<RewriteMethod> BlendedTermQuery::DISJUNCTION_MAX_REWRITE =
    make_shared<DisjunctionMaxRewrite>(0.01f);

BlendedTermQuery::BlendedTermQuery(
    std::deque<std::shared_ptr<Term>> &terms, std::deque<float> &boosts,
    std::deque<std::shared_ptr<TermContext>> &contexts,
    shared_ptr<RewriteMethod> rewriteMethod)
    : terms(terms), boosts(boosts), contexts(contexts),
      rewriteMethod(rewriteMethod)
{
  assert(terms.size() == boosts.size());
  assert(terms.size() == contexts.size());

  // we sort terms so that equals/hashcode does not rely on the order
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  make_shared<InPlaceMergeSorterAnonymousInnerClass>(shared_from_this(), terms,
                                                     boosts, contexts)
      .sort(0, terms.size());
}

BlendedTermQuery::InPlaceMergeSorterAnonymousInnerClass::
    InPlaceMergeSorterAnonymousInnerClass(
        shared_ptr<BlendedTermQuery> outerInstance,
        deque<std::shared_ptr<Term>> &terms, deque<float> &boosts,
        deque<std::shared_ptr<TermContext>> &contexts)
{
  this->outerInstance = outerInstance;
  this->terms = terms;
  this->boosts = boosts;
  this->contexts = contexts;
}

void BlendedTermQuery::InPlaceMergeSorterAnonymousInnerClass::swap(int i, int j)
{
  shared_ptr<Term> tmpTerm = terms[i];
  terms[i] = terms[j];
  terms[j] = tmpTerm;

  shared_ptr<TermContext> tmpContext = contexts[i];
  contexts[i] = contexts[j];
  contexts[j] = tmpContext;

  float tmpBoost = boosts[i];
  boosts[i] = boosts[j];
  boosts[j] = tmpBoost;
}

int BlendedTermQuery::InPlaceMergeSorterAnonymousInnerClass::compare(int i,
                                                                     int j)
{
  return terms[i]->compareTo(terms[j]);
}

bool BlendedTermQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool BlendedTermQuery::equalsTo(shared_ptr<BlendedTermQuery> other)
{
  return Arrays::equals(terms, other->terms) &&
         Arrays::equals(contexts, other->contexts) &&
         Arrays::equals(boosts, other->boosts) &&
         rewriteMethod->equals(other->rewriteMethod);
}

int BlendedTermQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + Arrays::hashCode(terms);
  h = 31 * h + Arrays::hashCode(contexts);
  h = 31 * h + Arrays::hashCode(boosts);
  h = 31 * h + rewriteMethod->hashCode();
  return h;
}

wstring BlendedTermQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>(L"Blended(");
  for (int i = 0; i < terms.size(); ++i) {
    if (i != 0) {
      builder->append(L" ");
    }
    shared_ptr<Query> termQuery = make_shared<TermQuery>(terms[i]);
    if (boosts[i] != 1.0f) {
      termQuery = make_shared<BoostQuery>(termQuery, boosts[i]);
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->append(termQuery->toString(field));
  }
  builder->append(L")");
  return builder->toString();
}

shared_ptr<Query>
BlendedTermQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  std::deque<std::shared_ptr<TermContext>> contexts =
      Arrays::copyOf(this->contexts, this->contexts.size());
  for (int i = 0; i < contexts.size(); ++i) {
    if (contexts[i] == nullptr ||
        contexts[i]->wasBuiltFor(reader->getContext()) == false) {
      contexts[i] = TermContext::build(reader->getContext(), terms[i]);
    }
  }

  // Compute aggregated doc freq and total term freq
  // df will be the max of all doc freqs
  // ttf will be the sum of all total term freqs
  int df = 0;
  int64_t ttf = 0;
  for (auto ctx : contexts) {
    df = max(df, ctx->docFreq());
    if (ctx->totalTermFreq() == -1LL) {
      ttf = -1LL;
    } else if (ttf != -1LL) {
      ttf += ctx->totalTermFreq();
    }
  }

  for (int i = 0; i < contexts.size(); ++i) {
    contexts[i] = adjustFrequencies(reader->getContext(), contexts[i], df, ttf);
  }

  std::deque<std::shared_ptr<Query>> termQueries(terms.size());
  for (int i = 0; i < terms.size(); ++i) {
    termQueries[i] = make_shared<TermQuery>(terms[i], contexts[i]);
    if (boosts[i] != 1.0f) {
      termQueries[i] = make_shared<BoostQuery>(termQueries[i], boosts[i]);
    }
  }
  return rewriteMethod->rewrite(termQueries);
}

shared_ptr<TermContext> BlendedTermQuery::adjustFrequencies(
    shared_ptr<IndexReaderContext> readerContext, shared_ptr<TermContext> ctx,
    int artificialDf, int64_t artificialTtf)
{
  deque<std::shared_ptr<LeafReaderContext>> leaves = readerContext->leaves();
  constexpr int len;
  if (leaves.empty()) {
    len = 1;
  } else {
    len = leaves.size();
  }
  shared_ptr<TermContext> newCtx = make_shared<TermContext>(readerContext);
  for (int i = 0; i < len; ++i) {
    shared_ptr<TermState> termState = ctx->get(i);
    if (termState == nullptr) {
      continue;
    }
    newCtx->register_(termState, i);
  }
  newCtx->accumulateStatistics(artificialDf, artificialTtf);
  return newCtx;
}
} // namespace org::apache::lucene::search