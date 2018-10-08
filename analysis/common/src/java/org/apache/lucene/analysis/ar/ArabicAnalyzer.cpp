using namespace std;

#include "ArabicAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../core/DecimalDigitFilter.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "ArabicNormalizationFilter.h"
#include "ArabicStemFilter.h"

namespace org::apache::lucene::analysis::ar
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DecimalDigitFilter =
    org::apache::lucene::analysis::core::DecimalDigitFilter;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring ArabicAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> ArabicAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    ArabicAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

ArabicAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, ArabicAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    ArabicAnalyzer::DefaultSetHolder::staticConstructor;

ArabicAnalyzer::ArabicAnalyzer()
    : ArabicAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

ArabicAnalyzer::ArabicAnalyzer(shared_ptr<CharArraySet> stopwords)
    : ArabicAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

ArabicAnalyzer::ArabicAnalyzer(shared_ptr<CharArraySet> stopwords,
                               shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
ArabicAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<DecimalDigitFilter>(result);
  // the order here is important: the stopword deque is not normalized!
  result = make_shared<StopFilter>(result, stopwords);
  // TODO maybe we should make ArabicNormalization filter also KeywordAttribute
  // aware?!
  result = make_shared<ArabicNormalizationFilter>(result);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<ArabicStemFilter>(result));
}

shared_ptr<TokenStream> ArabicAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(in_);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<ArabicNormalizationFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::ar