using namespace std;

#include "TermQuery.h"

namespace org::apache::lucene::search
{
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

TermQuery::TermWeight::TermWeight(
    shared_ptr<TermQuery> outerInstance, shared_ptr<IndexSearcher> searcher,
    bool needsScores, float boost,
    shared_ptr<TermContext> termStates) 
    : Weight(TermQuery::this), similarity(searcher->getSimilarity(needsScores)),
      stats(similarity->computeWeight(boost, collectionStats, {termStats})),
      termStates(termStates), needsScores(needsScores),
      outerInstance(outerInstance)
{
  if (needsScores && termStates == nullptr) {
    throw make_shared<IllegalStateException>(
        L"termStates are required when scores are needed");
  }

  shared_ptr<CollectionStatistics> *const collectionStats;
  shared_ptr<TermStatistics> *const termStats;
  if (needsScores) {
    collectionStats =
        searcher->collectionStatistics(outerInstance->term->field());
    termStats = searcher->termStatistics(outerInstance->term, termStates);
  } else {
    // we do not need the actual stats, use fake stats with docFreq=maxDoc and
    // ttf=-1
    constexpr int maxDoc = searcher->getIndexReader()->maxDoc();
    collectionStats = make_shared<CollectionStatistics>(
        outerInstance->term->field(), maxDoc, -1, -1, -1);
    termStats =
        make_shared<TermStatistics>(outerInstance->term->bytes(), maxDoc, -1);
  }
}

void TermQuery::TermWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  terms->add(outerInstance->getTerm());
}

shared_ptr<Matches>
TermQuery::TermWeight::matches(shared_ptr<LeafReaderContext> context,
                               int doc) 
{
  shared_ptr<TermsEnum> te = getTermsEnum(context);
  if (te == nullptr) {
    return nullptr;
  }
  if (context->reader()->terms(outerInstance->term->field())->hasPositions() ==
      false) {
    return Weight::matches(context, doc);
  }
  return Matches::forField(outerInstance->term->field(), [&]() {
    shared_ptr<PostingsEnum> pe = te->postings(nullptr, PostingsEnum::OFFSETS);
    if (pe->advance(doc) != doc) {
      return nullptr;
    }
    return make_shared<TermMatchesIterator>(pe);
  });
}

wstring TermQuery::TermWeight::toString()
{
  return L"weight(" + outerInstance + L")";
}

shared_ptr<Scorer> TermQuery::TermWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  assert((termStates == nullptr ||
              termStates->wasBuiltFor(ReaderUtil::getTopLevelContext(context)),
          L"The top-reader used to create Weight is not the same as the "
          L"current reader's top-reader (" +
              ReaderUtil::getTopLevelContext(context)));
  shared_ptr<TermsEnum> *const termsEnum = getTermsEnum(context);
  if (termsEnum == nullptr) {
    return nullptr;
  }
  shared_ptr<PostingsEnum> docs = termsEnum->postings(
      nullptr, needsScores ? PostingsEnum::FREQS : PostingsEnum::NONE);
  assert(docs != nullptr);
  return make_shared<TermScorer>(shared_from_this(), docs,
                                 similarity->simScorer(stats, context));
}

bool TermQuery::TermWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

shared_ptr<TermsEnum> TermQuery::TermWeight::getTermsEnum(
    shared_ptr<LeafReaderContext> context) 
{
  if (termStates != nullptr) {
    // TermQuery either used as a Query or the term states have been provided at
    // construction time
    assert((termStates->wasBuiltFor(ReaderUtil::getTopLevelContext(context)),
            L"The top-reader used to create Weight is not the same as the "
            L"current reader's top-reader (" +
                ReaderUtil::getTopLevelContext(context)));
    shared_ptr<TermState> *const state = termStates->get(context->ord);
    if (state == nullptr) { // term is not present in that reader
      assert((termNotInReader(context->reader(), outerInstance->term),
              L"no termstate found but term exists in reader term=" +
                  outerInstance->term));
      return nullptr;
    }
    shared_ptr<TermsEnum> *const termsEnum =
        context->reader()->terms(outerInstance->term->field())->begin();
    termsEnum->seekExact(outerInstance->term->bytes(), state);
    return termsEnum;
  } else {
    // TermQuery used as a filter, so the term states have not been built up
    // front
    shared_ptr<Terms> terms =
        context->reader()->terms(outerInstance->term->field());
    if (terms == nullptr) {
      return nullptr;
    }
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (termsEnum->seekExact(outerInstance->term->bytes())) {
      return termsEnum;
    } else {
      return nullptr;
    }
  }
}

bool TermQuery::TermWeight::termNotInReader(
    shared_ptr<LeafReader> reader, shared_ptr<Term> term) 
{
  // only called from assert
  // System.out.println("TQ.termNotInReader reader=" + reader + " term=" +
  // field + ":" + bytes.utf8ToString());
  return reader->docFreq(term) == 0;
}

shared_ptr<Explanation>
TermQuery::TermWeight::explain(shared_ptr<LeafReaderContext> context,
                               int doc) 
{
  shared_ptr<TermScorer> scorer =
      std::static_pointer_cast<TermScorer>(this->scorer(context));
  if (scorer != nullptr) {
    int newDoc = scorer->begin().advance(doc);
    if (newDoc == doc) {
      float freq = scorer->freq();
      shared_ptr<Similarity::SimScorer> docScorer =
          similarity->simScorer(stats, context);
      shared_ptr<Explanation> freqExplanation =
          Explanation::match(freq, L"termFreq=" + to_wstring(freq));
      shared_ptr<Explanation> scoreExplanation =
          docScorer->explain(doc, freqExplanation);
      return Explanation::match(
          scoreExplanation->getValue(),
          L"weight(" + getQuery() + L" in " + to_wstring(doc) + L") [" +
              similarity->getClass().getSimpleName() + L"], result of:",
          scoreExplanation);
    }
  }
  return Explanation::noMatch(L"no matching term");
}

TermQuery::TermQuery(shared_ptr<Term> t)
    : term(Objects::requireNonNull(t)),
      perReaderTermState(perReaderTermState.reset())
{
}

TermQuery::TermQuery(shared_ptr<Term> t, shared_ptr<TermContext> states)
    : term(Objects::requireNonNull(t)),
      perReaderTermState(Objects::requireNonNull(states))
{
  assert(states != nullptr);
}

shared_ptr<Term> TermQuery::getTerm() { return term; }

shared_ptr<Weight> TermQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                           bool needsScores,
                                           float boost) 
{
  shared_ptr<IndexReaderContext> *const context =
      searcher->getTopReaderContext();
  shared_ptr<TermContext> *const termState;
  if (perReaderTermState == nullptr ||
      perReaderTermState->wasBuiltFor(context) == false) {
    if (needsScores) {
      // make TermQuery single-pass if we don't have a PRTS or if the context
      // differs!
      termState = TermContext::build(context, term);
    } else {
      // do not compute the term state, this will help save seeks in the terms
      // dict on segments that have a cache entry for this query
      termState.reset();
    }
  } else {
    // PRTS was pre-build for this IS
    termState = this->perReaderTermState;
  }

  return make_shared<TermWeight>(shared_from_this(), searcher, needsScores,
                                 boost, termState);
}

wstring TermQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (term->field() != field) {
    buffer->append(term->field());
    buffer->append(L":");
  }
  buffer->append(term->text());
  return buffer->toString();
}

bool TermQuery::equals(any other)
{
  return sameClassAs(other) &&
         term->equals((any_cast<std::shared_ptr<TermQuery>>(other)).term);
}

int TermQuery::hashCode() { return classHash() ^ term->hashCode(); }
} // namespace org::apache::lucene::search