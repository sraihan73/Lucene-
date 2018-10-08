using namespace std;

#include "SpanTermQuery.h"

namespace org::apache::lucene::search::spans
{
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SpanTermQuery::SpanTermQuery(shared_ptr<Term> term)
    : term(Objects::requireNonNull(term)),
      termContext(this->termContext.reset())
{
}

SpanTermQuery::SpanTermQuery(shared_ptr<Term> term,
                             shared_ptr<TermContext> context)
    : term(Objects::requireNonNull(term)), termContext(context)
{
}

shared_ptr<Term> SpanTermQuery::getTerm() { return term; }

wstring SpanTermQuery::getField() { return term->field(); }

shared_ptr<SpanWeight>
SpanTermQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                            bool needsScores, float boost) 
{
  shared_ptr<TermContext> *const context;
  shared_ptr<IndexReaderContext> *const topContext =
      searcher->getTopReaderContext();
  if (termContext == nullptr || termContext->wasBuiltFor(topContext) == false) {
    context = TermContext::build(topContext, term);
  } else {
    context = termContext;
  }
  return make_shared<SpanTermWeight>(
      shared_from_this(), context, searcher,
      needsScores ? Collections::singletonMap(term, context) : nullptr, boost);
}

SpanTermQuery::SpanTermWeight::SpanTermWeight(
    shared_ptr<SpanTermQuery> outerInstance,
    shared_ptr<TermContext> termContext, shared_ptr<IndexSearcher> searcher,
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>> &terms,
    float boost) 
    : SpanWeight(SpanTermQuery::this, searcher, terms, boost),
      termContext(termContext), outerInstance(outerInstance)
{
  assert((termContext != nullptr, L"TermContext must not be null"));
}

void SpanTermQuery::SpanTermWeight::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  terms->add(outerInstance->term);
}

bool SpanTermQuery::SpanTermWeight::isCacheable(
    shared_ptr<LeafReaderContext> ctx)
{
  return true;
}

void SpanTermQuery::SpanTermWeight::extractTermContexts(
    unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
        &contexts)
{
  contexts.emplace(outerInstance->term, termContext);
}

shared_ptr<Spans> SpanTermQuery::SpanTermWeight::getSpans(
    shared_ptr<LeafReaderContext> context,
    Postings requiredPostings) 
{

  assert((termContext->wasBuiltFor(ReaderUtil::getTopLevelContext(context)),
          L"The top-reader used to create Weight is not the same as the "
          L"current reader's top-reader (" +
              ReaderUtil::getTopLevelContext(context)));

  shared_ptr<TermState> *const state = termContext->get(context->ord);
  if (state == nullptr) { // term is not present in that reader
    assert((context->reader()->docFreq(outerInstance->term) == 0,
            L"no termstate found but term exists in reader term=" +
                outerInstance->term));
    return nullptr;
  }

  shared_ptr<Terms> *const terms =
      context->reader()->terms(outerInstance->term->field());
  if (terms == nullptr) {
    return nullptr;
  }
  if (terms->hasPositions() == false) {
    throw make_shared<IllegalStateException>(
        L"field \"" + outerInstance->term->field() +
        L"\" was indexed without position data; cannot run SpanTermQuery "
        L"(term=" +
        outerInstance->term->text() + L")");
  }

  shared_ptr<TermsEnum> *const termsEnum = terms->begin();
  termsEnum->seekExact(outerInstance->term->bytes(), state);

  shared_ptr<PostingsEnum> *const postings =
      termsEnum->postings(nullptr, requiredPostings.getRequiredPostings());
  float positionsCost =
      termPositionsCost(termsEnum) * PHRASE_TO_SPAN_TERM_POSITIONS_COST;
  return make_shared<TermSpans>(getSimScorer(context), postings,
                                outerInstance->term, positionsCost);
}

float SpanTermQuery::termPositionsCost(shared_ptr<TermsEnum> termsEnum) throw(
    IOException)
{
  int docFreq = termsEnum->docFreq();
  assert(docFreq > 0);
  int64_t totalTermFreq = termsEnum->totalTermFreq(); // -1 when not available
  float expOccurrencesInMatchingDoc =
      (totalTermFreq < docFreq) ? 1
                                : (totalTermFreq / static_cast<float>(docFreq));
  return TERM_POSNS_SEEK_OPS_PER_DOC +
         expOccurrencesInMatchingDoc * TERM_OPS_PER_POS;
}

wstring SpanTermQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (term->field() == field) {
    buffer->append(term->text());
  } else {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(term->toString());
  }
  return buffer->toString();
}

int SpanTermQuery::hashCode() { return classHash() ^ term->hashCode(); }

bool SpanTermQuery::equals(any other)
{
  return sameClassAs(other) &&
         term->equals((any_cast<std::shared_ptr<SpanTermQuery>>(other)).term);
}
} // namespace org::apache::lucene::search::spans