using namespace std;

#include "KeywordMarkerFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "PatternKeywordMarkerFilter.h"
#include "SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring KeywordMarkerFilterFactory::PROTECTED_TOKENS = L"protected";
const wstring KeywordMarkerFilterFactory::PATTERN = L"pattern";

KeywordMarkerFilterFactory::KeywordMarkerFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      wordFiles(get(args, PROTECTED_TOKENS)), stringPattern(get(args, PATTERN)),
      ignoreCase(getBoolean(args, L"ignoreCase", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void KeywordMarkerFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  if (wordFiles != L"") {
    protectedWords = getWordSet(loader, wordFiles, ignoreCase);
  }
  if (stringPattern != L"") {
    pattern = ignoreCase
                  ? Pattern::compile(stringPattern, Pattern::CASE_INSENSITIVE |
                                                        Pattern::UNICODE_CASE)
                  : Pattern::compile(stringPattern);
  }
}

bool KeywordMarkerFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<TokenStream>
KeywordMarkerFilterFactory::create(shared_ptr<TokenStream> input)
{
  if (pattern != nullptr) {
    input = make_shared<PatternKeywordMarkerFilter>(input, pattern);
  }
  if (protectedWords != nullptr) {
    input = make_shared<SetKeywordMarkerFilter>(input, protectedWords);
  }
  return input;
}
} // namespace org::apache::lucene::analysis::miscellaneous