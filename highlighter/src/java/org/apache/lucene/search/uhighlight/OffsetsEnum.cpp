using namespace std;

#include "OffsetsEnum.h"

namespace org::apache::lucene::search::uhighlight
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

int OffsetsEnum::compareTo(shared_ptr<OffsetsEnum> other)
{
  try {
    int cmp = Integer::compare(startOffset(), other->startOffset());
    if (cmp != 0) {
      return cmp; // vast majority of the time we return here.
    }
    shared_ptr<BytesRef> *const thisTerm = this->getTerm();
    shared_ptr<BytesRef> *const otherTerm = other->getTerm();
    if (thisTerm == nullptr || otherTerm == nullptr) {
      if (thisTerm == nullptr && otherTerm == nullptr) {
        return 0;
      } else if (thisTerm == nullptr) {
        return 1; // put "this" (wildcard mtq enum) last
      } else {
        return -1;
      }
    }
    return thisTerm->compareTo(otherTerm);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

OffsetsEnum::~OffsetsEnum() {}

wstring OffsetsEnum::toString()
{
  const wstring name = getClass().getSimpleName();
  try {
    return name + L"(term:" + getTerm()->utf8ToString() + L")";
  } catch (const runtime_error &e) {
    return name;
  }
}

OffsetsEnum::OfPostings::OfPostings(
    shared_ptr<BytesRef> term, int freq,
    shared_ptr<PostingsEnum> postingsEnum) 
    : term(Objects::requireNonNull(term)),
      postingsEnum(Objects::requireNonNull(postingsEnum)), freq(freq)
{
  this->posCounter = this->postingsEnum->freq();
}

OffsetsEnum::OfPostings::OfPostings(
    shared_ptr<BytesRef> term,
    shared_ptr<PostingsEnum> postingsEnum) 
    : OfPostings(term, postingsEnum->freq(), postingsEnum)
{
}

shared_ptr<PostingsEnum> OffsetsEnum::OfPostings::getPostingsEnum()
{
  return postingsEnum;
}

bool OffsetsEnum::OfPostings::nextPosition() 
{
  if (posCounter > 0) {
    posCounter--;
    postingsEnum->nextPosition(); // note: we don't need to save the position
    return true;
  } else {
    return false;
  }
}

shared_ptr<BytesRef> OffsetsEnum::OfPostings::getTerm() 
{
  return term;
}

int OffsetsEnum::OfPostings::startOffset() 
{
  return postingsEnum->startOffset();
}

int OffsetsEnum::OfPostings::endOffset() 
{
  return postingsEnum->endOffset();
}

int OffsetsEnum::OfPostings::freq()  { return freq_; }

const shared_ptr<OffsetsEnum> OffsetsEnum::EMPTY =
    make_shared<OffsetsEnumAnonymousInnerClass>();

OffsetsEnum::OffsetsEnumAnonymousInnerClass::OffsetsEnumAnonymousInnerClass() {}

bool OffsetsEnum::OffsetsEnumAnonymousInnerClass::nextPosition() throw(
    IOException)
{
  return false;
}

shared_ptr<BytesRef>
OffsetsEnum::OffsetsEnumAnonymousInnerClass::getTerm() 
{
  throw make_shared<UnsupportedOperationException>();
}

int OffsetsEnum::OffsetsEnumAnonymousInnerClass::startOffset() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int OffsetsEnum::OffsetsEnumAnonymousInnerClass::endOffset() 
{
  throw make_shared<UnsupportedOperationException>();
}

int OffsetsEnum::OffsetsEnumAnonymousInnerClass::freq() 
{
  return 0;
}

OffsetsEnum::MultiOffsetsEnum::MultiOffsetsEnum(
    deque<std::shared_ptr<OffsetsEnum>> &inner) 
    : queue(make_shared<PriorityQueue<>>())
{
  for (auto oe : inner) {
    if (oe->nextPosition()) {
      this->queue->add(oe);
    }
  }
}

bool OffsetsEnum::MultiOffsetsEnum::nextPosition() 
{
  if (started == false) {
    started = true;
    return this->queue->size() > 0;
  }
  if (this->queue->size() > 0) {
    shared_ptr<OffsetsEnum> top = this->queue->poll();
    if (top->nextPosition()) {
      this->queue->add(top);
      return true;
    } else {
      delete top;
    }
    return this->queue->size() > 0;
  }
  return false;
}

shared_ptr<BytesRef> OffsetsEnum::MultiOffsetsEnum::getTerm() 
{
  return this->queue->peek().getTerm();
}

int OffsetsEnum::MultiOffsetsEnum::startOffset() 
{
  return this->queue->peek().startOffset();
}

int OffsetsEnum::MultiOffsetsEnum::endOffset() 
{
  return this->queue->peek().endOffset();
}

int OffsetsEnum::MultiOffsetsEnum::freq() 
{
  return this->queue->peek().freq();
}

OffsetsEnum::MultiOffsetsEnum::~MultiOffsetsEnum()
{
  // most child enums will have been closed in .nextPosition()
  // here all remaining non-exhausted enums are closed
  IOUtils::close({queue});
}
} // namespace org::apache::lucene::search::uhighlight