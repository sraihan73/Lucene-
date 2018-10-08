using namespace std;

#include "BlockTermState.h"
#include "../index/TermState.h"

namespace org::apache::lucene::codecs
{
using OrdTermState = org::apache::lucene::index::OrdTermState;
using TermState = org::apache::lucene::index::TermState;

BlockTermState::BlockTermState() {}

void BlockTermState::copyFrom(shared_ptr<TermState> _other)
{
  assert((std::dynamic_pointer_cast<BlockTermState>(_other) != nullptr,
          L"can not copy from " + _other->getClassName()));
  shared_ptr<BlockTermState> other =
      std::static_pointer_cast<BlockTermState>(_other);
  OrdTermState::copyFrom(_other);
  docFreq = other->docFreq;
  totalTermFreq = other->totalTermFreq;
  termBlockOrd = other->termBlockOrd;
  blockFilePointer = other->blockFilePointer;
}

wstring BlockTermState::toString()
{
  return L"docFreq=" + to_wstring(docFreq) + L" totalTermFreq=" +
         to_wstring(totalTermFreq) + L" termBlockOrd=" +
         to_wstring(termBlockOrd) + L" blockFP=" + to_wstring(blockFilePointer);
}
} // namespace org::apache::lucene::codecs