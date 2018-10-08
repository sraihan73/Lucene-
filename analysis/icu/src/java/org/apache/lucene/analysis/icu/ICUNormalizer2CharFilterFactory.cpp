using namespace std;

#include "ICUNormalizer2CharFilterFactory.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "ICUNormalizer2CharFilter.h"

namespace org::apache::lucene::analysis::icu
{
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using com::ibm::icu::text::FilteredNormalizer2;
using com::ibm::icu::text::Normalizer2;
using com::ibm::icu::text::UnicodeSet;

ICUNormalizer2CharFilterFactory::ICUNormalizer2CharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args)
{
  wstring name = get(args, L"name", L"nfkc_cf");
  wstring mode =
      get(args, L"mode", Arrays::asList(L"compose", L"decompose"), L"compose");
  shared_ptr<Normalizer2> normalizer = Normalizer2::getInstance(
      nullptr, name,
      L"compose" == mode ? Normalizer2::Mode::COMPOSE
                         : Normalizer2::Mode::DECOMPOSE);

  wstring filter = get(args, L"filter");
  if (filter != L"") {
    shared_ptr<UnicodeSet> set = make_shared<UnicodeSet>(filter);
    if (!set->isEmpty()) {
      set->freeze();
      normalizer = make_shared<FilteredNormalizer2>(normalizer, set);
    }
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
  this->normalizer = normalizer;
}

shared_ptr<Reader>
ICUNormalizer2CharFilterFactory::create(shared_ptr<Reader> input)
{
  return make_shared<ICUNormalizer2CharFilter>(input, normalizer);
}

shared_ptr<AbstractAnalysisFactory>
ICUNormalizer2CharFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::icu