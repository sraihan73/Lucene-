using namespace std;

#include "TeeSinkTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

namespace org::apache::lucene::analysis::sinks
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AttributeSource = org::apache::lucene::util::AttributeSource;

TeeSinkTokenFilter::TeeSinkTokenFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

shared_ptr<TokenStream> TeeSinkTokenFilter::newSinkTokenStream()
{
  return make_shared<SinkTokenStream>(this->cloneAttributes(), cachedStates);
}

void TeeSinkTokenFilter::consumeAllTokens() 
{
  while (incrementToken()) {
  }
}

bool TeeSinkTokenFilter::incrementToken() 
{
  if (input->incrementToken()) {
    cachedStates->add(captureState());
    return true;
  }

  return false;
}

void TeeSinkTokenFilter::end() 
{
  TokenFilter::end();
  cachedStates->setFinalState(captureState());
}

void TeeSinkTokenFilter::reset() 
{
  cachedStates->reset();
  TokenFilter::reset();
}

TeeSinkTokenFilter::SinkTokenStream::SinkTokenStream(
    shared_ptr<AttributeSource> source, shared_ptr<States> cachedStates)
    : org::apache::lucene::analysis::TokenStream(source),
      cachedStates(cachedStates)
{
}

bool TeeSinkTokenFilter::SinkTokenStream::incrementToken()
{
  if (!it->hasNext()) {
    return false;
  }

  shared_ptr<AttributeSource::State> state = it->next();
  restoreState(state);
  return true;
}

void TeeSinkTokenFilter::SinkTokenStream::end() 
{
  shared_ptr<AttributeSource::State> finalState = cachedStates->getFinalState();
  if (finalState != nullptr) {
    restoreState(finalState);
  }
}

void TeeSinkTokenFilter::SinkTokenStream::reset()
{
  it = cachedStates->getStates();
}

TeeSinkTokenFilter::States::States() {}

void TeeSinkTokenFilter::States::setFinalState(
    shared_ptr<AttributeSource::State> finalState)
{
  this->finalState = finalState;
}

shared_ptr<AttributeSource::State> TeeSinkTokenFilter::States::getFinalState()
{
  return finalState;
}

void TeeSinkTokenFilter::States::add(shared_ptr<AttributeSource::State> state)
{
  states.push_back(state);
}

shared_ptr<Iterator<std::shared_ptr<AttributeSource::State>>>
TeeSinkTokenFilter::States::getStates()
{
  return states.begin();
}

void TeeSinkTokenFilter::States::reset()
{
  finalState.reset();
  states.clear();
}
} // namespace org::apache::lucene::analysis::sinks