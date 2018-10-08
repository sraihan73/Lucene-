using namespace std;

#include "BengaliAnalyzer.h"
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
#include "BengaliNormalizationFilter.h"
#include "BengaliStemFilter.h"

namespace org::apache::lucene::analysis::bn
{
using namespace org::apache::lucene::analysis;
using DecimalDigitFilter =
    org::apache::lucene::analysis::core::DecimalDigitFilter;
using IndicNormalizationFilter =
    org::apache::lucene::analysis::in_::IndicNormalizationFilter;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring BengaliAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring BengaliAnalyzer::STOPWORDS_COMMENT = L"#";

shared_ptr<CharArraySet> BengaliAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<CharArraySet>
    BengaliAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

BengaliAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET =
        loadStopwordSet(false, BengaliAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
                        STOPWORDS_COMMENT);
  } catch (const IOException &ex) {
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    BengaliAnalyzer::DefaultSetHolder::staticConstructor;

BengaliAnalyzer::BengaliAnalyzer(shared_ptr<CharArraySet> stopwords,
                                 shared_ptr<CharArraySet> stemExclusionSet)
    : StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

BengaliAnalyzer::BengaliAnalyzer(shared_ptr<CharArraySet> stopwords)
    : BengaliAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

BengaliAnalyzer::BengaliAnalyzer()
    : BengaliAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

shared_ptr<TokenStreamComponents>
BengaliAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<DecimalDigitFilter>(result);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<IndicNormalizationFilter>(result);
  result = make_shared<BengaliNormalizationFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  result = make_shared<BengaliStemFilter>(result);
  return make_shared<TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> BengaliAnalyzer::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<IndicNormalizationFilter>(result);
  result = make_shared<BengaliNormalizationFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::bn