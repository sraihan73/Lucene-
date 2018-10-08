using namespace std;

#include "ElisionFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../fr/FrenchAnalyzer.h"
#include "AbstractAnalysisFactory.h"
#include "ElisionFilter.h"
#include "ResourceLoader.h"

namespace org::apache::lucene::analysis::util
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FrenchAnalyzer = org::apache::lucene::analysis::fr::FrenchAnalyzer;

ElisionFilterFactory::ElisionFilterFactory(
    unordered_map<wstring, wstring> &args)
    : TokenFilterFactory(args), articlesFile(get(args, L"articles")),
      ignoreCase(getBoolean(args, L"ignoreCase", false))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void ElisionFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (articlesFile == L"") {
    articles = FrenchAnalyzer::DEFAULT_ARTICLES;
  } else {
    articles = getWordSet(loader, articlesFile, ignoreCase);
  }
}

shared_ptr<ElisionFilter>
ElisionFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<ElisionFilter>(input, articles);
}

shared_ptr<AbstractAnalysisFactory>
ElisionFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::util