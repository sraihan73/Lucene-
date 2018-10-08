using namespace std;

#include "DictionaryCompoundWordTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "DictionaryCompoundWordTokenFilter.h"

namespace org::apache::lucene::analysis::compound
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

DictionaryCompoundWordTokenFilterFactory::
    DictionaryCompoundWordTokenFilterFactory(
        unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dictFile(require(args, L"dictionary")),
      minWordSize(getInt(args, L"minWordSize",
                         CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE)),
      minSubwordSize(
          getInt(args, L"minSubwordSize",
                 CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE)),
      maxSubwordSize(
          getInt(args, L"maxSubwordSize",
                 CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE)),
      onlyLongestMatch(getBoolean(args, L"onlyLongestMatch", true))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void DictionaryCompoundWordTokenFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  dictionary = TokenFilterFactory::getWordSet(loader, dictFile, false);
}

shared_ptr<TokenStream>
DictionaryCompoundWordTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  // if the dictionary is null, it means it was empty
  if (dictionary == nullptr) {
    return input;
  }
  return make_shared<DictionaryCompoundWordTokenFilter>(
      input, dictionary, minWordSize, minSubwordSize, maxSubwordSize,
      onlyLongestMatch);
}
} // namespace org::apache::lucene::analysis::compound