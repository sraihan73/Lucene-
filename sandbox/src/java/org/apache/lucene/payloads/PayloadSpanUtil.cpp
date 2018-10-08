using namespace std;

#include "PayloadSpanUtil.h"

namespace org::apache::lucene::payloads
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DisjunctionMaxQuery = org::apache::lucene::search::DisjunctionMaxQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MultiPhraseQuery = org::apache::lucene::search::MultiPhraseQuery;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using SpanNearQuery = org::apache::lucene::search::spans::SpanNearQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using Spans = org::apache::lucene::search::spans::Spans;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using SpanWeight = org::apache::lucene::search::spans::SpanWeight;

PayloadSpanUtil::PayloadSpanUtil(shared_ptr<IndexReaderContext> context)
{
  this->context = context;
}

shared_ptr<deque<std::deque<char>>>
PayloadSpanUtil::getPayloadsForQuery(shared_ptr<Query> query) 
{
  shared_ptr<deque<std::deque<char>>> payloads =
      deque<std::deque<char>>();
  queryToSpanQuery(query, payloads);
  return payloads;
}

void PayloadSpanUtil::queryToSpanQuery(
    shared_ptr<Query> query,
    shared_ptr<deque<std::deque<char>>> payloads) 
{
  if (std::dynamic_pointer_cast<BooleanQuery>(query) != nullptr) {
    for (auto clause : std::static_pointer_cast<BooleanQuery>(query)) {
      if (!clause->isProhibited()) {
        queryToSpanQuery(clause->getQuery(), payloads);
      }
    }

  } else if (std::dynamic_pointer_cast<PhraseQuery>(query) != nullptr) {
    std::deque<std::shared_ptr<Term>> phraseQueryTerms =
        (std::static_pointer_cast<PhraseQuery>(query))->getTerms();
    std::deque<std::shared_ptr<SpanQuery>> clauses(phraseQueryTerms.size());
    for (int i = 0; i < phraseQueryTerms.size(); i++) {
      clauses[i] = make_shared<SpanTermQuery>(phraseQueryTerms[i]);
    }

    int slop = (std::static_pointer_cast<PhraseQuery>(query))->getSlop();
    bool inorder = false;

    if (slop == 0) {
      inorder = true;
    }

    shared_ptr<SpanNearQuery> sp =
        make_shared<SpanNearQuery>(clauses, slop, inorder);
    getPayloads(payloads, sp);
  } else if (std::dynamic_pointer_cast<TermQuery>(query) != nullptr) {
    shared_ptr<SpanTermQuery> stq = make_shared<SpanTermQuery>(
        (std::static_pointer_cast<TermQuery>(query))->getTerm());
    getPayloads(payloads, stq);
  } else if (std::dynamic_pointer_cast<SpanQuery>(query) != nullptr) {
    getPayloads(payloads, std::static_pointer_cast<SpanQuery>(query));
  } else if (std::dynamic_pointer_cast<DisjunctionMaxQuery>(query) != nullptr) {

    for (shared_ptr<Iterator<std::shared_ptr<Query>>> iterator =
             (std::static_pointer_cast<DisjunctionMaxQuery>(query))->begin();
         iterator->hasNext();) {
      queryToSpanQuery(iterator->next(), payloads);
    }

  } else if (std::dynamic_pointer_cast<MultiPhraseQuery>(query) != nullptr) {
    shared_ptr<MultiPhraseQuery> *const mpq =
        std::static_pointer_cast<MultiPhraseQuery>(query);
    std::deque<std::deque<std::shared_ptr<Term>>> termArrays =
        mpq->getTermArrays();
    const std::deque<int> positions = mpq->getPositions();
    if (positions.size() > 0) {

      int maxPosition = positions[positions.size() - 1];
      for (int i = 0; i < positions.size() - 1; ++i) {
        if (positions[i] > maxPosition) {
          maxPosition = positions[i];
        }
      }

      // C++ TODO: Most Java annotations will not have direct C++ equivalents:
      // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
      // java.util.List<org.apache.lucene.search.Query>[] disjunctLists = new
      // java.util.List[maxPosition + 1];
      const std::deque<deque<std::shared_ptr<Query>>> disjunctLists =
          std::deque<deque>(maxPosition + 1);
      int distinctPositions = 0;

      for (int i = 0; i < termArrays.size(); ++i) {
        std::deque<std::shared_ptr<Term>> termArray = termArrays[i];
        deque<std::shared_ptr<Query>> disjuncts = disjunctLists[positions[i]];
        if (disjuncts.empty()) {
          disjuncts =
              (disjunctLists[positions[i]] = deque<>(termArray.size()));
          ++distinctPositions;
        }
        for (auto term : termArray) {
          disjuncts.push_back(make_shared<SpanTermQuery>(term));
        }
      }

      int positionGaps = 0;
      int position = 0;
      std::deque<std::shared_ptr<SpanQuery>> clauses(distinctPositions);
      for (int i = 0; i < disjunctLists.size(); ++i) {
        deque<std::shared_ptr<Query>> disjuncts = disjunctLists[i];
        if (disjuncts.size() > 0) {
          clauses[position++] = make_shared<SpanOrQuery>(disjuncts.toArray(
              std::deque<std::shared_ptr<SpanQuery>>(disjuncts.size())));
        } else {
          ++positionGaps;
        }
      }

      constexpr int slop = mpq->getSlop();
      constexpr bool inorder = (slop == 0);

      shared_ptr<SpanNearQuery> sp =
          make_shared<SpanNearQuery>(clauses, slop + positionGaps, inorder);
      getPayloads(payloads, sp);
    }
  }
}

void PayloadSpanUtil::getPayloads(
    shared_ptr<deque<std::deque<char>>> payloads,
    shared_ptr<SpanQuery> query) 
{

  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(context);
  searcher->setQueryCache(nullptr);

  query = std::static_pointer_cast<SpanQuery>(searcher->rewrite(query));
  shared_ptr<SpanWeight> w = std::static_pointer_cast<SpanWeight>(
      searcher->createWeight(query, false, 1));

  shared_ptr<PayloadSpanCollector> collector =
      make_shared<PayloadSpanCollector>();
  for (auto leafReaderContext : context->leaves()) {
    shared_ptr<Spans> *const spans =
        w->getSpans(leafReaderContext, SpanWeight::Postings::PAYLOADS);
    if (spans != nullptr) {
      while (spans->nextDoc() != Spans::NO_MORE_DOCS) {
        while (spans->nextStartPosition() != Spans::NO_MORE_POSITIONS) {
          collector->reset();
          spans->collect(collector);
          payloads->addAll(collector->getPayloads());
        }
      }
    }
  }
}
} // namespace org::apache::lucene::payloads