using namespace std;

#include "ConditionalTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

ConditionalTokenFilter::OneTimeWrapper::OneTimeWrapper(
    shared_ptr<ConditionalTokenFilter> outerInstance,
    shared_ptr<AttributeSource> attributeSource)
    : org::apache::lucene::analysis::TokenStream(attributeSource),
      offsetAtt(attributeSource->addAttribute(OffsetAttribute::typeid)),
      posIncAtt(
          attributeSource->addAttribute(PositionIncrementAttribute::typeid)),
      outerInstance(outerInstance)
{
}

bool ConditionalTokenFilter::OneTimeWrapper::incrementToken() 
{
  if (outerInstance->state == TokenState::PREBUFFERING) {
    if (posIncAtt->getPositionIncrement() == 0) {
      outerInstance->adjustPosition = true;
      posIncAtt->setPositionIncrement(1);
    }
    outerInstance->state = TokenState::DELEGATING;
    return true;
  }
  assert(outerInstance->state == TokenState::DELEGATING);
  if (outerInstance->input->incrementToken()) {
    if (outerInstance->shouldFilter()) {
      return true;
    }
    outerInstance->endOffset = offsetAtt->endOffset();
    outerInstance->bufferedState = captureState();
  } else {
    outerInstance->exhausted = true;
  }
  return false;
}

void ConditionalTokenFilter::OneTimeWrapper::reset() 
{
  // clearing attributes etc is done by the parent stream,
  // so must be avoided here
}

void ConditionalTokenFilter::OneTimeWrapper::end() 
{
  // imitate Tokenizer.end() call - endAttributes, set final offset
  if (outerInstance->exhausted) {
    if (outerInstance->endState == nullptr) {
      outerInstance->input->end();
      outerInstance->endState = captureState();
    }
    outerInstance->endOffset = offsetAtt->endOffset();
  }
  endAttributes();
  offsetAtt->setOffset(outerInstance->endOffset, outerInstance->endOffset);
}

ConditionalTokenFilter::ConditionalTokenFilter(
    shared_ptr<TokenStream> input,
    function<TokenStream *(TokenStream *)> &inputFactory)
    : org::apache::lucene::analysis::TokenFilter(input),
      delegate_(inputFactory(
          make_shared<OneTimeWrapper>(shared_from_this(), this->input)))
{
}

void ConditionalTokenFilter::reset() 
{
  TokenFilter::reset();
  this->delegate_->reset();
  this->state = TokenState::READING;
  this->lastTokenFiltered = false;
  this->bufferedState.reset();
  this->exhausted = false;
  this->adjustPosition = false;
  this->endOffset = -1;
  this->endState.reset();
}

void ConditionalTokenFilter::end() 
{
  if (endState == nullptr) {
    TokenFilter::end();
    endState = captureState();
  } else {
    restoreState(endState);
  }
  endOffset = getAttribute(OffsetAttribute::typeid)->endOffset();
  if (lastTokenFiltered) {
    this->delegate_->end();
    endState = captureState();
  }
}

ConditionalTokenFilter::~ConditionalTokenFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  delete this->delegate_;
}

bool ConditionalTokenFilter::incrementToken() 
{
  while (true) {
    if (state == TokenState::READING) {
      if (bufferedState != nullptr) {
        restoreState(bufferedState);
        bufferedState.reset();
        lastTokenFiltered = false;
        return true;
      }
      if (exhausted == true) {
        return false;
      }
      if (input->incrementToken() == false) {
        exhausted = true;
        return false;
      }
      if (shouldFilter()) {
        lastTokenFiltered = true;
        state = TokenState::PREBUFFERING;
        // we determine that the delegate has emitted all the tokens it can at
        // the current position when OneTimeWrapper.incrementToken() is called
        // in DELEGATING state.  To signal this back to the delegate, we return
        // false, so we now need to reset it to ensure that it can continue to
        // emit more tokens
        delegate_->reset();
        bool more = delegate_->incrementToken();
        if (more) {
          state = TokenState::DELEGATING;
          if (adjustPosition) {
            int posInc = posIncAtt->getPositionIncrement();
            posIncAtt->setPositionIncrement(posInc - 1);
          }
          adjustPosition = false;
        } else {
          lastTokenFiltered = false;
          state = TokenState::READING;
          return endDelegating();
        }
        return true;
      }
      lastTokenFiltered = false;
      return true;
    }
    if (state == TokenState::DELEGATING) {
      if (delegate_->incrementToken()) {
        return true;
      }
      // no more cached tokens
      state = TokenState::READING;
      return endDelegating();
    }
  }
}

bool ConditionalTokenFilter::endDelegating() 
{
  if (bufferedState == nullptr) {
    assert(exhausted == true);
    return false;
  }
  delegate_->end();
  int posInc = posIncAtt->getPositionIncrement();
  restoreState(bufferedState);
  // System.out.println("Buffered posInc: " + posIncAtt.getPositionIncrement() +
  // "   Delegated posInc: " + posInc);
  posIncAtt->setPositionIncrement(posIncAtt->getPositionIncrement() + posInc);
  if (adjustPosition) {
    posIncAtt->setPositionIncrement(posIncAtt->getPositionIncrement() - 1);
    adjustPosition = false;
  }
  bufferedState.reset();
  return true;
}
} // namespace org::apache::lucene::analysis::miscellaneous