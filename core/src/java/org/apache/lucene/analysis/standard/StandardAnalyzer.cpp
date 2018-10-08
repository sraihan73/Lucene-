using namespace std;

#include "StandardAnalyzer.h"
#include "../CharArraySet.h"
#include "../LowerCaseFilter.h"
#include "../StopFilter.h"
#include "../TokenStream.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"

namespace org::apache::lucene::analysis::standard
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    StandardAnalyzer::ENGLISH_STOP_WORDS_SET;

StandardAnalyzer::StaticConstructor::StaticConstructor()
{
  const deque<wstring> stopWords = Arrays::asList(
      L"a", L"an", L"and", L"are", L"as", L"at", L"be", L"but", L"by", L"for",
      L"if", L"in", L"into", L"is", L"it", L"no", L"not", L"of", L"on", L"or",
      L"such", L"that", L"the", L"their", L"then", L"there", L"these", L"they",
      L"this", L"to", L"was", L"will", L"with");
  shared_ptr<CharArraySet> *const stopSet =
      make_shared<CharArraySet>(stopWords, false);
  ENGLISH_STOP_WORDS_SET = CharArraySet::unmodifiableSet(stopSet);
}

StandardAnalyzer::StaticConstructor StandardAnalyzer::staticConstructor;
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    StandardAnalyzer::STOP_WORDS_SET = ENGLISH_STOP_WORDS_SET;

StandardAnalyzer::StandardAnalyzer(shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopWords)
{
}

StandardAnalyzer::StandardAnalyzer() : StandardAnalyzer(STOP_WORDS_SET) {}

StandardAnalyzer::StandardAnalyzer(shared_ptr<Reader> stopwords) throw(
    IOException)
    : StandardAnalyzer(loadStopwordSet(stopwords))
{
}

void StandardAnalyzer::setMaxTokenLength(int length)
{
  maxTokenLength = length;
}

int StandardAnalyzer::getMaxTokenLength() { return maxTokenLength; }

shared_ptr<TokenStreamComponents>
StandardAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<StandardTokenizer> *const src = make_shared<StandardTokenizer>();
  src->setMaxTokenLength(maxTokenLength);
  shared_ptr<TokenStream> tok = make_shared<StandardFilter>(src);
  tok = make_shared<LowerCaseFilter>(tok);
  tok = make_shared<StopFilter>(tok, stopwords);
  return make_shared<TokenStreamComponentsAnonymousInnerClass>(
      shared_from_this(), tok, src);
}

StandardAnalyzer::TokenStreamComponentsAnonymousInnerClass::
    TokenStreamComponentsAnonymousInnerClass(
        shared_ptr<StandardAnalyzer> outerInstance, shared_ptr<TokenStream> tok,
        shared_ptr<org::apache::lucene::analysis::standard::StandardTokenizer>
            src)
    : TokenStreamComponents(src, tok)
{
  this->outerInstance = outerInstance;
  this->src = src;
}

void StandardAnalyzer::TokenStreamComponentsAnonymousInnerClass::setReader(
    shared_ptr<Reader> reader)
{
  // So that if maxTokenLength was changed, the change takes
  // effect next time tokenStream is called:
  src->setMaxTokenLength(outerInstance->maxTokenLength);
  outerInstance->super->setReader(reader);
}

shared_ptr<TokenStream> StandardAnalyzer::normalize(const wstring &fieldName,
                                                    shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::standard