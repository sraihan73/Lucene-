using namespace std;

#include "ICUTransformFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "ICUTransformFilter.h"

namespace org::apache::lucene::analysis::icu
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using com::ibm::icu::text::Transliterator;

ICUTransformFilterFactory::ICUTransformFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      transliterator(Transliterator::getInstance(id, dir))
{
  wstring id = require(args, L"id");
  wstring direction =
      get(args, L"direction", Arrays::asList(L"forward", L"reverse"),
          L"forward", false);
  int dir = L"forward" == direction ? Transliterator::FORWARD
                                    : Transliterator::REVERSE;
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
ICUTransformFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ICUTransformFilter>(input, transliterator);
}

shared_ptr<AbstractAnalysisFactory>
ICUTransformFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::icu