using namespace std;

#include "ConditionalTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "ConditionalTokenFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

ConditionalTokenFilterFactory::ConditionalTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
}

void ConditionalTokenFilterFactory::setInnerFilters(
    deque<std::shared_ptr<TokenFilterFactory>> &innerFilters)
{
  this->innerFilters = innerFilters;
}

shared_ptr<TokenStream>
ConditionalTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  if (innerFilters.empty() || innerFilters.empty()) {
    return input;
  }
  function<TokenStream *(TokenStream *)> innerStream =
      [&](org.apache.lucene.analysis.TokenStream *ts) {
        for (auto factory : innerFilters) {
          ts = factory->create(ts);
        }
        return ts;
      };
  return create(input, innerStream);
}

void ConditionalTokenFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  if (innerFilters.empty()) {
    return;
  }
  for (auto factory : innerFilters) {
    if (std::dynamic_pointer_cast<ResourceLoaderAware>(factory) != nullptr) {
      (std::static_pointer_cast<ResourceLoaderAware>(factory))->inform(loader);
    }
  }
  doInform(loader);
}

void ConditionalTokenFilterFactory::doInform(
    shared_ptr<ResourceLoader> loader) 
{
}
} // namespace org::apache::lucene::analysis::miscellaneous