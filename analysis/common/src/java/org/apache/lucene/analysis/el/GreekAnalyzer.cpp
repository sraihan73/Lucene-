using namespace std;

#include "GreekAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "GreekLowerCaseFilter.h"
#include "GreekStemFilter.h"

namespace org::apache::lucene::analysis::el
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring GreekAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> GreekAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekAnalyzer::DefaultSetHolder::DEFAULT_SET;

GreekAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, GreekAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    GreekAnalyzer::DefaultSetHolder::staticConstructor;

GreekAnalyzer::GreekAnalyzer() : GreekAnalyzer(DefaultSetHolder::DEFAULT_SET) {}

GreekAnalyzer::GreekAnalyzer(shared_ptr<CharArraySet> stopwords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
GreekAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<GreekLowerCaseFilter>(source);
  result = make_shared<StandardFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  result = make_shared<GreekStemFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> GreekAnalyzer::normalize(const wstring &fieldName,
                                                 shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<GreekLowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::el