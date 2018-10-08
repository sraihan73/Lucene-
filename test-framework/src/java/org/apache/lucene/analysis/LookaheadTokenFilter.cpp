using namespace std;

#include "LookaheadTokenFilter.h"

namespace org::apache::lucene::analysis
{
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;

void LookaheadTokenFilter<T>::Position::reset()
{
  inputTokens.clear();
  nextRead = 0;
  startOffset = -1;
  endOffset = -1;
}

void LookaheadTokenFilter<T>::Position::add(
    shared_ptr<AttributeSource::State> state)
{
  inputTokens.push_back(state);
}

shared_ptr<AttributeSource::State>
LookaheadTokenFilter<T>::Position::nextState()
{
  assert(nextRead < inputTokens.size());
  return inputTokens[nextRead++];
}

LookaheadTokenFilter<
    T>::RollingBufferAnonymousInnerClass::RollingBufferAnonymousInnerClass()
{
}

shared_ptr<T>
LookaheadTokenFilter<T>::RollingBufferAnonymousInnerClass::newInstance()
{
  return outerInstance->newPosition();
}
} // namespace org::apache::lucene::analysis