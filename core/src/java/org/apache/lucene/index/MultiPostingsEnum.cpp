using namespace std;

#include "MultiPostingsEnum.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;

MultiPostingsEnum::MultiPostingsEnum(shared_ptr<MultiTermsEnum> parent,
                                     int subReaderCount)
    : parent(parent),
      subPostingsEnums(
          std::deque<std::shared_ptr<PostingsEnum>>(subReaderCount)),
      subs(std::deque<std::shared_ptr<EnumWithSlice>>(subReaderCount))
{
  for (int i = 0; i < subs.size(); i++) {
    subs[i] = make_shared<EnumWithSlice>();
  }
}

bool MultiPostingsEnum::canReuse(shared_ptr<MultiTermsEnum> parent)
{
  return this->parent == parent;
}

shared_ptr<MultiPostingsEnum>
MultiPostingsEnum::reset(std::deque<std::shared_ptr<EnumWithSlice>> &subs,
                         int const numSubs)
{
  this->numSubs = numSubs;
  for (int i = 0; i < numSubs; i++) {
    this->subs[i]->postingsEnum = subs[i]->postingsEnum;
    this->subs[i]->slice = subs[i]->slice;
  }
  upto = -1;
  doc = -1;
  current.reset();
  return shared_from_this();
}

int MultiPostingsEnum::getNumSubs() { return numSubs; }

std::deque<std::shared_ptr<EnumWithSlice>> MultiPostingsEnum::getSubs()
{
  return subs;
}

int MultiPostingsEnum::freq() 
{
  assert(current != nullptr);
  return current->freq();
}

int MultiPostingsEnum::docID() { return doc; }

int MultiPostingsEnum::advance(int target) 
{
  assert(target > doc);
  while (true) {
    if (current != nullptr) {
      constexpr int doc;
      if (target < currentBase) {
        // target was in the previous slice but there was no matching doc after
        // it
        doc = current->nextDoc();
      } else {
        doc = current->advance(target - currentBase);
      }
      if (doc == NO_MORE_DOCS) {
        current.reset();
      } else {
        return this->doc = doc + currentBase;
      }
    } else if (upto == numSubs - 1) {
      return this->doc = NO_MORE_DOCS;
    } else {
      upto++;
      current = subs[upto]->postingsEnum;
      currentBase = subs[upto]->slice.start;
    }
  }
}

int MultiPostingsEnum::nextDoc() 
{
  while (true) {
    if (current == nullptr) {
      if (upto == numSubs - 1) {
        return this->doc = NO_MORE_DOCS;
      } else {
        upto++;
        current = subs[upto]->postingsEnum;
        currentBase = subs[upto]->slice.start;
      }
    }

    constexpr int doc = current->nextDoc();
    if (doc != NO_MORE_DOCS) {
      return this->doc = currentBase + doc;
    } else {
      current.reset();
    }
  }
}

int MultiPostingsEnum::nextPosition() 
{
  return current->nextPosition();
}

int MultiPostingsEnum::startOffset() 
{
  return current->startOffset();
}

int MultiPostingsEnum::endOffset() 
{
  return current->endOffset();
}

shared_ptr<BytesRef> MultiPostingsEnum::getPayload() 
{
  return current->getPayload();
}

MultiPostingsEnum::EnumWithSlice::EnumWithSlice() {}

wstring MultiPostingsEnum::EnumWithSlice::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return slice->toString() + L":" + postingsEnum;
}

int64_t MultiPostingsEnum::cost()
{
  int64_t cost = 0;
  for (int i = 0; i < numSubs; i++) {
    cost += subs[i]->postingsEnum.cost();
  }
  return cost;
}

wstring MultiPostingsEnum::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"MultiDocsAndPositionsEnum(" + Arrays->toString(getSubs()) + L")";
}
} // namespace org::apache::lucene::index