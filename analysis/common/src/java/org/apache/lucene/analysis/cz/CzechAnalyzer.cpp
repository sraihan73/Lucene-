using namespace std;

#include "CzechAnalyzer.h"
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
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "CzechStemFilter.h"

namespace org::apache::lucene::analysis::cz
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
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring CzechAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> CzechAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    CzechAnalyzer::DefaultSetHolder::DEFAULT_SET;

CzechAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_SET = WordlistLoader::getWordSet(
        IOUtils::getDecodingReader(CzechAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
                                   StandardCharsets::UTF_8),
        L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    CzechAnalyzer::DefaultSetHolder::staticConstructor;

CzechAnalyzer::CzechAnalyzer() : CzechAnalyzer(DefaultSetHolder::DEFAULT_SET) {}

CzechAnalyzer::CzechAnalyzer(shared_ptr<CharArraySet> stopwords)
    : CzechAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

CzechAnalyzer::CzechAnalyzer(shared_ptr<CharArraySet> stopwords,
                             shared_ptr<CharArraySet> stemExclusionTable)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionTable(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionTable)))
{
}

shared_ptr<Analyzer::TokenStreamComponents>
CzechAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!this->stemExclusionTable->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionTable);
  }
  result = make_shared<CzechStemFilter>(result);
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> CzechAnalyzer::normalize(const wstring &fieldName,
                                                 shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::cz