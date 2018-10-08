using namespace std;

#include "MorfologikFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "MorfologikFilter.h"

namespace org::apache::lucene::analysis::morfologik
{
using morfologik::stemming::Dictionary;
using morfologik::stemming::DictionaryMetadata;
using morfologik::stemming::polish::PolishStemmer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring MorfologikFilterFactory::DICTIONARY_ATTRIBUTE = L"dictionary";

MorfologikFilterFactory::MorfologikFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{

  // Be specific about no-longer-supported dictionary attribute.
  const wstring DICTIONARY_RESOURCE_ATTRIBUTE = L"dictionary-resource";
  wstring dictionaryResource = get(args, DICTIONARY_RESOURCE_ATTRIBUTE);
  if (dictionaryResource != L"" && !dictionaryResource.isEmpty()) {
    throw invalid_argument(
        L"The " + DICTIONARY_RESOURCE_ATTRIBUTE + L" attribute is no " +
        L"longer supported. Use the '" + DICTIONARY_ATTRIBUTE +
        L"' attribute instead (see LUCENE-6833).");
  }

  resourceName = get(args, DICTIONARY_ATTRIBUTE);

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void MorfologikFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (resourceName == L"") {
    // Get the dictionary lazily, does not hold up memory.
    this->dictionary = (make_shared<PolishStemmer>())->getDictionary();
  } else {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream dict =
    // loader.openResource(resourceName); java.io.InputStream meta =
    // loader.openResource(morfologik.stemming.DictionaryMetadata.getExpectedMetadataFileName(resourceName)))
    {
      java::io::InputStream dict = loader->openResource(resourceName);
      java::io::InputStream meta = loader->openResource(
          morfologik::stemming::DictionaryMetadata::getExpectedMetadataFileName(
              resourceName));
      this->dictionary = Dictionary::read(dict, meta);
    }
  }
}

shared_ptr<TokenStream>
MorfologikFilterFactory::create(shared_ptr<TokenStream> ts)
{
  return make_shared<MorfologikFilter>(
      ts,
      Objects::requireNonNull(
          dictionary, L"MorfologikFilterFactory was not fully initialized."));
}
} // namespace org::apache::lucene::analysis::morfologik