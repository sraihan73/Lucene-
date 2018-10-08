using namespace std;

#include "PersianCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../util/AbstractAnalysisFactory.h"
#include "PersianCharFilter.h"

namespace org::apache::lucene::analysis::fa
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using PersianCharFilter = org::apache::lucene::analysis::fa::PersianCharFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;

PersianCharFilterFactory::PersianCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args)
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<CharFilter>
PersianCharFilterFactory::create(shared_ptr<Reader> input)
{
  return make_shared<PersianCharFilter>(input);
}

shared_ptr<AbstractAnalysisFactory>
PersianCharFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::fa