using namespace std;

#include "StandardFilter.h"
#include "../TokenStream.h"

namespace org::apache::lucene::analysis::standard
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;

StandardFilter::StandardFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool StandardFilter::incrementToken() 
{
  return input->incrementToken(); // TODO: add some niceties for the new grammar
}
} // namespace org::apache::lucene::analysis::standard