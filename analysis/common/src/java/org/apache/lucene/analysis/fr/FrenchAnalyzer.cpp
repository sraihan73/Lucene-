using namespace std;

#include "FrenchAnalyzer.h"
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
#include "../snowball/SnowballFilter.h"
#include "../util/ElisionFilter.h"
#include "FrenchLightStemFilter.h"

namespace org::apache::lucene::analysis::fr
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
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using ElisionFilter = org::apache::lucene::analysis::util::ElisionFilter;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring FrenchAnalyzer::DEFAULT_STOPWORD_FILE = L"french_stop.txt";
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    FrenchAnalyzer::DEFAULT_ARTICLES =
        org::apache::lucene::analysis::CharArraySet::unmodifiableSet(
            make_shared<org::apache::lucene::analysis::CharArraySet>(
                java::util::Arrays::asList(L"l", L"m", L"t", L"qu", L"n", L"s",
                                           L"j", L"d", L"c", L"jusqu",
                                           L"quoiqu", L"lorsqu", L"puisqu"),
                true));

shared_ptr<CharArraySet> FrenchAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    FrenchAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

FrenchAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET =
        WordlistLoader::getSnowballWordSet(IOUtils::getDecodingReader(
            SnowballFilter::typeid, DEFAULT_STOPWORD_FILE,
            StandardCharsets::UTF_8));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    FrenchAnalyzer::DefaultSetHolder::staticConstructor;

FrenchAnalyzer::FrenchAnalyzer()
    : FrenchAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

FrenchAnalyzer::FrenchAnalyzer(shared_ptr<CharArraySet> stopwords)
    : FrenchAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

FrenchAnalyzer::FrenchAnalyzer(shared_ptr<CharArraySet> stopwords,
                               shared_ptr<CharArraySet> stemExclutionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      excltable(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclutionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
FrenchAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<ElisionFilter>(result, DEFAULT_ARTICLES);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!excltable->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, excltable);
  }
  result = make_shared<FrenchLightStemFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> FrenchAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<ElisionFilter>(result, DEFAULT_ARTICLES);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::fr