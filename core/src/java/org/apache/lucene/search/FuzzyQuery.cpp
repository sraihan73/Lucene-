using namespace std;

#include "FuzzyQuery.h"

namespace org::apache::lucene::search
{
using SingleTermsEnum = org::apache::lucene::index::SingleTermsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

FuzzyQuery::FuzzyQuery(shared_ptr<Term> term, int maxEdits, int prefixLength,
                       int maxExpansions, bool transpositions)
    : MultiTermQuery(term->field()), maxEdits(maxEdits),
      maxExpansions(maxExpansions), transpositions(transpositions),
      prefixLength(prefixLength), term(term)
{

  if (maxEdits < 0 ||
      maxEdits > LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) {
    throw invalid_argument(
        L"maxEdits must be between 0 and " +
        to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE));
  }
  if (prefixLength < 0) {
    throw invalid_argument(L"prefixLength cannot be negative.");
  }
  if (maxExpansions <= 0) {
    throw invalid_argument(L"maxExpansions must be positive.");
  }

  setRewriteMethod(
      make_shared<MultiTermQuery::TopTermsBlendedFreqScoringRewrite>(
          maxExpansions));
}

FuzzyQuery::FuzzyQuery(shared_ptr<Term> term, int maxEdits, int prefixLength)
    : FuzzyQuery(term, maxEdits, prefixLength, defaultMaxExpansions,
                 defaultTranspositions)
{
}

FuzzyQuery::FuzzyQuery(shared_ptr<Term> term, int maxEdits)
    : FuzzyQuery(term, maxEdits, defaultPrefixLength)
{
}

FuzzyQuery::FuzzyQuery(shared_ptr<Term> term)
    : FuzzyQuery(term, defaultMaxEdits)
{
}

int FuzzyQuery::getMaxEdits() { return maxEdits; }

int FuzzyQuery::getPrefixLength() { return prefixLength; }

bool FuzzyQuery::getTranspositions() { return transpositions; }

shared_ptr<TermsEnum>
FuzzyQuery::getTermsEnum(shared_ptr<Terms> terms,
                         shared_ptr<AttributeSource> atts) 
{
  if (maxEdits == 0 ||
      prefixLength >= term->text().length()) { // can only match if it's exact
    return make_shared<SingleTermsEnum>(terms->begin(), term->bytes());
  }
  return make_shared<FuzzyTermsEnum>(terms, atts, getTerm(), maxEdits,
                                     prefixLength, transpositions);
}

shared_ptr<Term> FuzzyQuery::getTerm() { return term; }

wstring FuzzyQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> *const buffer = make_shared<StringBuilder>();
  if (term->field() != field) {
    buffer->append(term->field());
    buffer->append(L":");
  }
  buffer->append(term->text());
  buffer->append(L'~');
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(Integer::toString(maxEdits));
  return buffer->toString();
}

int FuzzyQuery::hashCode()
{
  constexpr int prime = 31;
  int result = MultiTermQuery::hashCode();
  result = prime * result + maxEdits;
  result = prime * result + prefixLength;
  result = prime * result + maxExpansions;
  result = prime * result + (transpositions ? 0 : 1);
  result = prime * result + ((term == nullptr) ? 0 : term->hashCode());
  return result;
}

bool FuzzyQuery::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!MultiTermQuery::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<FuzzyQuery> other = any_cast<std::shared_ptr<FuzzyQuery>>(obj);
  if (maxEdits != other->maxEdits) {
    return false;
  }
  if (prefixLength != other->prefixLength) {
    return false;
  }
  if (maxExpansions != other->maxExpansions) {
    return false;
  }
  if (transpositions != other->transpositions) {
    return false;
  }
  if (term == nullptr) {
    if (other->term != nullptr) {
      return false;
    }
  } else if (!term->equals(other->term)) {
    return false;
  }
  return true;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static int floatToEdits(float
// minimumSimilarity, int termLen)
int FuzzyQuery::floatToEdits(float minimumSimilarity, int termLen)
{
  if (minimumSimilarity >= 1.0f) {
    return static_cast<int>(min(
        minimumSimilarity, LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE));
  } else if (minimumSimilarity == 0.0f) {
    return 0; // 0 means exact, not infinite # of edits!
  } else {
    return min(static_cast<int>((1 - minimumSimilarity) * termLen),
               LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE);
  }
}
} // namespace org::apache::lucene::search