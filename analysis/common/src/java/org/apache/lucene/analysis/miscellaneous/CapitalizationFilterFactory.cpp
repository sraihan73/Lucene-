using namespace std;

#include "CapitalizationFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "CapitalizationFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring CapitalizationFilterFactory::KEEP = L"keep";
const wstring CapitalizationFilterFactory::KEEP_IGNORE_CASE = L"keepIgnoreCase";
const wstring CapitalizationFilterFactory::OK_PREFIX = L"okPrefix";
const wstring CapitalizationFilterFactory::MIN_WORD_LENGTH = L"minWordLength";
const wstring CapitalizationFilterFactory::MAX_WORD_COUNT = L"maxWordCount";
const wstring CapitalizationFilterFactory::MAX_TOKEN_LENGTH = L"maxTokenLength";
const wstring CapitalizationFilterFactory::ONLY_FIRST_WORD = L"onlyFirstWord";
const wstring CapitalizationFilterFactory::FORCE_FIRST_LETTER =
    L"forceFirstLetter";

CapitalizationFilterFactory::CapitalizationFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      minWordLength(getInt(args, MIN_WORD_LENGTH, 0)),
      maxWordCount(getInt(args, MAX_WORD_COUNT,
                          CapitalizationFilter::DEFAULT_MAX_WORD_COUNT)),
      maxTokenLength(getInt(args, MAX_TOKEN_LENGTH,
                            CapitalizationFilter::DEFAULT_MAX_TOKEN_LENGTH)),
      onlyFirstWord(getBoolean(args, ONLY_FIRST_WORD, true)),
      forceFirstLetter(getBoolean(args, FORCE_FIRST_LETTER, true))
{
  bool ignoreCase = getBoolean(args, KEEP_IGNORE_CASE, false);
  shared_ptr<Set<wstring>> k = getSet(args, KEEP);
  if (k != nullptr) {
    keep = make_shared<CharArraySet>(10, ignoreCase);
    keep->addAll(k);
  }

  k = getSet(args, OK_PREFIX);
  if (k != nullptr) {
    okPrefix = deque<>();
    for (auto item : k) {
      okPrefix->add(item.toCharArray());
    }
  }

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<CapitalizationFilter>
CapitalizationFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<CapitalizationFilter>(
      input, onlyFirstWord, keep, forceFirstLetter, okPrefix, minWordLength,
      maxWordCount, maxTokenLength);
}
} // namespace org::apache::lucene::analysis::miscellaneous