using namespace std;

#include "UAX29URLEmailAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "UAX29URLEmailTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    UAX29URLEmailAnalyzer::STOP_WORDS_SET = org::apache::lucene::analysis::
        core::StopAnalyzer::ENGLISH_STOP_WORDS_SET;

UAX29URLEmailAnalyzer::UAX29URLEmailAnalyzer(shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopWords)
{
}

UAX29URLEmailAnalyzer::UAX29URLEmailAnalyzer()
    : UAX29URLEmailAnalyzer(STOP_WORDS_SET)
{
}

UAX29URLEmailAnalyzer::UAX29URLEmailAnalyzer(
    shared_ptr<Reader> stopwords) 
    : UAX29URLEmailAnalyzer(loadStopwordSet(stopwords))
{
}

void UAX29URLEmailAnalyzer::setMaxTokenLength(int length)
{
  maxTokenLength = length;
}

int UAX29URLEmailAnalyzer::getMaxTokenLength() { return maxTokenLength; }

shared_ptr<TokenStreamComponents>
UAX29URLEmailAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<UAX29URLEmailTokenizer> *const src =
      make_shared<UAX29URLEmailTokenizer>();
  src->setMaxTokenLength(maxTokenLength);
  shared_ptr<TokenStream> tok = make_shared<StandardFilter>(src);
  tok = make_shared<LowerCaseFilter>(tok);
  tok = make_shared<StopFilter>(tok, stopwords);
  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tok, src);
}

UAX29URLEmailAnalyzer::TokenStreamComponentsAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass(
        shared_ptr<UAX29URLEmailAnalyzer> outerInstance,
        shared_ptr<TokenStream> tok,
        shared_ptr<
            org::apache::lucene::analysis::standard::UAX29URLEmailTokenizer>
            src)
    : TokenStreamComponents(src, tok)
{
  this->outerInstance = outerInstance;
  this->src = src;
}

void UAX29URLEmailAnalyzer::TokenStreamComponentsAnonymousInnerClass::setReader(
    shared_ptr<Reader> reader)
{
  // So that if maxTokenLength was changed, the change takes
  // effect next time tokenStream is called:
  src->setMaxTokenLength(outerInstance->maxTokenLength);
  outerInstance->super->setReader(reader);
}

shared_ptr<TokenStream>
UAX29URLEmailAnalyzer::normalize(const wstring &fieldName,
                                 shared_ptr<TokenStream> in_)
{
  return make_shared<LowerCaseFilter>(in_);
}
} // namespace org::apache::lucene::analysis::standard