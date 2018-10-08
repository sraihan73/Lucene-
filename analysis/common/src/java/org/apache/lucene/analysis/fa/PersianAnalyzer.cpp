using namespace std;

#include "PersianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../ar/ArabicNormalizationFilter.h"
#include "../core/DecimalDigitFilter.h"
#include "PersianCharFilter.h"
#include "PersianNormalizationFilter.h"

namespace org::apache::lucene::analysis::fa
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using ArabicNormalizationFilter =
    org::apache::lucene::analysis::ar::ArabicNormalizationFilter;
using DecimalDigitFilter =
    org::apache::lucene::analysis::core::DecimalDigitFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring PersianAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring PersianAnalyzer::STOPWORDS_COMMENT = L"#";

shared_ptr<CharArraySet> PersianAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    PersianAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

PersianAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, PersianAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
        STOPWORDS_COMMENT);
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    PersianAnalyzer::DefaultSetHolder::staticConstructor;

PersianAnalyzer::PersianAnalyzer()
    : PersianAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

PersianAnalyzer::PersianAnalyzer(shared_ptr<CharArraySet> stopwords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
PersianAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<ArabicNormalizationFilter>(result);
  /* additional persian-specific normalization */
  result = make_shared<PersianNormalizationFilter>(result);
  /*
   * the order here is important: the stopword deque is normalized with the
   * above!
   */
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<StopFilter>(result, stopwords));
}

shared_ptr<TokenStream> PersianAnalyzer::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<DecimalDigitFilter>(result);
  result = make_shared<ArabicNormalizationFilter>(result);
  /* additional persian-specific normalization */
  result = make_shared<PersianNormalizationFilter>(result);
  return result;
}

shared_ptr<Reader> PersianAnalyzer::initReader(const wstring &fieldName,
                                               shared_ptr<Reader> reader)
{
  return make_shared<PersianCharFilter>(reader);
}
} // namespace org::apache::lucene::analysis::fa