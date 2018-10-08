using namespace std;

#include "HunspellStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../util/ResourceLoader.h"
#include "Dictionary.h"
#include "HunspellStemFilter.h"

namespace org::apache::lucene::analysis::hunspell
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring HunspellStemFilterFactory::PARAM_DICTIONARY = L"dictionary";
const wstring HunspellStemFilterFactory::PARAM_AFFIX = L"affix";
const wstring HunspellStemFilterFactory::PARAM_RECURSION_CAP = L"recursionCap";
const wstring HunspellStemFilterFactory::PARAM_IGNORE_CASE = L"ignoreCase";
const wstring HunspellStemFilterFactory::PARAM_LONGEST_ONLY = L"longestOnly";

HunspellStemFilterFactory::HunspellStemFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dictionaryFiles(require(args, PARAM_DICTIONARY)),
      affixFile(get(args, PARAM_AFFIX)),
      ignoreCase(getBoolean(args, PARAM_IGNORE_CASE, false)),
      longestOnly(getBoolean(args, PARAM_LONGEST_ONLY, false))
{
  // this isnt necessary: we properly load all dictionaries.
  // but recognize and ignore for back compat
  getBoolean(args, L"strictAffixParsing", true);
  // this isn't necessary: multi-stage stripping is fixed and
  // flags like COMPLEXPREFIXES in the data itself control this.
  // but recognize and ignore for back compat
  getInt(args, L"recursionCap", 0);
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void HunspellStemFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  std::deque<wstring> dicts = dictionaryFiles.split(L",");

  shared_ptr<InputStream> affix = nullptr;
  deque<std::shared_ptr<InputStream>> dictionaries =
      deque<std::shared_ptr<InputStream>>();

  try {
    dictionaries = deque<>();
    for (auto file : dicts) {
      dictionaries.push_back(loader->openResource(file));
    }
    affix = loader->openResource(affixFile);

    shared_ptr<Path> tempPath = Files::createTempDirectory(
        Dictionary::getDefaultTempDir(), L"Hunspell");
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory tempDir
    // = org.apache.lucene.store.FSDirectory.open(tempPath))
    {
      org::apache::lucene::store::Directory tempDir =
          org::apache::lucene::store::FSDirectory::open(tempPath);
      try {
        this->dictionary = make_shared<Dictionary>(tempDir, L"hunspell", affix,
                                                   dictionaries, ignoreCase);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        IOUtils::rm({tempPath});
      }
    }
  } catch (const ParseException &e) {
    throw make_shared<IOException>(
        L"Unable to load hunspell data! [dictionary=" + dictionaries +
            L",affix=" + affixFile + L"]",
        e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({affix});
    IOUtils::closeWhileHandlingException(dictionaries);
  }
}

shared_ptr<TokenStream>
HunspellStemFilterFactory::create(shared_ptr<TokenStream> tokenStream)
{
  return make_shared<HunspellStemFilter>(tokenStream, dictionary, true,
                                         longestOnly);
}
} // namespace org::apache::lucene::analysis::hunspell