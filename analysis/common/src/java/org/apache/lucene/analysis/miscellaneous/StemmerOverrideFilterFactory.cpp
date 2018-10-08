using namespace std;

#include "StemmerOverrideFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "StemmerOverrideFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

StemmerOverrideFilterFactory::StemmerOverrideFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      dictionaryFiles(get(args, L"dictionary")),
      ignoreCase(getBoolean(args, L"ignoreCase", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void StemmerOverrideFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  if (dictionaryFiles != L"") {
    deque<wstring> files = splitFileNames(dictionaryFiles);
    if (files.size() > 0) {
      shared_ptr<StemmerOverrideFilter::Builder> builder =
          make_shared<StemmerOverrideFilter::Builder>(ignoreCase);
      for (auto file : files) {
        deque<wstring> deque = getLines(loader, file.trim());
        for (auto line : deque) {
          std::deque<wstring> mapping = line.split(L"\t", 2);
          builder->add(mapping[0], mapping[1]);
        }
      }
      dictionary = builder->build();
    }
  }
}

bool StemmerOverrideFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<TokenStream>
StemmerOverrideFilterFactory::create(shared_ptr<TokenStream> input)
{
  return dictionary == nullptr
             ? input
             : make_shared<StemmerOverrideFilter>(input, dictionary);
}
} // namespace org::apache::lucene::analysis::miscellaneous