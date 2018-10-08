using namespace std;

#include "BrazilianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "BrazilianStemFilter.h"

namespace org::apache::lucene::analysis::br
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring BrazilianAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> BrazilianAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    BrazilianAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

BrazilianAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = WordlistLoader::getWordSet(
        IOUtils::getDecodingReader(BrazilianAnalyzer::typeid,
                                   DEFAULT_STOPWORD_FILE,
                                   StandardCharsets::UTF_8),
        L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    BrazilianAnalyzer::DefaultSetHolder::staticConstructor;

BrazilianAnalyzer::BrazilianAnalyzer()
    : BrazilianAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

BrazilianAnalyzer::BrazilianAnalyzer(shared_ptr<CharArraySet> stopwords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords)
{
}

BrazilianAnalyzer::BrazilianAnalyzer(shared_ptr<CharArraySet> stopwords,
                                     shared_ptr<CharArraySet> stemExclusionSet)
    : BrazilianAnalyzer(stopwords)
{
  excltable =
      CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet));
}

shared_ptr<Analyzer::TokenStreamComponents>
BrazilianAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<StandardFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (excltable != nullptr && !excltable->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, excltable);
  }
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<BrazilianStemFilter>(result));
}

shared_ptr<TokenStream>
BrazilianAnalyzer::normalize(const wstring &fieldName,
                             shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::br