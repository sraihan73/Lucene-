using namespace std;

#include "ClassicAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "ClassicFilter.h"
#include "ClassicTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    ClassicAnalyzer::STOP_WORDS_SET = org::apache::lucene::analysis::core::
        StopAnalyzer::ENGLISH_STOP_WORDS_SET;

ClassicAnalyzer::ClassicAnalyzer(shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopWords)
{
}

ClassicAnalyzer::ClassicAnalyzer() : ClassicAnalyzer(STOP_WORDS_SET) {}

ClassicAnalyzer::ClassicAnalyzer(shared_ptr<Reader> stopwords) throw(
    IOException)
    : ClassicAnalyzer(loadStopwordSet(stopwords))
{
}

void ClassicAnalyzer::setMaxTokenLength(int length) { maxTokenLength = length; }

int ClassicAnalyzer::getMaxTokenLength() { return maxTokenLength; }

shared_ptr<TokenStreamComponents>
ClassicAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<ClassicTokenizer> *const src = make_shared<ClassicTokenizer>();
  src->setMaxTokenLength(maxTokenLength);
  shared_ptr<TokenStream> tok = make_shared<ClassicFilter>(src);
  tok = make_shared<LowerCaseFilter>(tok);
  tok = make_shared<StopFilter>(tok, stopwords);
  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tok, src);
}

ClassicAnalyzer::TokenStreamComponentsAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass(
        shared_ptr<ClassicAnalyzer> outerInstance, shared_ptr<TokenStream> tok,
        shared_ptr<org::apache::lucene::analysis::standard::ClassicTokenizer>
            src)
    : TokenStreamComponents(src, tok)
{
  this->outerInstance = outerInstance;
  this->src = src;
}

void ClassicAnalyzer::TokenStreamComponentsAnonymousInnerClass::setReader(
    shared_ptr<Reader> reader)
{
  src->setMaxTokenLength(outerInstance->maxTokenLength);
  outerInstance->super->setReader(reader);
}

shared_ptr<TokenStream> ClassicAnalyzer::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  return make_shared<LowerCaseFilter>(in_);
}
} // namespace org::apache::lucene::analysis::standard