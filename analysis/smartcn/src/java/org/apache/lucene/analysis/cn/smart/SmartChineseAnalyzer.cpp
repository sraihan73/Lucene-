using namespace std;

#include "SmartChineseAnalyzer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/en/PorterStemFilter.h"
#include "HMMChineseTokenizer.h"

namespace org::apache::lucene::analysis::cn::smart
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using PorterStemFilter = org::apache::lucene::analysis::en::PorterStemFilter;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring SmartChineseAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";
const wstring SmartChineseAnalyzer::STOPWORD_FILE_COMMENT = L"//";

shared_ptr<CharArraySet> SmartChineseAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    SmartChineseAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

SmartChineseAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = loadDefaultStopWordSet();
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    SmartChineseAnalyzer::DefaultSetHolder::staticConstructor;

shared_ptr<CharArraySet>
SmartChineseAnalyzer::DefaultSetHolder::loadDefaultStopWordSet() throw(
    IOException)
{
  // make sure it is unmodifiable as we expose it in the outer class
  return CharArraySet::unmodifiableSet(WordlistLoader::getWordSet(
      IOUtils::getDecodingReader(SmartChineseAnalyzer::typeid,
                                 DEFAULT_STOPWORD_FILE,
                                 StandardCharsets::UTF_8),
      STOPWORD_FILE_COMMENT));
}

SmartChineseAnalyzer::SmartChineseAnalyzer() : SmartChineseAnalyzer(true) {}

SmartChineseAnalyzer::SmartChineseAnalyzer(bool useDefaultStopWords)
    : stopWords(useDefaultStopWords ? DefaultSetHolder::DEFAULT_STOP_SET
                                    : CharArraySet::EMPTY_SET)
{
}

SmartChineseAnalyzer::SmartChineseAnalyzer(shared_ptr<CharArraySet> stopWords)
    : stopWords(stopWords == nullptr ? CharArraySet::EMPTY_SET : stopWords)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
SmartChineseAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const tokenizer = make_shared<HMMChineseTokenizer>();
  shared_ptr<TokenStream> result = tokenizer;
  // result = new LowerCaseFilter(result);
  // LowerCaseFilter is not needed, as SegTokenFilter lowercases Basic Latin
  // text. The porter stemming is too strict, this is not a bug, this is a
  // feature:)
  result = make_shared<PorterStemFilter>(result);
  if (!stopWords->isEmpty()) {
    result = make_shared<StopFilter>(result, stopWords);
  }
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, result);
}

shared_ptr<TokenStream>
SmartChineseAnalyzer::normalize(const wstring &fieldName,
                                shared_ptr<TokenStream> in_)
{
  return make_shared<LowerCaseFilter>(in_);
}
} // namespace org::apache::lucene::analysis::cn::smart