using namespace std;

#include "StopAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "LowerCaseTokenizer.h"

namespace org::apache::lucene::analysis::core
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    StopAnalyzer::ENGLISH_STOP_WORDS_SET = org::apache::lucene::analysis::
        standard::StandardAnalyzer::ENGLISH_STOP_WORDS_SET;

StopAnalyzer::StopAnalyzer() : StopAnalyzer(ENGLISH_STOP_WORDS_SET) {}

StopAnalyzer::StopAnalyzer(shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopWords)
{
}

StopAnalyzer::StopAnalyzer(shared_ptr<Path> stopwordsFile) 
    : StopAnalyzer(loadStopwordSet(stopwordsFile))
{
}

StopAnalyzer::StopAnalyzer(shared_ptr<Reader> stopwords) 
    : StopAnalyzer(loadStopwordSet(stopwords))
{
}

shared_ptr<TokenStreamComponents>
StopAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<LowerCaseTokenizer>();
  return make_shared<TokenStreamComponents>(
      source, make_shared<StopFilter>(source, stopwords));
}

shared_ptr<TokenStream> StopAnalyzer::normalize(const wstring &fieldName,
                                                shared_ptr<TokenStream> in_)
{
  return make_shared<LowerCaseFilter>(in_);
}
} // namespace org::apache::lucene::analysis::core