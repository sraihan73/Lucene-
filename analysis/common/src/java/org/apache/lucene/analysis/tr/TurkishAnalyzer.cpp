using namespace std;

#include "TurkishAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../tartarus/snowball/ext/TurkishStemmer.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "../snowball/SnowballFilter.h"
#include "ApostropheFilter.h"
#include "TurkishLowerCaseFilter.h"

namespace org::apache::lucene::analysis::tr
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
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
using TurkishStemmer = org::tartarus::snowball::ext::TurkishStemmer;
const wstring TurkishAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring TurkishAnalyzer::STOPWORDS_COMMENT = L"#";

shared_ptr<CharArraySet> TurkishAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    TurkishAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

TurkishAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, TurkishAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
        STOPWORDS_COMMENT);
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    TurkishAnalyzer::DefaultSetHolder::staticConstructor;

TurkishAnalyzer::TurkishAnalyzer()
    : TurkishAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

TurkishAnalyzer::TurkishAnalyzer(shared_ptr<CharArraySet> stopwords)
    : TurkishAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

TurkishAnalyzer::TurkishAnalyzer(shared_ptr<CharArraySet> stopwords,
                                 shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
TurkishAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<ApostropheFilter>(result);
  result = make_shared<TurkishLowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<SnowballFilter>(result, make_shared<TurkishStemmer>());
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> TurkishAnalyzer::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<TurkishLowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::tr