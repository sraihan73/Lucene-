using namespace std;

#include "CJKAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "CJKBigramFilter.h"
#include "CJKWidthFilter.h"

namespace org::apache::lucene::analysis::cjk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring CJKAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> CJKAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    CJKAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

CJKAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, CJKAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    CJKAnalyzer::DefaultSetHolder::staticConstructor;

CJKAnalyzer::CJKAnalyzer() : CJKAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET) {}

CJKAnalyzer::CJKAnalyzer(shared_ptr<CharArraySet> stopwords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
CJKAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  // run the widthfilter first before bigramming, it sometimes combines
  // characters.
  shared_ptr<TokenStream> result = make_shared<CJKWidthFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<CJKBigramFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<StopFilter>(result, stopwords));
}

shared_ptr<TokenStream> CJKAnalyzer::normalize(const wstring &fieldName,
                                               shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<CJKWidthFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::cjk