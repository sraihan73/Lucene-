using namespace std;

#include "MultiPhraseQuery.h"

namespace org::apache::lucene::search
{
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
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

MultiPhraseQuery::Builder::Builder()
    : termArrays(deque<>()), positions(deque<>())
{
  this->field = L"";
  this->slop = 0;
}

MultiPhraseQuery::Builder::Builder(
    shared_ptr<MultiPhraseQuery> multiPhraseQuery)
    : termArrays(deque<>(length)), positions(deque<>(length))
{
  this->field = multiPhraseQuery->field;

  int length = multiPhraseQuery->termArrays.length;

  for (int i = 0; i < length; ++i) {
    this->termArrays.push_back(multiPhraseQuery->termArrays[i]);
    this->positions.push_back(multiPhraseQuery->positions[i]);
  }

  this->slop = multiPhraseQuery->slop;
}

shared_ptr<Builder> MultiPhraseQuery::Builder::setSlop(int s)
{
  if (s < 0) {
    throw invalid_argument(L"slop value cannot be negative");
  }
  slop = s;

  return shared_from_this();
}

shared_ptr<Builder> MultiPhraseQuery::Builder::add(shared_ptr<Term> term)
{
  return add(std::deque<std::shared_ptr<Term>>{term});
}

shared_ptr<Builder>
MultiPhraseQuery::Builder::add(std::deque<std::shared_ptr<Term>> &terms)
{
  int position = 0;
  if (positions.size() > 0) {
    position = positions[positions.size() - 1] + 1;
  }

  return add(terms, position);
}

shared_ptr<Builder>
MultiPhraseQuery::Builder::add(std::deque<std::shared_ptr<Term>> &terms,
                               int position)
{
  Objects::requireNonNull(terms, L"Term array must not be null");
  if (termArrays.empty()) {
    field = terms[0]->field();
  }

  for (auto term : terms) {
    if (term->field() != field) {
      throw invalid_argument(L"All phrase terms must be in the same field (" +
                             field + L"): " + term);
    }
  }

  termArrays.push_back(terms);
  positions.push_back(position);

  return shared_from_this();
}

shared_ptr<MultiPhraseQuery> MultiPhraseQuery::Builder::build()
{
  std::deque<int> positionsArray(this->positions.size());

  for (int i = 0; i < this->positions.size(); ++i) {
    positionsArray[i] = this->positions[i];
  }

  std::deque<std::deque<std::shared_ptr<Term>>> termArraysArray =
      termArrays.toArray(
          std::deque<std::deque<std::shared_ptr<Term>>>(termArrays.size()));

  return make_shared<MultiPhraseQuery>(field, termArraysArray, positionsArray,
                                       slop);
}

MultiPhraseQuery::MultiPhraseQuery(
    const wstring &field,
    std::deque<std::deque<std::shared_ptr<Term>>> &termArrays,
    std::deque<int> &positions, int slop)
    : field(field), termArrays(termArrays), positions(positions), slop(slop)
{
  // No argument checks here since they are provided by the
  // MultiPhraseQuery.Builder
}

int MultiPhraseQuery::getSlop() { return slop; }

std::deque<std::deque<std::shared_ptr<Term>>>
MultiPhraseQuery::getTermArrays()
{
  return termArrays;
}

std::deque<int> MultiPhraseQuery::getPositions() { return positions; }

shared_ptr<Query>
MultiPhraseQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  if (termArrays.empty()) {
    return make_shared<MatchNoDocsQuery>(L"empty MultiPhraseQuery");
  } else if (termArrays.size() == 1) { // optimize one-term case
    std::deque<std::shared_ptr<Term>> terms = termArrays[0];
    shared_ptr<BooleanQuery::Builder> builder =
        make_shared<BooleanQuery::Builder>();
    for (auto term : terms) {
      builder->add(make_shared<TermQuery>(term), BooleanClause::Occur::SHOULD);
    }
    return builder->build();
  } else {
    return Query::rewrite(reader);
  }
}

shared_ptr<Weight>
MultiPhraseQuery::createWeight(shared_ptr<IndexSearcher> searcher,
                               bool needsScores, float boost) 
{
  const unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
      termStates =
          unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>();
  return make_shared<PhraseWeightAnonymousInnerClass>(
      shared_from_this(), field, searcher, needsScores, boost, termStates);
}

MultiPhraseQuery::PhraseWeightAnonymousInnerClass::
    PhraseWeightAnonymousInnerClass(
        shared_ptr<MultiPhraseQuery> outerInstance, const wstring &field,
        shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores, float boost,
        unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &termStates)
    : PhraseWeight(outerInstance, field, searcher, needsScores)
{
  this->outerInstance = outerInstance;
  this->searcher = searcher;
  this->needsScores = needsScores;
  this->boost = boost;
  this->termStates = termStates;
}

shared_ptr<Similarity::SimWeight>
MultiPhraseQuery::PhraseWeightAnonymousInnerClass::getStats(
    shared_ptr<IndexSearcher> searcher) 
{
  shared_ptr<IndexReaderContext> *const context =
      searcher->getTopReaderContext();

  // compute idf
  deque<std::shared_ptr<TermStatistics>> allTermStats =
      deque<std::shared_ptr<TermStatistics>>();
  for (auto terms : outerInstance->termArrays) {
    for (auto term : terms) {
      shared_ptr<TermContext> ts = termStates[term];
      if (ts == nullptr) {
        ts = TermContext::build(context, term);
        termStates.emplace(term, ts);
      }
      if (needsScores) {
        shared_ptr<TermStatistics> termStatistics =
            searcher->termStatistics(term, ts);
        if (termStatistics != nullptr) {
          allTermStats.push_back(termStatistics);
        }
      }
      allTermStats.push_back(searcher->termStatistics(term, ts));
    }
  }
  if (allTermStats.empty()) {
    return nullptr; // none of the terms were found, we won't use sim at all
  } else {
    return similarity::computeWeight(
        boost, searcher->collectionStatistics(outerInstance->field),
        allTermStats.toArray(
            std::deque<std::shared_ptr<TermStatistics>>(allTermStats.size())));
  }
}

shared_ptr<PhraseMatcher>
MultiPhraseQuery::PhraseWeightAnonymousInnerClass::getPhraseMatcher(
    shared_ptr<LeafReaderContext> context,
    bool exposeOffsets) 
{
  assert(outerInstance->termArrays.size() != 0);
  shared_ptr<LeafReader> *const reader = context->reader();

  std::deque<std::shared_ptr<PhraseQuery::PostingsAndFreq>> postingsFreqs(
      outerInstance->termArrays.size());

  shared_ptr<Terms> *const fieldTerms = reader->terms(outerInstance->field);
  if (fieldTerms == nullptr) {
    return nullptr;
  }

  // TODO: move this check to createWeight to happen earlier to the user?
  if (fieldTerms->hasPositions() == false) {
    throw make_shared<IllegalStateException>(
        L"field \"" + outerInstance->field +
        L"\" was indexed without position data;" +
        L" cannot run MultiPhraseQuery (phrase=" + getQuery() + L")");
  }

  // Reuse single TermsEnum below:
  shared_ptr<TermsEnum> *const termsEnum = fieldTerms->begin();
  float totalMatchCost = 0;

  for (int pos = 0; pos < postingsFreqs.size(); pos++) {
    std::deque<std::shared_ptr<Term>> terms = outerInstance->termArrays[pos];
    deque<std::shared_ptr<PostingsEnum>> postings =
        deque<std::shared_ptr<PostingsEnum>>();

    for (auto term : terms) {
      shared_ptr<TermState> termState = termStates[term]->get(context->ord);
      if (termState != nullptr) {
        termsEnum->seekExact(term->bytes(), termState);
        postings.push_back(termsEnum->postings(
            nullptr,
            exposeOffsets ? PostingsEnum::OFFSETS : PostingsEnum::POSITIONS));
        totalMatchCost += PhraseQuery::termPositionsCost(termsEnum);
      }
    }

    if (postings.empty()) {
      return nullptr;
    }

    shared_ptr<PostingsEnum> *const postingsEnum;
    if (postings.size() == 1) {
      postingsEnum = postings[0];
    } else {
      postingsEnum = exposeOffsets
                         ? make_shared<UnionFullPostingsEnum>(postings)
                         : make_shared<UnionPostingsEnum>(postings);
    }

    postingsFreqs[pos] = make_shared<PhraseQuery::PostingsAndFreq>(
        postingsEnum, outerInstance->positions[pos], terms);
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

void MultiPhraseQuery::PhraseWeightAnonymousInnerClass::extractTerms(
    shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  for (auto arr : outerInstance->termArrays) {
    Collections::addAll(terms, arr);
  }
}

wstring MultiPhraseQuery::toString(const wstring &f)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (field == L"" || field != f) {
    buffer->append(field);
    buffer->append(L":");
  }

  buffer->append(L"\"");
  int lastPos = -1;

  for (int i = 0; i < termArrays.size(); ++i) {
    std::deque<std::shared_ptr<Term>> terms = termArrays[i];
    int position = positions[i];
    if (i != 0) {
      buffer->append(L" ");
      for (int j = 1; j < (position - lastPos); j++) {
        buffer->append(L"? ");
      }
    }
    if (terms.size() > 1) {
      buffer->append(L"(");
      for (int j = 0; j < terms.size(); j++) {
        buffer->append(terms[j]->text());
        if (j < terms.size() - 1) {
          buffer->append(L" ");
        }
      }
      buffer->append(L")");
    } else {
      buffer->append(terms[0]->text());
    }
    lastPos = position;
  }
  buffer->append(L"\"");

  if (slop != 0) {
    buffer->append(L"~");
    buffer->append(slop);
  }

  return buffer->toString();
}

bool MultiPhraseQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool MultiPhraseQuery::equalsTo(shared_ptr<MultiPhraseQuery> other)
{
  return this->slop == other->slop &&
         termArraysEquals(this->termArrays, other->termArrays) &&
         Arrays::equals(this->positions, other->positions);
}

int MultiPhraseQuery::hashCode()
{
  return classHash() ^ slop ^ termArraysHashCode() ^
         Arrays::hashCode(positions);
}

int MultiPhraseQuery::termArraysHashCode()
{
  int hashCode = 1;
  for (auto termArray : termArrays) {
    hashCode =
        31 * hashCode + (termArray.empty() ? 0 : Arrays::hashCode(termArray));
  }
  return hashCode;
}

bool MultiPhraseQuery::termArraysEquals(
    std::deque<std::deque<std::shared_ptr<Term>>> &termArrays1,
    std::deque<std::deque<std::shared_ptr<Term>>> &termArrays2)
{
  if (termArrays1.size() != termArrays2.size()) {
    return false;
  }

  for (int i = 0; i < termArrays1.size(); ++i) {
    std::deque<std::shared_ptr<Term>> termArray1 = termArrays1[i];
    std::deque<std::shared_ptr<Term>> termArray2 = termArrays2[i];
    if (!(termArray1.empty() ? termArray2.empty()
                             : Arrays::equals(termArray1, termArray2))) {
      return false;
    }
  }
  return true;
}

MultiPhraseQuery::UnionPostingsEnum::UnionPostingsEnum(
    shared_ptr<deque<std::shared_ptr<PostingsEnum>>> subs)
    : docsQueue(make_shared<DocsQueue>(subs->size())), cost(cost_),
      subs(subs->toArray(
          std::deque<std::shared_ptr<PostingsEnum>>(subs->size())))
{
  int64_t cost = 0;
  for (auto sub : subs) {
    docsQueue->push_back(sub);
    cost += sub->cost();
  }
}

int MultiPhraseQuery::UnionPostingsEnum::freq() 
{
  int doc = docID();
  if (doc != posQueueDoc) {
    posQueue->clear();
    for (auto sub : subs) {
      if (sub->docID() == doc) {
        int freq = sub->freq();
        for (int i = 0; i < freq; i++) {
          posQueue->add(sub->nextPosition());
        }
      }
    }
    posQueue->sort();
    posQueueDoc = doc;
  }
  return posQueue->size();
}

int MultiPhraseQuery::UnionPostingsEnum::nextPosition() 
{
  return posQueue->next();
}

int MultiPhraseQuery::UnionPostingsEnum::docID()
{
  return docsQueue->top()->docID();
}

int MultiPhraseQuery::UnionPostingsEnum::nextDoc() 
{
  shared_ptr<PostingsEnum> top = docsQueue->top();
  int doc = top->docID();

  do {
    top->nextDoc();
    top = docsQueue->updateTop();
  } while (top->docID() == doc);

  return top->docID();
}

int MultiPhraseQuery::UnionPostingsEnum::advance(int target) 
{
  shared_ptr<PostingsEnum> top = docsQueue->top();

  do {
    top->advance(target);
    top = docsQueue->updateTop();
  } while (top->docID() < target);

  return top->docID();
}

int64_t MultiPhraseQuery::UnionPostingsEnum::cost() { return cost_; }

int MultiPhraseQuery::UnionPostingsEnum::startOffset() 
{
  return -1; // offsets are unsupported
}

int MultiPhraseQuery::UnionPostingsEnum::endOffset() 
{
  return -1; // offsets are unsupported
}

shared_ptr<BytesRef>
MultiPhraseQuery::UnionPostingsEnum::getPayload() 
{
  return nullptr; // payloads are unsupported
}

MultiPhraseQuery::UnionPostingsEnum::DocsQueue::DocsQueue(int size)
    : org::apache::lucene::util::PriorityQueue<
          org::apache::lucene::index::PostingsEnum>(size)
{
}

bool MultiPhraseQuery::UnionPostingsEnum::DocsQueue::lessThan(
    shared_ptr<PostingsEnum> a, shared_ptr<PostingsEnum> b)
{
  return a->docID() < b->docID();
}

void MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::add(int i)
{
  if (size_ == arraySize) {
    growArray();
  }

  array_[size_++] = i;
}

int MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::next()
{
  return array_[index++];
}

void MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::sort()
{
  Arrays::sort(array_, index, size_);
}

void MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::clear()
{
  index = 0;
  size_ = 0;
}

int MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::size()
{
  return size_;
}

void MultiPhraseQuery::UnionPostingsEnum::PositionsQueue::growArray()
{
  std::deque<int> newArray(arraySize * 2);
  System::arraycopy(array_, 0, newArray, 0, arraySize);
  array_ = newArray;
  arraySize *= 2;
}

MultiPhraseQuery::PostingsAndPosition::PostingsAndPosition(
    shared_ptr<PostingsEnum> pe)
    : pe(pe)
{
}

MultiPhraseQuery::UnionFullPostingsEnum::UnionFullPostingsEnum(
    deque<std::shared_ptr<PostingsEnum>> &subs)
    : UnionPostingsEnum(subs),
      posQueue(make_shared<PriorityQueue<std::shared_ptr<PostingsAndPosition>>>(
          subs.size())),
      subs(deque<std::shared_ptr<org::apache::lucene::index::PostingsEnum>>())
{
  {protected : bool lessThan(PostingsAndPosition a,
                             PostingsAndPosition b){return a::pos < b::pos;
}
}; // namespace org::apache::lucene::search
for (auto pe : subs) {
  this->subs->add(make_shared<PostingsAndPosition>(pe));
}
}

int MultiPhraseQuery::UnionFullPostingsEnum::freq() 
{
  int doc = docID();
  if (doc == posQueueDoc) {
    return freq_;
  }
  freq_ = 0;
  started = false;
  posQueue->clear();
  for (auto pp : subs) {
    if (pp->pe->docID() == doc) {
      pp->pos = pp->pe->nextPosition();
      pp->upto = pp->pe->freq();
      posQueue->push_back(pp);
      freq_ += pp->upto;
    }
  }
  return freq_;
}

int MultiPhraseQuery::UnionFullPostingsEnum::nextPosition() 
{
  if (started == false) {
    started = true;
    return posQueue->top()->pos;
  }
  if (posQueue->top()->upto == 1) {
    posQueue->pop();
    return posQueue->top()->pos;
  }
  posQueue->top()->pos = posQueue->top()->pe->nextPosition();
  posQueue->top()->upto--;
  posQueue->updateTop();
  return posQueue->top()->pos;
}

int MultiPhraseQuery::UnionFullPostingsEnum::startOffset() 
{
  return posQueue->top()->pe->startOffset();
}

int MultiPhraseQuery::UnionFullPostingsEnum::endOffset() 
{
  return posQueue->top()->pe->endOffset();
}

shared_ptr<BytesRef>
MultiPhraseQuery::UnionFullPostingsEnum::getPayload() 
{
  return posQueue->top()->pe->getPayload();
}
}