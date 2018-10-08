using namespace std;

#include "CatalanAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../tartarus/snowball/ext/CatalanStemmer.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "../snowball/SnowballFilter.h"
#include "../util/ElisionFilter.h"

namespace org::apache::lucene::analysis::ca
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using ElisionFilter = org::apache::lucene::analysis::util::ElisionFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using CatalanStemmer = org::tartarus::snowball::ext::CatalanStemmer;
const wstring CatalanAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    CatalanAnalyzer::DEFAULT_ARTICLES =
        org::apache::lucene::analysis::CharArraySet::unmodifiableSet(
            make_shared<org::apache::lucene::analysis::CharArraySet>(
                java::util::Arrays::asList(L"d", L"l", L"m", L"n", L"s", L"t"),
                true));

shared_ptr<CharArraySet> CatalanAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    CatalanAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

CatalanAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, CatalanAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    CatalanAnalyzer::DefaultSetHolder::staticConstructor;

CatalanAnalyzer::CatalanAnalyzer()
    : CatalanAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

CatalanAnalyzer::CatalanAnalyzer(shared_ptr<CharArraySet> stopwords)
    : CatalanAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

CatalanAnalyzer::CatalanAnalyzer(shared_ptr<CharArraySet> stopwords,
                                 shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
CatalanAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<ElisionFilter>(result, DEFAULT_ARTICLES);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<SnowballFilter>(result, make_shared<CatalanStemmer>());
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> CatalanAnalyzer::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<ElisionFilter>(result, DEFAULT_ARTICLES);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::ca