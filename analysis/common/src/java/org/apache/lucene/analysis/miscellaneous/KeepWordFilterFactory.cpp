using namespace std;

#include "KeepWordFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "KeepWordFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

KeepWordFilterFactory::KeepWordFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      ignoreCase(getBoolean(args, L"ignoreCase", false)),
      wordFiles(get(args, L"words"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void KeepWordFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (wordFiles != L"") {
    words = getWordSet(loader, wordFiles, ignoreCase);
  }
}

bool KeepWordFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<CharArraySet> KeepWordFilterFactory::getWords() { return words; }

shared_ptr<TokenStream>
KeepWordFilterFactory::create(shared_ptr<TokenStream> input)
{
  // if the set is null, it means it was empty
  if (words == nullptr) {
    return input;
  } else {
    shared_ptr<TokenStream> *const filter =
        make_shared<KeepWordFilter>(input, words);
    return filter;
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous