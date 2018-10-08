using namespace std;

#include "PhraseQuery.h"

namespace org::apache::lucene::search
{
using Lucene50PostingsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;

PhraseQuery::Builder::Builder() : terms(deque<>()), positions(deque<>())
{
  slop = 0;
}

shared_ptr<Builder> PhraseQuery::Builder::setSlop(int slop)
{
  this->slop = slop;
  return shared_from_this();
}

shared_ptr<Builder> PhraseQuery::Builder::add(shared_ptr<Term> term)
{
  return add(term, positions.empty() ? 0 : 1 + positions[positions.size() - 1]);
}

shared_ptr<Builder> PhraseQuery::Builder::add(shared_ptr<Term> term,
                                              int position)
{
  if (position < 0) {
    throw invalid_argument(L"Positions must be >= 0, got " +
                           to_wstring(position));
  }
  if (positions.empty() == false) {
    constexpr int lastPosition = positions[positions.size() - 1];
    if (position < lastPosition) {
      throw invalid_argument(L"Positions must be added in order, got " +
                             to_wstring(position) + L" after " +
                             to_wstring(lastPosition));
    }
  }
  if (terms.empty() == false && term->field() == terms[0]->field() == false) {
    throw invalid_argument(L"All terms must be on the same field, got " +
                           term->field() + L" and " + terms[0]->field());
  }
  terms.push_back(term);
  positions.push_back(position);
  return shared_from_this();
}

shared_ptr<PhraseQuery> PhraseQuery::Builder::build()
{
  std::deque<std::shared_ptr<Term>> terms = this->terms.toArray(
      std::deque<std::shared_ptr<Term>>(this->terms.size()));
  std::deque<int> positions(this->positions.size());
  for (int i = 0; i < positions.size(); ++i) {
    positions[i] = this->positions[i];
  }
  return make_shared<PhraseQuery>(slop, terms, positions);
}

PhraseQuery::PhraseQuery(int slop, std::deque<std::shared_ptr<Term>> &terms,
                         std::deque<int> &positions)
    : slop(slop), field(terms.empty() ? L"" : terms[0]->field()), terms(terms),
      positions(positions)
{
  if (terms.size() != positions.size()) {
    throw invalid_argument(L"Must have as many terms as positions");
  }
  if (slop < 0) {
    throw invalid_argument(L"Slop must be >= 0, got " + to_wstring(slop));
  }
  for (int i = 1; i < terms.size(); ++i) {
    if (terms[i - 1]->field() == terms[i]->field() == false) {
      throw invalid_argument(L"All terms should have the same field");
    }
  }
  for (auto position : positions) {
    if (position < 0) {
      throw invalid_argument(L"Positions must be >= 0, got " +
                             to_wstring(position));
    }
  }
  for (int i = 1; i < positions.size(); ++i) {
    if (positions[i] < positions[i - 1]) {
      throw invalid_argument(L"Positions should not go backwards, got " +
                             to_wstring(positions[i - 1]) + L" before " +
                             to_wstring(positions[i]));
    }
  }
}

std::deque<int> PhraseQuery::incrementalPositions(int length)
{
  std::deque<int> positions(length);
  for (int i = 0; i < length; ++i) {
    positions[i] = i;
  }
  return positions;
}

std::deque<std::shared_ptr<Term>>
PhraseQuery::toTerms(const wstring &field, deque<wstring> &termStrings)
{
  std::deque<std::shared_ptr<Term>> terms(termStrings->length);
  for (int i = 0; i < terms.size(); ++i) {
    terms[i] = make_shared<Term>(field, termStrings[i]);
  }
  return terms;
}

std::deque<std::shared_ptr<Term>>
PhraseQuery::toTerms(const wstring &field, deque<BytesRef> &termBytes)
{
  std::deque<std::shared_ptr<Term>> terms(termBytes->length);
  for (int i = 0; i < terms.size(); ++i) {
    terms[i] = make_shared<Term>(field, termBytes[i]);
  }
  return terms;
}

PhraseQuery::PhraseQuery(int slop, const wstring &field, deque<wstring> &terms)
    : PhraseQuery(slop, toTerms(field, terms),
                  incrementalPositions(terms.length))
{
}

PhraseQuery::PhraseQuery(const wstring &field, deque<wstring> &terms)
    : PhraseQuery(0, field, terms)
{
}

PhraseQuery::PhraseQuery(int slop, const wstring &field,
                         deque<BytesRef> &terms)
    : PhraseQuery(slop, toTerms(field, terms),
                  incrementalPositions(terms.length))
{
}

PhraseQuery::PhraseQuery(const wstring &field, deque<BytesRef> &terms)
    : PhraseQuery(0, field, terms)
{
}

int PhraseQuery::getSlop() { return slop; }

std::deque<std::shared_ptr<Term>> PhraseQuery::getTerms() { return terms; }

std::deque<int> PhraseQuery::getPositions() { return positions; }

shared_ptr<Query>
PhraseQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (terms.empty()) {
    return make_shared<MatchNoDocsQuery>(L"empty PhraseQuery");
  } else if (terms.size() == 1) {
    return make_shared<TermQuery>(terms[0]);
  } else if (positions[0] != 0) {
    std::deque<int> newPositions(positions.size());
    for (int i = 0; i < positions.size(); ++i) {
      newPositions[i] = positions[i] - positions[0];
    }
    return make_shared<PhraseQuery>(slop, terms, newPositions);
  } else {
    return Query::rewrite(reader);
  }
}

PhraseQuery::PostingsAndFreq::PostingsAndFreq(shared_ptr<PostingsEnum> postings,
                                              int position, deque<Term> &terms)
    : postings(postings), position(position),
      nTerms(terms.empty() ? 0 : terms.size())
{
  if (nTerms > 0) {
    if (terms.size() == 1) {
      this->terms = terms;
    } else {
      std::deque<std::shared_ptr<Term>> terms2(terms.size());
      System::arraycopy(terms, 0, terms2, 0, terms.size());
      Arrays::sort(terms2);
      this->terms = terms2;
    }
  } else {
    this->terms.clear();
  }
}

int PhraseQuery::PostingsAndFreq::compareTo(shared_ptr<PostingsAndFreq> other)
{
  if (position != other->position) {
    return position - other->position;
  }
  if (nTerms != other->nTerms) {
    return nTerms - other->nTerms;
  }
  if (nTerms == 0) {
    return 0;
  }
  for (int i = 0; i < terms.size(); i++) {
    int res = terms[i]->compareTo(other->terms[i]);
    if (res != 0) {
      return res;
    }
  }
  return 0;
}

int PhraseQuery::PostingsAndFreq::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + position;
  for (int i = 0; i < nTerms; i++) {
    result = prime * result + terms[i]->hashCode();
  }
  return result;
}

bool PhraseQuery::PostingsAndFreq::equals(any obj)
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
  shared_ptr<PostingsAndFreq> other =
      any_cast<std::shared_ptr<PostingsAndFreq>>(obj);
  if (position != other->position) {
    return false;
  }
  if (terms.empty()) {
    return other->terms.empty();
  }
  return Arrays::equals(terms, other->terms);
}

float PhraseQuery::termPositionsCost(shared_ptr<TermsEnum> termsEnum) throw(
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

shared_ptr<Weight> PhraseQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                                             bool needsScores,
                                             float boost) 
{
  return make_shared<PhraseWeightAnonymousInnerClass>(
      shared_from_this(), field, searcher, needsScores, boost);
}

PhraseQuery::PhraseWeightAnonymousInnerClass::PhraseWeightAnonymousInnerClass(
    shared_ptr<PhraseQuery> outerInstance, const wstring &field,
    shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
    bool needsScores, float boost)
    : PhraseWeight(outerInstance, field, searcher, needsScores)
{
  this->outerInstance = outerInstance;
  this->searcher = searcher;
  this->needsScores = needsScores;
  this->boost = boost;
}

shared_ptr<Similarity::SimWeight>
PhraseQuery::PhraseWeightAnonymousInnerClass::getStats(
    shared_ptr<IndexSearcher> searcher) 
{
  const std::deque<int> positions = outerInstance->getPositions();
  if (positions.size() < 2) {
    throw make_shared<IllegalStateException>(
        L"PhraseWeight does not support less than 2 terms, call rewrite first");
  } else if (positions[0] != 0) {
    throw make_shared<IllegalStateException>(
        L"PhraseWeight requires that the first position is 0, call rewrite "
        L"first");
  }
  shared_ptr<IndexReaderContext> *const context =
      searcher->getTopReaderContext();
  states =
      std::deque<std::shared_ptr<TermContext>>(outerInstance->terms.size());
  std::deque<std::shared_ptr<TermStatistics>> termStats(
      outerInstance->terms.size());
  int termUpTo = 0;
  for (int i = 0; i < outerInstance->terms.size(); i++) {
    shared_ptr<Term> *const term = outerInstance->terms[i];
    states[i] = TermContext::build(context, term);
    if (needsScores) {
      shared_ptr<TermStatistics> termStatistics =
          searcher->termStatistics(term, states[i]);
      if (termStatistics != nullptr) {
        termStats[termUpTo++] = termStatistics;
      }
    }
  }
  if (termUpTo > 0) {
    return similarity::computeWeight(
        boost, searcher->collectionStatistics(outerInstance->field),
        Arrays::copyOf(termStats, termUpTo));
  } else {
    return nullptr; // no terms at all, we won't use similarity
  }
}

shared_ptr<PhraseMatcher>
PhraseQuery::PhraseWeightAnonymousInnerClass::getPhraseMatcher(
    shared_ptr<LeafReaderContext> context,
    bool exposeOffsets) 
{
  assert(outerInstance->terms.size() > 0);
  shared_ptr<LeafReader> *const reader = context->reader();
  std::deque<std::shared_ptr<PostingsAndFreq>> postingsFreqs(
      outerInstance->terms.size());

  shared_ptr<Terms> *const fieldTerms = reader->terms(outerInstance->field);
  if (fieldTerms == nullptr) {
    return nullptr;
  }

  if (fieldTerms->hasPositions() == false) {
    throw make_shared<IllegalStateException>(
        L"field \"" + outerInstance->field +
        L"\" was indexed without position data; cannot run PhraseQuery "
        L"(phrase=" +
        getQuery() + L")");
  }

  // Reuse single TermsEnum below:
  shared_ptr<TermsEnum> *const te = fieldTerms->begin();
  float totalMatchCost = 0;

  for (int i = 0; i < outerInstance->terms.size(); i++) {
    shared_ptr<Term> *const t = outerInstance->terms[i];
    shared_ptr<TermState> *const state = states[i]->get(context->ord);
    if (state == nullptr) { // term doesnt exist in this segment
      assert((termNotInReader(reader, t),
              L"no termstate found but term exists in reader"));
      return nullptr;
    }
    te->seekExact(t->bytes(), state);
    shared_ptr<PostingsEnum> postingsEnum =
        te->postings(nullptr, exposeOffsets ? PostingsEnum::OFFSETS
                                            : PostingsEnum::POSITIONS);
    postingsFreqs[i] = make_shared<PostingsAndFreq>(
        postingsEnum, outerInstance->positions[i], t);
    totalMatchCost += termPositionsCost(te);
  }

  // sort by increasing docFreq order
  if (outerInstance->slop == 0) {
    ArrayUtil::timSort(postingsFreqs);
    return make_shared<ExactPhraseMatcher>(postingsFreqs, totalMatchCost);
  } else {
    return make_shared<SloppyPhraseMatcher>(postingsFreqs, outerInstance->slop,
                                            totalMatchCost);
  }
}

void PhraseQuery::PhraseWeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> queryTerms)
{
  Collections::addAll(queryTerms, outerInstance->terms);
}

bool PhraseQuery::termNotInReader(shared_ptr<LeafReader> reader,
                                  shared_ptr<Term> term) 
{
  return reader->docFreq(term) == 0;
}

wstring PhraseQuery::toString(const wstring &f)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (field != L"" && field != f) {
    buffer->append(field);
    buffer->append(L":");
  }

  buffer->append(L"\"");
  constexpr int maxPosition;
  if (positions.empty()) {
    maxPosition = -1;
  } else {
    maxPosition = positions[positions.size() - 1];
  }
  std::deque<wstring> pieces(maxPosition + 1);
  for (int i = 0; i < terms.size(); i++) {
    int pos = positions[i];
    wstring s = pieces[pos];
    if (s == L"") {
      s = (terms[i])->text();
    } else {
      s = s + L"|" + (terms[i])->text();
    }
    pieces[pos] = s;
  }
  for (int i = 0; i < pieces.size(); i++) {
    if (i > 0) {
      buffer->append(L' ');
    }
    wstring s = pieces[i];
    if (s == L"") {
      buffer->append(L'?');
    } else {
      buffer->append(s);
    }
  }
  buffer->append(L"\"");

  if (slop != 0) {
    buffer->append(L"~");
    buffer->append(slop);
  }

  return buffer->toString();
}

bool PhraseQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool PhraseQuery::equalsTo(shared_ptr<PhraseQuery> other)
{
  return slop == other->slop && Arrays::equals(terms, other->terms) &&
         Arrays::equals(positions, other->positions);
}

int PhraseQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + slop;
  h = 31 * h + Arrays::hashCode(terms);
  h = 31 * h + Arrays::hashCode(positions);
  return h;
}
} // namespace org::apache::lucene::search