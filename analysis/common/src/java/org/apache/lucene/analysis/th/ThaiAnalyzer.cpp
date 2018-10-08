using namespace std;

#include "ThaiAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../core/DecimalDigitFilter.h"
#include "ThaiTokenizer.h"

namespace org::apache::lucene::analysis::th
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
const wstring ThaiAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring ThaiAnalyzer::STOPWORDS_COMMENT = L"#";

shared_ptr<CharArraySet> ThaiAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    ThaiAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

ThaiAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, ThaiAnalyzer::typeid, DEFAULT_STOPWORD_FILE, STOPWORDS_COMMENT);
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    ThaiAnalyzer::DefaultSetHolder::staticConstructor;

ThaiAnalyzer::ThaiAnalyzer() : ThaiAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

ThaiAnalyzer::ThaiAnalyzer(shared_ptr<CharArraySet> stopwords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
ThaiAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<ThaiTokenizer>();
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(source);
  result = make_shared<DecimalDigitFilter>(result);

  result = make_shared<StopFilter>(result, stopwords);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> ThaiAnalyzer::normalize(const wstring &fieldName,
                                                shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(in_);
  result = make_shared<DecimalDigitFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::th