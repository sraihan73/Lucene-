using namespace std;

#include "ScoringRewrite.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using RewriteMethod =
    org::apache::lucene::search::MultiTermQuery::RewriteMethod;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using DirectBytesStartArray =
    org::apache::lucene::util::BytesRefHash::DirectBytesStartArray;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

ScoringRewrite<
    B>::ScoringRewriteAnonymousInnerClass::ScoringRewriteAnonymousInnerClass()
{
}

shared_ptr<BooleanQuery::Builder>
ScoringRewrite<B>::ScoringRewriteAnonymousInnerClass::getTopLevelBuilder()
{
  return make_shared<BooleanQuery::Builder>();
}

shared_ptr<Query> ScoringRewrite<B>::ScoringRewriteAnonymousInnerClass::build(
    shared_ptr<BooleanQuery::Builder> builder)
{
  return builder->build();
}

void ScoringRewrite<B>::ScoringRewriteAnonymousInnerClass::addClause(
    shared_ptr<BooleanQuery::Builder> topLevel, shared_ptr<Term> term,
    int docCount, float boost, shared_ptr<TermContext> states)
{
  shared_ptr<TermQuery> *const tq = make_shared<TermQuery>(term, states);
  topLevel->add(make_shared<BoostQuery>(tq, boost),
                BooleanClause::Occur::SHOULD);
}

void ScoringRewrite<B>::ScoringRewriteAnonymousInnerClass::checkMaxClauseCount(
    int count)
{
  if (count > BooleanQuery::getMaxClauseCount()) {
    throw make_shared<BooleanQuery::TooManyClauses>();
  }
}

ScoringRewrite<
    B>::RewriteMethodAnonymousInnerClass::RewriteMethodAnonymousInnerClass()
{
}

shared_ptr<Query> ScoringRewrite<B>::RewriteMethodAnonymousInnerClass::rewrite(
    shared_ptr<IndexReader> reader,
    shared_ptr<MultiTermQuery> query) 
{
  shared_ptr<Query> *const bq = SCORING_BOOLEAN_REWRITE::rewrite(reader, query);
  // strip the scores off
  return make_shared<ConstantScoreQuery>(bq);
}

ScoringRewrite<B>::ParallelArraysTermCollector::ParallelArraysTermCollector(
    shared_ptr<ScoringRewrite<B>> outerInstance)
    : outerInstance(outerInstance)
{
}

void ScoringRewrite<B>::ParallelArraysTermCollector::setNextEnum(
    shared_ptr<TermsEnum> termsEnum)
{
  this->termsEnum = termsEnum;
  this->boostAtt =
      termsEnum->attributes()->addAttribute(BoostAttribute::typeid);
}

bool ScoringRewrite<B>::ParallelArraysTermCollector::collect(
    shared_ptr<BytesRef> bytes) 
{
  constexpr int e = terms->add(bytes);
  shared_ptr<TermState> *const state = termsEnum->termState();
  assert(state != nullptr);
  if (e < 0) {
    // duplicate term: update docFreq
    constexpr int pos = (-e) - 1;
    array_->termState[pos]->register_(state, readerContext->ord,
                                      termsEnum->docFreq(),
                                      termsEnum->totalTermFreq());
    assert((array_->boost[pos] == boostAtt->getBoost(),
            L"boost should be equal in all segment TermsEnums"));
  } else {
    // new entry: we populate the entry initially
    array_->boost[e] = boostAtt->getBoost();
    array_->termState[e] = make_shared<TermContext>(
        topReaderContext, state, readerContext->ord, termsEnum->docFreq(),
        termsEnum->totalTermFreq());
    outerInstance->checkMaxClauseCount(terms->size());
  }
  return true;
}

ScoringRewrite<B>::TermFreqBoostByteStart::TermFreqBoostByteStart(int initSize)
    : org::apache::lucene::util::BytesRefHash::DirectBytesStartArray(initSize)
{
}

std::deque<int> ScoringRewrite<B>::TermFreqBoostByteStart::init()
{
  const std::deque<int> ord = DirectBytesStartArray::init();
  boost = std::deque<float>(ArrayUtil::oversize(ord.size(), Float::BYTES));
  termState = std::deque<std::shared_ptr<TermContext>>(
      ArrayUtil::oversize(ord.size(), RamUsageEstimator::NUM_BYTES_OBJECT_REF));
  assert(termState.size() >= ord.size() && boost.size() >= ord.size());
  return ord;
}

std::deque<int> ScoringRewrite<B>::TermFreqBoostByteStart::grow()
{
  const std::deque<int> ord = DirectBytesStartArray::grow();
  boost = ArrayUtil::grow(boost, ord.size());
  if (termState.size() < ord.size()) {
    std::deque<std::shared_ptr<TermContext>> tmpTermState(ArrayUtil::oversize(
        ord.size(), RamUsageEstimator::NUM_BYTES_OBJECT_REF));
    System::arraycopy(termState, 0, tmpTermState, 0, termState.size());
    termState = tmpTermState;
  }
  assert(termState.size() >= ord.size() && boost.size() >= ord.size());
  return ord;
}

std::deque<int> ScoringRewrite<B>::TermFreqBoostByteStart::clear()
{
  boost.clear();
  termState.clear();
  return DirectBytesStartArray::clear();
}
} // namespace org::apache::lucene::search