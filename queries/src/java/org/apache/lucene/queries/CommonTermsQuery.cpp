using namespace std;

#include "CommonTermsQuery.h"

namespace org::apache::lucene::queries
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;

CommonTermsQuery::CommonTermsQuery(Occur highFreqOccur, Occur lowFreqOccur,
                                   float maxTermFrequency)
    : maxTermFrequency(maxTermFrequency), lowFreqOccur(lowFreqOccur),
      highFreqOccur(highFreqOccur)
{
  if (highFreqOccur == Occur::MUST_NOT) {
    throw invalid_argument(
        L"highFreqOccur should be MUST or SHOULD but was MUST_NOT");
  }
  if (lowFreqOccur == Occur::MUST_NOT) {
    throw invalid_argument(
        L"lowFreqOccur should be MUST or SHOULD but was MUST_NOT");
  }
}

void CommonTermsQuery::add(shared_ptr<Term> term)
{
  if (term == nullptr) {
    throw invalid_argument(L"Term must not be null");
  }
  this->terms.push_back(term);
}

shared_ptr<Query>
CommonTermsQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (this->terms.empty()) {
    return make_shared<MatchNoDocsQuery>(L"CommonTermsQuery with no terms");
  } else if (this->terms.size() == 1) {
    return newTermQuery(this->terms[0], nullptr);
  }
  const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  constexpr int maxDoc = reader->maxDoc();
  std::deque<std::shared_ptr<TermContext>> contextArray(terms.size());
  std::deque<std::shared_ptr<Term>> queryTerms =
      this->terms.toArray(std::deque<std::shared_ptr<Term>>(0));
  collectTermContext(reader, leaves, contextArray, queryTerms);
  return buildQuery(maxDoc, contextArray, queryTerms);
}

int CommonTermsQuery::calcLowFreqMinimumNumberShouldMatch(int numOptional)
{
  return minNrShouldMatch(lowFreqMinNrShouldMatch, numOptional);
}

int CommonTermsQuery::calcHighFreqMinimumNumberShouldMatch(int numOptional)
{
  return minNrShouldMatch(highFreqMinNrShouldMatch, numOptional);
}

int CommonTermsQuery::minNrShouldMatch(float minNrShouldMatch, int numOptional)
{
  if (minNrShouldMatch >= 1.0f || minNrShouldMatch == 0.0f) {
    return static_cast<int>(minNrShouldMatch);
  }
  return round(minNrShouldMatch * numOptional);
}

shared_ptr<Query> CommonTermsQuery::buildQuery(
    int const maxDoc, std::deque<std::shared_ptr<TermContext>> &contextArray,
    std::deque<std::shared_ptr<Term>> &queryTerms)
{
  deque<std::shared_ptr<Query>> lowFreqQueries =
      deque<std::shared_ptr<Query>>();
  deque<std::shared_ptr<Query>> highFreqQueries =
      deque<std::shared_ptr<Query>>();
  for (int i = 0; i < queryTerms.size(); i++) {
    shared_ptr<TermContext> termContext = contextArray[i];
    if (termContext == nullptr) {
      lowFreqQueries.push_back(newTermQuery(queryTerms[i], nullptr));
    } else {
      if ((maxTermFrequency >= 1.0f &&
           termContext->docFreq() > maxTermFrequency) ||
          (termContext->docFreq() >
           static_cast<int>(
               ceil(maxTermFrequency * static_cast<float>(maxDoc))))) {
        highFreqQueries.push_back(newTermQuery(queryTerms[i], termContext));
      } else {
        lowFreqQueries.push_back(newTermQuery(queryTerms[i], termContext));
      }
    }
  }
  constexpr int numLowFreqClauses = lowFreqQueries.size();
  constexpr int numHighFreqClauses = highFreqQueries.size();
  Occur lowFreqOccur = this->lowFreqOccur;
  Occur highFreqOccur = this->highFreqOccur;
  int lowFreqMinShouldMatch = 0;
  int highFreqMinShouldMatch = 0;
  if (lowFreqOccur == Occur::SHOULD && numLowFreqClauses > 0) {
    lowFreqMinShouldMatch =
        calcLowFreqMinimumNumberShouldMatch(numLowFreqClauses);
  }
  if (highFreqOccur == Occur::SHOULD && numHighFreqClauses > 0) {
    highFreqMinShouldMatch =
        calcHighFreqMinimumNumberShouldMatch(numHighFreqClauses);
  }
  if (lowFreqQueries.empty()) {
    /*
     * if lowFreq is empty we rewrite the high freq terms in a conjunction to
     * prevent slow queries.
     */
    if (highFreqMinShouldMatch == 0 && highFreqOccur != Occur::MUST) {
      highFreqOccur = Occur::MUST;
    }
  }
  shared_ptr<BooleanQuery::Builder> builder =
      make_shared<BooleanQuery::Builder>();

  if (lowFreqQueries.empty() == false) {
    shared_ptr<BooleanQuery::Builder> lowFreq =
        make_shared<BooleanQuery::Builder>();
    for (auto query : lowFreqQueries) {
      lowFreq->add(query, lowFreqOccur);
    }
    lowFreq->setMinimumNumberShouldMatch(lowFreqMinShouldMatch);
    shared_ptr<Query> lowFreqQuery = lowFreq->build();
    builder->add(make_shared<BoostQuery>(lowFreqQuery, lowFreqBoost),
                 Occur::MUST);
  }
  if (highFreqQueries.empty() == false) {
    shared_ptr<BooleanQuery::Builder> highFreq =
        make_shared<BooleanQuery::Builder>();
    for (auto query : highFreqQueries) {
      highFreq->add(query, highFreqOccur);
    }
    highFreq->setMinimumNumberShouldMatch(highFreqMinShouldMatch);
    shared_ptr<Query> highFreqQuery = highFreq->build();
    builder->add(make_shared<BoostQuery>(highFreqQuery, highFreqBoost),
                 Occur::SHOULD);
  }
  return builder->build();
}

void CommonTermsQuery::collectTermContext(
    shared_ptr<IndexReader> reader,
    deque<std::shared_ptr<LeafReaderContext>> &leaves,
    std::deque<std::shared_ptr<TermContext>> &contextArray,
    std::deque<std::shared_ptr<Term>> &queryTerms) 
{
  shared_ptr<TermsEnum> termsEnum = nullptr;
  for (auto context : leaves) {
    for (int i = 0; i < queryTerms.size(); i++) {
      shared_ptr<Term> term = queryTerms[i];
      shared_ptr<TermContext> termContext = contextArray[i];
      shared_ptr<Terms> *const terms = context->reader()->terms(term->field());
      if (terms == nullptr) {
        // field does not exist
        continue;
      }
      termsEnum = terms->begin();
      assert(termsEnum != nullptr);

      if (termsEnum == TermsEnum::EMPTY) {
        continue;
      }
      if (termsEnum->seekExact(term->bytes())) {
        if (termContext == nullptr) {
          contextArray[i] = make_shared<TermContext>(
              reader->getContext(), termsEnum->termState(), context->ord,
              termsEnum->docFreq(), termsEnum->totalTermFreq());
        } else {
          termContext->register_(termsEnum->termState(), context->ord,
                                 termsEnum->docFreq(),
                                 termsEnum->totalTermFreq());
        }
      }
    }
  }
}

void CommonTermsQuery::setLowFreqMinimumNumberShouldMatch(float min)
{
  this->lowFreqMinNrShouldMatch = min;
}

float CommonTermsQuery::getLowFreqMinimumNumberShouldMatch()
{
  return lowFreqMinNrShouldMatch;
}

void CommonTermsQuery::setHighFreqMinimumNumberShouldMatch(float min)
{
  this->highFreqMinNrShouldMatch = min;
}

float CommonTermsQuery::getHighFreqMinimumNumberShouldMatch()
{
  return highFreqMinNrShouldMatch;
}

deque<std::shared_ptr<Term>> CommonTermsQuery::getTerms()
{
  return Collections::unmodifiableList(terms);
}

float CommonTermsQuery::getMaxTermFrequency() { return maxTermFrequency; }

Occur CommonTermsQuery::getLowFreqOccur() { return lowFreqOccur; }

Occur CommonTermsQuery::getHighFreqOccur() { return highFreqOccur; }

float CommonTermsQuery::getLowFreqBoost() { return lowFreqBoost; }

float CommonTermsQuery::getHighFreqBoost() { return highFreqBoost; }

wstring CommonTermsQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  bool needParens = (getLowFreqMinimumNumberShouldMatch() > 0);
  if (needParens) {
    buffer->append(L"(");
  }
  for (int i = 0; i < terms.size(); i++) {
    shared_ptr<Term> t = terms[i];
    // C++ TODO: There is no native C++ equivalent to 'toString':
    buffer->append(newTermQuery(t, nullptr)->toString());

    if (i != terms.size() - 1) {
      buffer->append(L", ");
    }
  }
  if (needParens) {
    buffer->append(L")");
  }
  if (getLowFreqMinimumNumberShouldMatch() > 0 ||
      getHighFreqMinimumNumberShouldMatch() > 0) {
    buffer->append(L'~');
    buffer->append(L"(");
    buffer->append(getLowFreqMinimumNumberShouldMatch());
    buffer->append(getHighFreqMinimumNumberShouldMatch());
    buffer->append(L")");
  }
  return buffer->toString();
}

int CommonTermsQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result = prime * result + Float::floatToIntBits(highFreqBoost);
  result = prime * result + Objects::hashCode(highFreqOccur);
  result = prime * result + Objects::hashCode(lowFreqOccur);
  result = prime * result + Float::floatToIntBits(lowFreqBoost);
  result = prime * result + Float::floatToIntBits(maxTermFrequency);
  result = prime * result + Float::floatToIntBits(lowFreqMinNrShouldMatch);
  result = prime * result + Float::floatToIntBits(highFreqMinNrShouldMatch);
  result = prime * result + Objects::hashCode(terms);
  return result;
}

bool CommonTermsQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool CommonTermsQuery::equalsTo(shared_ptr<CommonTermsQuery> other)
{
  return Float::floatToIntBits(highFreqBoost) ==
             Float::floatToIntBits(other->highFreqBoost) &&
         highFreqOccur == other->highFreqOccur &&
         lowFreqOccur == other->lowFreqOccur &&
         Float::floatToIntBits(lowFreqBoost) ==
             Float::floatToIntBits(other->lowFreqBoost) &&
         Float::floatToIntBits(maxTermFrequency) ==
             Float::floatToIntBits(other->maxTermFrequency) &&
         lowFreqMinNrShouldMatch == other->lowFreqMinNrShouldMatch &&
         highFreqMinNrShouldMatch == other->highFreqMinNrShouldMatch &&
         terms.equals(other->terms);
}

shared_ptr<Query>
CommonTermsQuery::newTermQuery(shared_ptr<Term> term,
                               shared_ptr<TermContext> context)
{
  return context == nullptr ? make_shared<TermQuery>(term)
                            : make_shared<TermQuery>(term, context);
}
} // namespace org::apache::lucene::queries