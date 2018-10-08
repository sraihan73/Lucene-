using namespace std;

#include "IDVersionTermState.h"

namespace org::apache::lucene::codecs::idversion
{
using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using TermState = org::apache::lucene::index::TermState;

shared_ptr<IDVersionTermState> IDVersionTermState::clone()
{
  shared_ptr<IDVersionTermState> other = make_shared<IDVersionTermState>();
  other->copyFrom(shared_from_this());
  return other;
}

void IDVersionTermState::copyFrom(shared_ptr<TermState> _other)
{
  BlockTermState::copyFrom(_other);
  shared_ptr<IDVersionTermState> other =
      std::static_pointer_cast<IDVersionTermState>(_other);
  idVersion = other->idVersion;
  docID = other->docID;
}
} // namespace org::apache::lucene::codecs::idversion