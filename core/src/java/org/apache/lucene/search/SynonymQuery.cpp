using namespace std;

#include "SynonymQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

SynonymQuery::SynonymQuery(deque<Term> &terms)
    : terms(Objects::requireNonNull(terms).clone())
{
  // check that all terms are the same field
  wstring field = L"";
  for (auto term : terms) {
    if (field == L"") {
      field = term->field();
    } else if (term->field() != field) {
      throw invalid_argument(L"Synonyms must be across the same field");
    }
  }
  if (terms.size() > BooleanQuery::getMaxClauseCount()) {
    throw make_shared<BooleanQuery::TooManyClauses>();
  }
  Arrays::sort(this->terms);
}

deque<std::shared_ptr<Term>> SynonymQuery::getTerms()
{
  return Collections::unmodifiableList(Arrays::asList(terms));
}

wstring SynonymQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>(L"Synonym(");
  for (int i = 0; i < terms.size(); i++) {
    if (i != 0) {
      builder->append(L" ");
    }
    shared_ptr<Query> termQuery = make_shared<TermQuery>(terms[i]);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->append(termQuery->toString(field));
  }
  builder->append(L")");
  return builder->toString();
}

int SynonymQuery::hashCode()
{
  return 31 * classHash() + Arrays::hashCode(terms);
}

bool SynonymQuery::equals(any other)
{
  return sameClassAs(other) &&
         Arrays::equals(terms,
                        (any_cast<std::shared_ptr<SynonymQuery>>(other)).terms);
}

shared_ptr<Query>
SynonymQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  // optimize zero and single term cases
  if (terms.empty()) {
    return (make_shared<BooleanQuery::Builder>())->build();
  }
  if (terms.size() == 1) {
    return make_shared<TermQuery>(terms[0]);
  }
  return shared_from_this();
}

shared_ptr<Weight>
SynonymQuery::createWeight(shared_ptr<IndexSearcher> searcher, bool needsScores,
                           float boost) 
{
  if (needsScores) {
    return make_shared<SynonymWeight>(shared_from_this(), shared_from_this(),
                                      searcher, boost);
  } else {
    // if scores are not needed, let BooleanWeight deal with optimizing that
    // case.
    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    for (auto term : terms) {
      bq->add(make_shared<TermQuery>(term), BooleanClause::Occur::SHOULD);
    }
    return searcher->rewrite(bq->build())
        ->createWeight(searcher, needsScores, boost);
  }
}

SynonymQuery::SynonymWeight::SynonymWeight(
    shared_ptr<SynonymQuery> outerInstance, shared_ptr<Query> query,
    shared_ptr<IndexSearcher> searcher, float boost) 
    : Weight(query), termContexts(std::deque<std::shared_ptr<TermContext>>(
                         outerInstance->terms.size())),
      similarity(searcher->getSimilarity(true)),
      simWeight(
          similarity->computeWeight(boost, collectionStats, {pseudoStats})),
      outerInstance(outerInstance)
{
  shared_ptr<CollectionStatistics> collectionStats =
      searcher->collectionStatistics(outerInstance->terms[0]->field());
  int64_t docFreq = 0;
  int64_t totalTermFreq = 0;
  for (int i = 0; i < termContexts.size(); i++) {
    termContexts[i] = TermContext::build(searcher->getTopReaderContext(),
                                         outerInstance->terms[i]);
    shared_ptr<TermStatistics> termStats =
        searcher->termStatistics(outerInstance->terms[i], termContexts[i]);
    docFreq = max(termStats->docFreq(), docFreq);
    if (termStats->totalTermFreq() == -1) {
      totalTermFreq = -1;
    } else if (totalTermFreq != -1) {
      totalTermFreq += termStats->totalTermFreq();
    }
  }
  shared_ptr<TermStatistics> pseudoStats =
      make_shared<TermStatistics>(nullptr, docFreq, totalTermFreq);
}

void SynonymQuery::SynonymWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto term : outerInstance->terms) {
    terms->add(term);
  }
}

shared_ptr<Matches>
SynonymQuery::SynonymWeight::matches(shared_ptr<LeafReaderContext> context,
                                     int doc) 
{
  wstring field = outerInstance->terms[0]->field();
  shared_ptr<Terms> terms = context->reader()->terms(field);
  if (terms == nullptr || terms->hasPositions() == false) {
    return Weight::matches(context, doc);
  }
  return Matches::forField(field, [&]() {
    DisjunctionMatchesIterator::fromTerms(
        context, doc, field, java::util::Arrays::asList(outerInstance->terms));
  });
}

shared_ptr<Explanation>
SynonymQuery::SynonymWeight::explain(shared_ptr<LeafReaderContext> context,
                                     int doc) 
{
  shared_ptr<Scorer> scorer = this->scorer(context);
  if (scorer != nullptr) {
    int newDoc = scorer->begin().advance(doc);
    if (newDoc == doc) {
      constexpr float freq;
      if (std::dynamic_pointer_cast<SynonymScorer>(scorer) != nullptr) {
        shared_ptr<SynonymScorer> synScorer =
            std::static_pointer_cast<SynonymScorer>(scorer);
        freq = synScorer->tf(synScorer->getSubMatches());
      } else {
        assert(std::dynamic_pointer_cast<TermScorer>(scorer) != nullptr);
        freq = (std::static_pointer_cast<TermScorer>(scorer))->freq();
      }
      shared_ptr<Similarity::SimScorer> docScorer =
          similarity->simScorer(simWeight, context);
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

shared_ptr<Scorer> SynonymQuery::SynonymWeight::scorer(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<Similarity::SimScorer> simScorer =
      similarity->simScorer(simWeight, context);
  // we use termscorers + disjunction as an impl detail
  deque<std::shared_ptr<Scorer>> subScorers =
      deque<std::shared_ptr<Scorer>>();
  for (int i = 0; i < outerInstance->terms.size(); i++) {
    shared_ptr<TermState> state = termContexts[i]->get(context->ord);
    if (state != nullptr) {
      shared_ptr<TermsEnum> termsEnum =
          context->reader()->terms(outerInstance->terms[i]->field())->begin();
      termsEnum->seekExact(outerInstance->terms[i]->bytes(), state);
      shared_ptr<PostingsEnum> postings =
          termsEnum->postings(nullptr, PostingsEnum::FREQS);
      subScorers.push_back(
          make_shared<TermScorer>(shared_from_this(), postings, simScorer));
    }
  }
  if (subScorers.empty()) {
    return nullptr;
  } else if (subScorers.size() == 1) {
    // we must optimize this case (term not in segment), disjunctionscorer
    // requires >= 2 subs
    return subScorers[0];
  } else {
    return make_shared<SynonymScorer>(simScorer, shared_from_this(),
                                      subScorers);
  }
}

bool SynonymQuery::SynonymWeight::isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

SynonymQuery::SynonymScorer::SynonymScorer(
    shared_ptr<Similarity::SimScorer> similarity, shared_ptr<Weight> weight,
    deque<std::shared_ptr<Scorer>> &subScorers)
    : DisjunctionScorer(weight, subScorers, true), similarity(similarity)
{
}

float SynonymQuery::SynonymScorer::score(shared_ptr<DisiWrapper> topList) throw(
    IOException)
{
  return similarity->score(topList->doc, tf(topList));
}

int SynonymQuery::SynonymScorer::tf(shared_ptr<DisiWrapper> topList) throw(
    IOException)
{
  int tf = 0;
  for (shared_ptr<DisiWrapper> w = topList; w != nullptr; w = w->next) {
    tf += (std::static_pointer_cast<TermScorer>(w->scorer))->freq();
  }
  return tf;
}
} // namespace org::apache::lucene::search