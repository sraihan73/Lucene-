using namespace std;

#include "TypeTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "TypeTokenFilter.h"

namespace org::apache::lucene::analysis::core
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

TypeTokenFilterFactory::TypeTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      useWhitelist(getBoolean(args, L"useWhitelist", false)),
      stopTypesFiles(require(args, L"types"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void TypeTokenFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  deque<wstring> files = splitFileNames(stopTypesFiles);
  if (files.size() > 0) {
    stopTypes = unordered_set<>();
    for (auto file : files) {
      deque<wstring> typesLines = getLines(loader, file.trim());
      stopTypes->addAll(typesLines);
    }
  }
}

shared_ptr<Set<wstring>> TypeTokenFilterFactory::getStopTypes()
{
  return stopTypes;
}

shared_ptr<TokenStream>
TypeTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<TokenStream> *const filter =
      make_shared<TypeTokenFilter>(input, stopTypes, useWhitelist);
  return filter;
}
} // namespace org::apache::lucene::analysis::core