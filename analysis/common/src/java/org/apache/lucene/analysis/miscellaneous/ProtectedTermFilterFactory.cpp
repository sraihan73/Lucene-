using namespace std;

#include "ProtectedTermFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "../util/TokenFilterFactory.h"
#include "ConditionalTokenFilter.h"
#include "ProtectedTermFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring ProtectedTermFilterFactory::PROTECTED_TERMS = L"protected";

ProtectedTermFilterFactory::ProtectedTermFilterFactory(
    unordered_map<wstring, wstring> &args)
    : ConditionalTokenFilterFactory(args),
      termFiles(require(args, PROTECTED_TERMS)),
      ignoreCase(getBoolean(args, L"ignoreCase", false)),
      wrappedFilters(get(args, L"wrappedFilters"))
{
  if (wrappedFilters != L"") {
    handleWrappedFilterArgs(args);
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void ProtectedTermFilterFactory::handleWrappedFilterArgs(
    unordered_map<wstring, wstring> &args)
{
  shared_ptr<LinkedHashMap<wstring, unordered_map<wstring, wstring>>>
      wrappedFilterArgs = make_shared<
          LinkedHashMap<wstring, unordered_map<wstring, wstring>>>();
  splitAt(L',', wrappedFilters).forEach([&](any filterName) {
    filterName = filterName->trim()->toLowerCase(Locale::ROOT);
    if (wrappedFilterArgs->containsKey(filterName)) {
      throw invalid_argument(
          L"wrappedFilters contains duplicate '" + filterName +
          L"'. Add unique '-id' suffixes (stripped prior to SPI lookup).");
    }
    wrappedFilterArgs->put(filterName, unordered_map<>());
  });
  for (unordered_map<wstring, wstring>::const_iterator iterator = args.begin();
       iterator != args.end(); ++iterator) {
    unordered_map::Entry<wstring, wstring> entry = *iterator;
    wstring filterArgKey = entry.getKey();
    wstring argValue = entry.getValue();
    deque<wstring> splitKey = splitAt(
        FILTER_ARG_SEPARATOR, filterArgKey); // Format: filterName.argKey
    if (splitKey.size() == 2) {              // Skip if no slash
      wstring filterName = splitKey[0].toLowerCase(Locale::ROOT);
      if (wrappedFilterArgs->containsKey(
              filterName)) { // Skip if not in "wrappedFilter" arg
        unordered_map<wstring, wstring> filterArgs =
            wrappedFilterArgs->computeIfAbsent(
                filterName, [&](any k) { unordered_map<>(); });
        wstring argKey = splitKey[1];
        filterArgs.emplace(argKey,
                           argValue); // argKey is guaranteed unique, don't need
                                      // to check for duplicates
        iterator.remove();
      }
    }
  }
  if (args.empty()) {
    populateInnerFilters(wrappedFilterArgs);
  }
}

void ProtectedTermFilterFactory::populateInnerFilters(
    shared_ptr<LinkedHashMap<wstring, unordered_map<wstring, wstring>>>
        wrappedFilterArgs)
{
  deque<std::shared_ptr<TokenFilterFactory>> innerFilters =
      deque<std::shared_ptr<TokenFilterFactory>>();
  wrappedFilterArgs->forEach([&](filterName, filterArgs) {
    int idSuffixPos = filterName->find(FILTER_NAME_ID_SEPARATOR);
    if (idSuffixPos != -1) {
      filterName = filterName->substr(0, idSuffixPos);
    }
    innerFilters.add(TokenFilterFactory::forName(filterName, filterArgs));
  });
  setInnerFilters(innerFilters);
}

bool ProtectedTermFilterFactory::isIgnoreCase() { return ignoreCase; }

shared_ptr<CharArraySet> ProtectedTermFilterFactory::getProtectedTerms()
{
  return protectedTerms;
}

shared_ptr<ConditionalTokenFilter> ProtectedTermFilterFactory::create(
    shared_ptr<TokenStream> input,
    function<TokenStream *(TokenStream *)> &inner)
{
  return make_shared<ProtectedTermFilter>(protectedTerms, input, inner);
}

void ProtectedTermFilterFactory::doInform(
    shared_ptr<ResourceLoader> loader) 
{
  protectedTerms = getWordSet(loader, termFiles, ignoreCase);
}
} // namespace org::apache::lucene::analysis::miscellaneous