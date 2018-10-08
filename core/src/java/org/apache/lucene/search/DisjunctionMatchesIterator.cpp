using namespace std;

#include "DisjunctionMatchesIterator.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

shared_ptr<MatchesIterator> DisjunctionMatchesIterator::fromTerms(
    shared_ptr<LeafReaderContext> context, int doc, const wstring &field,
    deque<std::shared_ptr<Term>> &terms) 
{
  Objects::requireNonNull(field);
  for (auto term : terms) {
    if (Objects::equals(field, term->field()) == false) {
      throw invalid_argument(L"Tried to generate iterator from terms in "
                             L"multiple fields: expected [" +
                             field + L"] but got [" + term->field() + L"]");
    }
  }
  return fromTermsEnum(context, doc, field, asBytesRefIterator(terms));
}

shared_ptr<BytesRefIterator> DisjunctionMatchesIterator::asBytesRefIterator(
    deque<std::shared_ptr<Term>> &terms)
{
  return make_shared<BytesRefIteratorAnonymousInnerClass>(terms);
}

DisjunctionMatchesIterator::BytesRefIteratorAnonymousInnerClass::
    BytesRefIteratorAnonymousInnerClass(deque<std::shared_ptr<Term>> &terms)
{
  this->terms = terms;
}

shared_ptr<BytesRef>
DisjunctionMatchesIterator::BytesRefIteratorAnonymousInnerClass::next()
{
  if (i >= terms.size()) {
    return nullptr;
  }
  return terms[i++]->bytes();
}

shared_ptr<MatchesIterator> DisjunctionMatchesIterator::fromTermsEnum(
    shared_ptr<LeafReaderContext> context, int doc, const wstring &field,
    shared_ptr<BytesRefIterator> terms) 
{
  Objects::requireNonNull(field);
  deque<std::shared_ptr<MatchesIterator>> mis =
      deque<std::shared_ptr<MatchesIterator>>();
  shared_ptr<Terms> t = context->reader()->terms(field);
  if (t == nullptr) {
    return nullptr;
  }
  shared_ptr<TermsEnum> te = t->begin();
  shared_ptr<PostingsEnum> reuse = nullptr;
  for (shared_ptr<BytesRef> term = terms->next(); term != nullptr;
       term = terms->next()) {
    if (te->seekExact(term)) {
      shared_ptr<PostingsEnum> pe = te->postings(reuse, PostingsEnum::OFFSETS);
      if (pe->advance(doc) == doc) {
        mis.push_back(make_shared<TermMatchesIterator>(pe));
        reuse.reset();
      } else {
        reuse = pe;
      }
    }
  }
  return fromSubIterators(mis);
}

shared_ptr<MatchesIterator> DisjunctionMatchesIterator::fromSubIterators(
    deque<std::shared_ptr<MatchesIterator>> &mis) 
{
  if (mis.empty()) {
    return nullptr;
  }
  if (mis.size() == 1) {
    return mis[0];
  }
  return make_shared<DisjunctionMatchesIterator>(mis);
}

DisjunctionMatchesIterator::DisjunctionMatchesIterator(
    deque<std::shared_ptr<MatchesIterator>> &matches) 
    : queue(make_shared<PriorityQueue<std::shared_ptr<MatchesIterator>>>(
          matches.size()))
{
  {protected : bool lessThan(MatchesIterator a, MatchesIterator b){
      return a::startPosition() < b::startPosition() ||
             (a::startPosition() == b::startPosition() &&
              a::endPosition() < b::endPosition()) ||
             (a::startPosition() == b::startPosition() &&
              a::endPosition() == b::endPosition());
}
}; // namespace org::apache::lucene::search
for (auto mi : matches) {
  if (*mi) {
    queue->push_back(mi);
  }
}
}

bool DisjunctionMatchesIterator::next() 
{
  if (started == false) {
    return started = true;
  }
  if (queue->top()->next() == false) {
    queue->pop();
  }
  if (queue->size() > 0) {
    queue->updateTop();
    return true;
  }
  return false;
}

int DisjunctionMatchesIterator::startPosition()
{
  return queue->top()->startPosition();
}

int DisjunctionMatchesIterator::endPosition()
{
  return queue->top()->endPosition();
}

int DisjunctionMatchesIterator::startOffset() 
{
  return queue->top()->startOffset();
}

int DisjunctionMatchesIterator::endOffset() 
{
  return queue->top()->endOffset();
}
}