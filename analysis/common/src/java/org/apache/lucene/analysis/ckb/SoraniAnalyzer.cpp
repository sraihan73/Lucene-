using namespace std;

#include "SoraniAnalyzer.h"
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
#include "../core/DecimalDigitFilter.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "SoraniNormalizationFilter.h"
#include "SoraniStemFilter.h"

namespace org::apache::lucene::analysis::ckb
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
using DecimalDigitFilter =
    org::apache::lucene::analysis::core::DecimalDigitFilter;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring SoraniAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> SoraniAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    SoraniAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

SoraniAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = WordlistLoader::getWordSet(IOUtils::getDecodingReader(
        SoraniAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
        StandardCharsets::UTF_8));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    SoraniAnalyzer::DefaultSetHolder::staticConstructor;

SoraniAnalyzer::SoraniAnalyzer()
    : SoraniAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

SoraniAnalyzer::SoraniAnalyzer(shared_ptr<CharArraySet> stopwords)
    : SoraniAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

SoraniAnalyzer::SoraniAnalyzer(shared_ptr<CharArraySet> stopwords,
                               shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
SoraniAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<SoraniNormalizationFilter>(result);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<SoraniStemFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> SoraniAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<SoraniNormalizationFilter>(result);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<DecimalDigitFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::ckb