using namespace std;

#include "LithuanianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../tartarus/snowball/ext/LithuanianStemmer.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "../snowball/SnowballFilter.h"

namespace org::apache::lucene::analysis::lt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using LithuanianStemmer = org::tartarus::snowball::ext::LithuanianStemmer;
const wstring LithuanianAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> LithuanianAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    LithuanianAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

LithuanianAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, LithuanianAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    LithuanianAnalyzer::DefaultSetHolder::staticConstructor;

LithuanianAnalyzer::LithuanianAnalyzer()
    : LithuanianAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

LithuanianAnalyzer::LithuanianAnalyzer(shared_ptr<CharArraySet> stopwords)
    : LithuanianAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

LithuanianAnalyzer::LithuanianAnalyzer(
    shared_ptr<CharArraySet> stopwords,
    shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
LithuanianAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result =
      make_shared<SnowballFilter>(result, make_shared<LithuanianStemmer>());
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream>
LithuanianAnalyzer::normalize(const wstring &fieldName,
                              shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::lt