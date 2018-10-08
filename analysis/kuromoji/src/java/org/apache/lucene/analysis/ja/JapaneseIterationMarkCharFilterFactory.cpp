using namespace std;

#include "JapaneseIterationMarkCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"
#include "JapaneseIterationMarkCharFilter.h"

namespace org::apache::lucene::analysis::ja
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
using JapaneseIterationMarkCharFilter =
    org::apache::lucene::analysis::ja::JapaneseIterationMarkCharFilter;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
const wstring JapaneseIterationMarkCharFilterFactory::NORMALIZE_KANJI_PARAM =
    L"normalizeKanji";
const wstring JapaneseIterationMarkCharFilterFactory::NORMALIZE_KANA_PARAM =
    L"normalizeKana";

JapaneseIterationMarkCharFilterFactory::JapaneseIterationMarkCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args),
      normalizeKanji(
          getBoolean(args, NORMALIZE_KANJI_PARAM,
                     JapaneseIterationMarkCharFilter::NORMALIZE_KANJI_DEFAULT)),
      normalizeKana(
          getBoolean(args, NORMALIZE_KANA_PARAM,
                     JapaneseIterationMarkCharFilter::NORMALIZE_KANA_DEFAULT))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<CharFilter>
JapaneseIterationMarkCharFilterFactory::create(shared_ptr<Reader> input)
{
  return make_shared<JapaneseIterationMarkCharFilter>(input, normalizeKanji,
                                                      normalizeKana);
}

shared_ptr<AbstractAnalysisFactory>
JapaneseIterationMarkCharFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::ja