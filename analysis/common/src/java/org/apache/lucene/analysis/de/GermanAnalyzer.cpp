using namespace std;

#include "GermanAnalyzer.h"
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
#include "GermanLightStemFilter.h"
#include "GermanNormalizationFilter.h"

namespace org::apache::lucene::analysis::de
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
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring GermanAnalyzer::DEFAULT_STOPWORD_FILE = L"german_stop.txt";

shared_ptr<CharArraySet> GermanAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GermanAnalyzer::DefaultSetHolder::DEFAULT_SET;

GermanAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_SET = WordlistLoader::getSnowballWordSet(IOUtils::getDecodingReader(
        SnowballFilter::typeid, DEFAULT_STOPWORD_FILE,
        StandardCharsets::UTF_8));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    GermanAnalyzer::DefaultSetHolder::staticConstructor;

GermanAnalyzer::GermanAnalyzer() : GermanAnalyzer(DefaultSetHolder::DEFAULT_SET)
{
}

GermanAnalyzer::GermanAnalyzer(shared_ptr<CharArraySet> stopwords)
    : GermanAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

GermanAnalyzer::GermanAnalyzer(shared_ptr<CharArraySet> stopwords,
                               shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      exclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
GermanAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  result = make_shared<SetKeywordMarkerFilter>(result, exclusionSet);
  result = make_shared<GermanNormalizationFilter>(result);
  result = make_shared<GermanLightStemFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> GermanAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<GermanNormalizationFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::de