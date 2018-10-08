using namespace std;

#include "HindiAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../core/DecimalDigitFilter.h"
#include "../in/IndicNormalizationFilter.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "HindiNormalizationFilter.h"
#include "HindiStemFilter.h"

namespace org::apache::lucene::analysis::hi
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DecimalDigitFilter =
    org::apache::lucene::analysis::core::DecimalDigitFilter;
using IndicNormalizationFilter =
    org::apache::lucene::analysis::in_::IndicNormalizationFilter;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring HindiAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring HindiAnalyzer::STOPWORDS_COMMENT = L"#";

shared_ptr<CharArraySet> HindiAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    HindiAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

HindiAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, HindiAnalyzer::typeid, DEFAULT_STOPWORD_FILE, STOPWORDS_COMMENT);
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    HindiAnalyzer::DefaultSetHolder::staticConstructor;

HindiAnalyzer::HindiAnalyzer(shared_ptr<CharArraySet> stopwords,
                             shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

HindiAnalyzer::HindiAnalyzer(shared_ptr<CharArraySet> stopwords)
    : HindiAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

HindiAnalyzer::HindiAnalyzer()
    : HindiAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

shared_ptr<TokenStreamComponents>
HindiAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<DecimalDigitFilter>(result);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<IndicNormalizationFilter>(result);
  result = make_shared<HindiNormalizationFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  result = make_shared<HindiStemFilter>(result);
  return make_shared<TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> HindiAnalyzer::normalize(const wstring &fieldName,
                                                 shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<IndicNormalizationFilter>(result);
  result = make_shared<HindiNormalizationFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::hi