using namespace std;

#include "GalicianStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "GalicianStemFilter.h"

namespace org::apache::lucene::analysis::gl
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using GalicianStemFilter =
    org::apache::lucene::analysis::gl::GalicianStemFilter;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

GalicianStemFilterFactory::GalicianStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
GalicianStemFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<GalicianStemFilter>(input);
}
} // namespace org::apache::lucene::analysis::gl