using namespace std;

#include "PolishAnalyzer.h"
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
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../../../../egothor/stemmer/Trie.h"
#include "../stempel/StempelFilter.h"
#include "../stempel/StempelStemmer.h"

namespace org::apache::lucene::analysis::pl
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
using StempelFilter = org::apache::lucene::analysis::stempel::StempelFilter;
using StempelStemmer = org::apache::lucene::analysis::stempel::StempelStemmer;
using IOUtils = org::apache::lucene::util::IOUtils;
using Trie = org::egothor::stemmer::Trie;
const wstring PolishAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring PolishAnalyzer::DEFAULT_STEMMER_FILE = L"stemmer_20000.tbl";

shared_ptr<CharArraySet> PolishAnalyzer::getDefaultStopSet()
{
  return DefaultsHolder::DEFAULT_STOP_SET;
}

shared_ptr<Trie> PolishAnalyzer::getDefaultTable()
{
  return DefaultsHolder::DEFAULT_TABLE;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    PolishAnalyzer::DefaultsHolder::DEFAULT_STOP_SET;
const shared_ptr<org::egothor::stemmer::Trie>
    PolishAnalyzer::DefaultsHolder::DEFAULT_TABLE;

PolishAnalyzer::DefaultsHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = WordlistLoader::getWordSet(
        IOUtils::getDecodingReader(PolishAnalyzer::typeid,
                                   DEFAULT_STOPWORD_FILE,
                                   StandardCharsets::UTF_8),
        L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Unable to load default
    // stopword set", ex);
    throw runtime_error(L"Unable to load default stopword set");
  }

  try {
    DEFAULT_TABLE = StempelStemmer::load(
        PolishAnalyzer::typeid->getResourceAsStream(DEFAULT_STEMMER_FILE));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Unable to load default
    // stemming tables", ex);
    throw runtime_error(L"Unable to load default stemming tables");
  }
}

DefaultsHolder::StaticConstructor
    PolishAnalyzer::DefaultsHolder::staticConstructor;

PolishAnalyzer::PolishAnalyzer()
    : PolishAnalyzer(DefaultsHolder::DEFAULT_STOP_SET)
{
}

PolishAnalyzer::PolishAnalyzer(shared_ptr<CharArraySet> stopwords)
    : PolishAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

PolishAnalyzer::PolishAnalyzer(shared_ptr<CharArraySet> stopwords,
                               shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet))),
      stemTable(DefaultsHolder::DEFAULT_TABLE)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
PolishAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  result = make_shared<StempelFilter>(result,
                                      make_shared<StempelStemmer>(stemTable));
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> PolishAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::pl