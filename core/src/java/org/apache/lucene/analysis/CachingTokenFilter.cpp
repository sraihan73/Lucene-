using namespace std;

#include "CachingTokenFilter.h"
#include "../util/AttributeSource.h"
#include "TokenStream.h"

namespace org::apache::lucene::analysis
{
using AttributeSource = org::apache::lucene::util::AttributeSource;

CachingTokenFilter::CachingTokenFilter(shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
}

void CachingTokenFilter::reset() 
{
  if (cache.empty()) { // first time
    input->reset();
  } else {
    iterator = cache.begin();
  }
}

bool CachingTokenFilter::incrementToken() 
{
  if (cache.empty()) { // first-time
    // fill cache lazily
    cache = deque<>(64);
    fillCache();
    iterator = cache.begin();
  }

  if (!iterator->hasNext()) {
    // the cache is exhausted, return false
    return false;
  }
  // Since the TokenFilter can be reset, the tokens need to be preserved as
  // immutable.
  restoreState(iterator->next());
  return true;
}

void CachingTokenFilter::end()
{
  if (finalState != nullptr) {
    restoreState(finalState);
  }
}

void CachingTokenFilter::fillCache() 
{
  while (input->incrementToken()) {
    cache.push_back(captureState());
  }
  // capture final state
  input->end();
  finalState = captureState();
}

bool CachingTokenFilter::isCached() { return cache.size() > 0; }
} // namespace org::apache::lucene::analysis