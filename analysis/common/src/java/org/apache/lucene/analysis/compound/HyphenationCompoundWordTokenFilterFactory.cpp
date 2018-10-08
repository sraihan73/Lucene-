using namespace std;

#include "HyphenationCompoundWordTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../util/ResourceLoader.h"
#include "HyphenationCompoundWordTokenFilter.h"
#include "hyphenation/HyphenationTree.h"

namespace org::apache::lucene::analysis::compound
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using HyphenationTree =
    org::apache::lucene::analysis::compound::hyphenation::HyphenationTree;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::xml::sax::InputSource;

HyphenationCompoundWordTokenFilterFactory::
    HyphenationCompoundWordTokenFilterFactory(
        unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dictFile(get(args, L"dictionary")), hypFile(require(args, L"hyphenator")),
      encoding(get(args, L"encoding")),
      minWordSize(getInt(args, L"minWordSize",
                         CompoundWordTokenFilterBase::DEFAULT_MIN_WORD_SIZE)),
      minSubwordSize(
          getInt(args, L"minSubwordSize",
                 CompoundWordTokenFilterBase::DEFAULT_MIN_SUBWORD_SIZE)),
      maxSubwordSize(
          getInt(args, L"maxSubwordSize",
                 CompoundWordTokenFilterBase::DEFAULT_MAX_SUBWORD_SIZE)),
      onlyLongestMatch(getBoolean(args, L"onlyLongestMatch", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void HyphenationCompoundWordTokenFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<InputStream> stream = nullptr;
  try {
    if (dictFile != L"") // the dictionary can be empty.
    {
      dictionary = getWordSet(loader, dictFile, false);
    }
    // TODO: Broken, because we cannot resolve real system id
    // ResourceLoader should also supply method like ClassLoader to get resource
    // URL
    stream = loader->openResource(hypFile);
    shared_ptr<InputSource> *const is = make_shared<InputSource>(stream);
    is->setEncoding(encoding); // if it's null let xml parser decide
    is->setSystemId(hypFile);
    hyphenator = HyphenationCompoundWordTokenFilter::getHyphenationTree(is);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({stream});
  }
}

shared_ptr<TokenFilter>
HyphenationCompoundWordTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<HyphenationCompoundWordTokenFilter>(
      input, hyphenator, dictionary, minWordSize, minSubwordSize,
      maxSubwordSize, onlyLongestMatch);
}
} // namespace org::apache::lucene::analysis::compound