using namespace std;

#include "SnowballPorterFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../tartarus/snowball/SnowballProgram.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "../util/ResourceLoader.h"
#include "SnowballFilter.h"

namespace org::apache::lucene::analysis::snowball
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const wstring SnowballPorterFilterFactory::PROTECTED_TOKENS = L"protected";

SnowballPorterFilterFactory::SnowballPorterFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      language(get(args, L"language", L"English")),
      wordFiles(get(args, PROTECTED_TOKENS))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void SnowballPorterFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  wstring className = L"org.tartarus.snowball.ext." + language + L"Stemmer";
  stemClass =
      loader->newInstance(className, SnowballProgram::typeid)->getClass();

  if (wordFiles != L"") {
    protectedWords = getWordSet(loader, wordFiles, false);
  }
}

shared_ptr<TokenFilter>
SnowballPorterFilterFactory::create(shared_ptr<TokenStream> input)
{
  shared_ptr<SnowballProgram> program;
  try {
    program = stemClass.newInstance();
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Error instantiating stemmer
    // for language " + language + "from class " + stemClass, e);
    throw runtime_error(L"Error instantiating stemmer for language " +
                        language + L"from class " + stemClass);
  }

  if (protectedWords != nullptr) {
    input = make_shared<SetKeywordMarkerFilter>(input, protectedWords);
  }
  return make_shared<SnowballFilter>(input, program);
}
} // namespace org::apache::lucene::analysis::snowball