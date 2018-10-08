using namespace std;

#include "PatternReplaceCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../util/AbstractAnalysisFactory.h"
#include "PatternReplaceCharFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;

PatternReplaceCharFilterFactory::PatternReplaceCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args),
      pattern(getPattern(args, L"pattern")),
      replacement(get(args, L"replacement", L""))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<CharFilter>
PatternReplaceCharFilterFactory::create(shared_ptr<Reader> input)
{
  return make_shared<PatternReplaceCharFilter>(pattern, replacement, input);
}

shared_ptr<AbstractAnalysisFactory>
PatternReplaceCharFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::pattern