using namespace std;

#include "SortedDocValuesTermsEnum.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

SortedDocValuesTermsEnum::SortedDocValuesTermsEnum(
    shared_ptr<SortedDocValues> values)
    : values(values), scratch(make_shared<BytesRefBuilder>())
{
}

SeekStatus
SortedDocValuesTermsEnum::seekCeil(shared_ptr<BytesRef> text) 
{
  int ord = values->lookupTerm(text);
  if (ord >= 0) {
    currentOrd = ord;
    scratch->copyBytes(text);
    return SeekStatus::FOUND;
  } else {
    currentOrd = -ord - 1;
    if (currentOrd == values->getValueCount()) {
      return SeekStatus::END;
    } else {
      // TODO: hmm can we avoid this "extra" lookup?:
      scratch->copyBytes(values->lookupOrd(currentOrd));
      return SeekStatus::NOT_FOUND;
    }
  }
}

bool SortedDocValuesTermsEnum::seekExact(shared_ptr<BytesRef> text) throw(
    IOException)
{
  int ord = values->lookupTerm(text);
  if (ord >= 0) {
    currentOrd = ord;
    scratch->copyBytes(text);
    return true;
  } else {
    return false;
  }
}

void SortedDocValuesTermsEnum::seekExact(int64_t ord) 
{
  assert(ord >= 0 && ord < values->getValueCount());
  currentOrd = static_cast<int>(ord);
  scratch->copyBytes(values->lookupOrd(currentOrd));
}

shared_ptr<BytesRef> SortedDocValuesTermsEnum::next() 
{
  currentOrd++;
  if (currentOrd >= values->getValueCount()) {
    return nullptr;
  }
  scratch->copyBytes(values->lookupOrd(currentOrd));
  return scratch->get();
}

shared_ptr<BytesRef> SortedDocValuesTermsEnum::term() 
{
  return scratch->get();
}

int64_t SortedDocValuesTermsEnum::ord() 
{
  return currentOrd;
}

int SortedDocValuesTermsEnum::docFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t SortedDocValuesTermsEnum::totalTermFreq() { return -1; }

shared_ptr<PostingsEnum>
SortedDocValuesTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                                   int flags) 
{
  throw make_shared<UnsupportedOperationException>();
}

void SortedDocValuesTermsEnum::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state) 
{
  assert(state != nullptr &&
         std::dynamic_pointer_cast<OrdTermState>(state) != nullptr);
  this->seekExact((std::static_pointer_cast<OrdTermState>(state))->ord);
}

shared_ptr<TermState> SortedDocValuesTermsEnum::termState() 
{
  shared_ptr<OrdTermState> state = make_shared<OrdTermState>();
  state->ord = currentOrd;
  return state;
}
} // namespace org::apache::lucene::index