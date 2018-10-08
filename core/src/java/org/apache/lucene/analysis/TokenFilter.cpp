using namespace std;

#include "TokenFilter.h"

namespace org::apache::lucene::analysis
{

TokenFilter::TokenFilter(shared_ptr<TokenStream> input)
    : TokenStream(input), input(input)
{
}

void TokenFilter::end()  { input->end(); }

TokenFilter::~TokenFilter() { delete input; }

void TokenFilter::reset()  { input->reset(); }
} // namespace org::apache::lucene::analysis