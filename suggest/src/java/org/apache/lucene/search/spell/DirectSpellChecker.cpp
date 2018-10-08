using namespace std;

#include "DirectSpellChecker.h"

namespace org::apache::lucene::search::spell
{
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using BoostAttribute = org::apache::lucene::search::BoostAttribute;
using FuzzyTermsEnum = org::apache::lucene::search::FuzzyTermsEnum;
using MaxNonCompetitiveBoostAttribute =
    org::apache::lucene::search::MaxNonCompetitiveBoostAttribute;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;
const shared_ptr<StringDistance> DirectSpellChecker::INTERNAL_LEVENSHTEIN =
    make_shared<LuceneLevenshteinDistance>();

DirectSpellChecker::DirectSpellChecker() {}

int DirectSpellChecker::getMaxEdits() { return maxEdits; }

void DirectSpellChecker::setMaxEdits(int maxEdits)
{
  if (maxEdits < 1 ||
      maxEdits > LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) {
    throw make_shared<UnsupportedOperationException>(L"Invalid maxEdits");
  }
  this->maxEdits = maxEdits;
}

int DirectSpellChecker::getMinPrefix() { return minPrefix; }

void DirectSpellChecker::setMinPrefix(int minPrefix)
{
  this->minPrefix = minPrefix;
}

int DirectSpellChecker::getMaxInspections() { return maxInspections; }

void DirectSpellChecker::setMaxInspections(int maxInspections)
{
  this->maxInspections = maxInspections;
}

float DirectSpellChecker::getAccuracy() { return accuracy; }

void DirectSpellChecker::setAccuracy(float accuracy)
{
  this->accuracy = accuracy;
}

float DirectSpellChecker::getThresholdFrequency() { return thresholdFrequency; }

void DirectSpellChecker::setThresholdFrequency(float thresholdFrequency)
{
  if (thresholdFrequency >= 1.0f &&
      thresholdFrequency != static_cast<int>(thresholdFrequency)) {
    throw invalid_argument(
        L"Fractional absolute document frequencies are not allowed");
  }
  this->thresholdFrequency = thresholdFrequency;
}

int DirectSpellChecker::getMinQueryLength() { return minQueryLength; }

void DirectSpellChecker::setMinQueryLength(int minQueryLength)
{
  this->minQueryLength = minQueryLength;
}

float DirectSpellChecker::getMaxQueryFrequency() { return maxQueryFrequency; }

void DirectSpellChecker::setMaxQueryFrequency(float maxQueryFrequency)
{
  if (maxQueryFrequency >= 1.0f &&
      maxQueryFrequency != static_cast<int>(maxQueryFrequency)) {
    throw invalid_argument(
        L"Fractional absolute document frequencies are not allowed");
  }
  this->maxQueryFrequency = maxQueryFrequency;
}

bool DirectSpellChecker::getLowerCaseTerms() { return lowerCaseTerms; }

void DirectSpellChecker::setLowerCaseTerms(bool lowerCaseTerms)
{
  this->lowerCaseTerms = lowerCaseTerms;
}

shared_ptr<Comparator<std::shared_ptr<SuggestWord>>>
DirectSpellChecker::getComparator()
{
  return comparator;
}

void DirectSpellChecker::setComparator(
    shared_ptr<Comparator<std::shared_ptr<SuggestWord>>> comparator)
{
  this->comparator = comparator;
}

shared_ptr<StringDistance> DirectSpellChecker::getDistance()
{
  return distance;
}

void DirectSpellChecker::setDistance(shared_ptr<StringDistance> distance)
{
  this->distance = distance;
}

std::deque<std::shared_ptr<SuggestWord>> DirectSpellChecker::suggestSimilar(
    shared_ptr<Term> term, int numSug,
    shared_ptr<IndexReader> ir) 
{
  return suggestSimilar(term, numSug, ir,
                        SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX);
}

std::deque<std::shared_ptr<SuggestWord>>
DirectSpellChecker::suggestSimilar(shared_ptr<Term> term, int numSug,
                                   shared_ptr<IndexReader> ir,
                                   SuggestMode suggestMode) 
{
  return suggestSimilar(term, numSug, ir, suggestMode, this->accuracy);
}

std::deque<std::shared_ptr<SuggestWord>> DirectSpellChecker::suggestSimilar(
    shared_ptr<Term> term, int numSug, shared_ptr<IndexReader> ir,
    SuggestMode suggestMode, float accuracy) 
{
  shared_ptr<CharsRefBuilder> *const spare = make_shared<CharsRefBuilder>();
  wstring text = term->text();
  if (minQueryLength > 0 &&
      text.codePointCount(0, text.length()) < minQueryLength) {
    return std::deque<std::shared_ptr<SuggestWord>>(0);
  }

  if (lowerCaseTerms) {
    term = make_shared<Term>(term->field(), text.toLowerCase(Locale::ROOT));
  }

  int docfreq = ir->docFreq(term);

  if (suggestMode == SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX && docfreq > 0) {
    return std::deque<std::shared_ptr<SuggestWord>>(0);
  }

  int maxDoc = ir->maxDoc();

  if (maxQueryFrequency >= 1.0f && docfreq > maxQueryFrequency) {
    return std::deque<std::shared_ptr<SuggestWord>>(0);
  } else if (docfreq > static_cast<int>(ceil(maxQueryFrequency *
                                             static_cast<float>(maxDoc)))) {
    return std::deque<std::shared_ptr<SuggestWord>>(0);
  }

  if (suggestMode != SuggestMode::SUGGEST_MORE_POPULAR) {
    docfreq = 0;
  }

  if (thresholdFrequency >= 1.0f) {
    docfreq = max(docfreq, static_cast<int>(thresholdFrequency));
  } else if (thresholdFrequency > 0.0f) {
    docfreq = max(
        docfreq,
        static_cast<int>(thresholdFrequency * static_cast<float>(maxDoc)) - 1);
  }

  shared_ptr<deque<std::shared_ptr<ScoreTerm>>> terms = nullptr;
  int inspections = numSug * maxInspections;

  // try ed=1 first, in case we get lucky
  terms = suggestSimilar(term, inspections, ir, docfreq, 1, accuracy, spare);
  if (maxEdits > 1 && terms->size() < inspections) {
    unordered_set<std::shared_ptr<ScoreTerm>> moreTerms =
        unordered_set<std::shared_ptr<ScoreTerm>>();
    moreTerms.addAll(terms);
    moreTerms.addAll(suggestSimilar(term, inspections, ir, docfreq, maxEdits,
                                    accuracy, spare));
    terms = moreTerms;
  }

  // create the suggestword response, sort it, and trim it to size.

  std::deque<std::shared_ptr<SuggestWord>> suggestions(terms->size());
  int index = suggestions.size() - 1;
  for (auto s : terms) {
    shared_ptr<SuggestWord> suggestion = make_shared<SuggestWord>();
    if (s->termAsString == L"") {
      spare->copyUTF8Bytes(s->term);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      s->termAsString = spare->toString();
    }
    suggestion->string = s->termAsString;
    suggestion->score = s->score;
    suggestion->freq = s->docfreq;
    suggestions[index--] = suggestion;
  }

  ArrayUtil::timSort(suggestions, Collections::reverseOrder(comparator));
  if (numSug < suggestions.size()) {
    std::deque<std::shared_ptr<SuggestWord>> trimmed(numSug);
    System::arraycopy(suggestions, 0, trimmed, 0, numSug);
    suggestions = trimmed;
  }
  return suggestions;
}

shared_ptr<deque<std::shared_ptr<ScoreTerm>>>
DirectSpellChecker::suggestSimilar(
    shared_ptr<Term> term, int numSug, shared_ptr<IndexReader> ir, int docfreq,
    int editDistance, float accuracy,
    shared_ptr<CharsRefBuilder> spare) 
{

  shared_ptr<AttributeSource> atts = make_shared<AttributeSource>();
  shared_ptr<MaxNonCompetitiveBoostAttribute> maxBoostAtt =
      atts->addAttribute(MaxNonCompetitiveBoostAttribute::typeid);
  shared_ptr<Terms> terms = MultiFields::getTerms(ir, term->field());
  if (terms == nullptr) {
    return Collections::emptyList();
  }
  shared_ptr<FuzzyTermsEnum> e = make_shared<FuzzyTermsEnum>(
      terms, atts, term, editDistance, max(minPrefix, editDistance - 1), true);
  shared_ptr<PriorityQueue<std::shared_ptr<ScoreTerm>>> *const stQueue =
      make_shared<PriorityQueue<std::shared_ptr<ScoreTerm>>>();

  shared_ptr<BytesRef> queryTerm = make_shared<BytesRef>(term->text());
  shared_ptr<BytesRef> candidateTerm;
  shared_ptr<ScoreTerm> st = make_shared<ScoreTerm>();
  shared_ptr<BoostAttribute> boostAtt =
      e->attributes()->addAttribute(BoostAttribute::typeid);
  while ((candidateTerm = e->next()) != nullptr) {
    // For FuzzyQuery, boost is the score:
    float score = boostAtt->getBoost();
    // ignore uncompetitive hits
    if (stQueue->size() >= numSug && score <= stQueue->peek().boost) {
      continue;
    }

    // ignore exact match of the same term
    if (queryTerm->bytesEquals(candidateTerm)) {
      continue;
    }

    int df = e->docFreq();

    // check docFreq if required
    if (df <= docfreq) {
      continue;
    }

    const wstring termAsString;
    if (distance == INTERNAL_LEVENSHTEIN) {
      // delay creating strings until the end
      termAsString = L"";
    } else {
      spare->copyUTF8Bytes(candidateTerm);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      termAsString = spare->toString();
      score = distance->getDistance(term->text(), termAsString);
    }

    if (score < accuracy) {
      continue;
    }

    // add new entry in PQ
    st->term = BytesRef::deepCopyOf(candidateTerm);
    st->boost = score;
    st->docfreq = df;
    st->termAsString = termAsString;
    st->score = score;
    stQueue->offer(st);
    // possibly drop entries from queue
    st =
        (stQueue->size() > numSug) ? stQueue->poll() : make_shared<ScoreTerm>();
    maxBoostAtt->setMaxNonCompetitiveBoost(
        (stQueue->size() >= numSug) ? stQueue->peek().boost
                                    : -numeric_limits<float>::infinity());
  }

  return stQueue;
}

DirectSpellChecker::ScoreTerm::ScoreTerm() {}

int DirectSpellChecker::ScoreTerm::compareTo(shared_ptr<ScoreTerm> other)
{
  if (term->bytesEquals(other->term)) {
    return 0; // consistent with equals
  }
  if (this->boost == other->boost) {
    return other->term->compareTo(this->term);
  } else {
    return Float::compare(this->boost, other->boost);
  }
}

int DirectSpellChecker::ScoreTerm::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + ((term == nullptr) ? 0 : term->hashCode());
  return result;
}

bool DirectSpellChecker::ScoreTerm::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<ScoreTerm> other = any_cast<std::shared_ptr<ScoreTerm>>(obj);
  if (term == nullptr) {
    if (other->term != nullptr) {
      return false;
    }
  } else if (!term->bytesEquals(other->term)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search::spell