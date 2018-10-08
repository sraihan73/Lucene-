using namespace std;

#include "GalicianMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GalicianMinimalStemFilter.h"

namespace org::apache::lucene::analysis::gl
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GalicianMinimalStemFilter =
    org::apache::lucene::analysis::gl::GalicianMinimalStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GalicianMinimalStemFilterFactory::GalicianMinimalStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GalicianMinimalStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GalicianMinimalStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::gl