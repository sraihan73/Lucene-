using namespace std;

#include "QueryAutoStopWordAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

namespace org::apache::lucene::analysis::query
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

QueryAutoStopWordAnalyzer::QueryAutoStopWordAnalyzer(
    shared_ptr<Analyzer> delegate_,
    shared_ptr<IndexReader> indexReader) 
    : QueryAutoStopWordAnalyzer(delegate_, indexReader,
                                defaultMaxDocFreqPercent)
{
}

QueryAutoStopWordAnalyzer::QueryAutoStopWordAnalyzer(
    shared_ptr<Analyzer> delegate_, shared_ptr<IndexReader> indexReader,
    int maxDocFreq) 
    : QueryAutoStopWordAnalyzer(delegate_, indexReader,
                                MultiFields::getIndexedFields(indexReader),
                                maxDocFreq)
{
}

QueryAutoStopWordAnalyzer::QueryAutoStopWordAnalyzer(
    shared_ptr<Analyzer> delegate_, shared_ptr<IndexReader> indexReader,
    float maxPercentDocs) 
    : QueryAutoStopWordAnalyzer(delegate_, indexReader,
                                MultiFields::getIndexedFields(indexReader),
                                maxPercentDocs)
{
}

QueryAutoStopWordAnalyzer::QueryAutoStopWordAnalyzer(
    shared_ptr<Analyzer> delegate_, shared_ptr<IndexReader> indexReader,
    shared_ptr<deque<wstring>> fields,
    float maxPercentDocs) 
    : QueryAutoStopWordAnalyzer(delegate_, indexReader, fields,
                                (int)(indexReader->numDocs() * maxPercentDocs))
{
}

QueryAutoStopWordAnalyzer::QueryAutoStopWordAnalyzer(
    shared_ptr<Analyzer> delegate_, shared_ptr<IndexReader> indexReader,
    shared_ptr<deque<wstring>> fields, int maxDocFreq) 
    : org::apache::lucene::analysis::AnalyzerWrapper(
          delegate_->getReuseStrategy()),
      delegate_(delegate_)
{

  for (auto field : fields) {
    shared_ptr<Set<wstring>> stopWords = unordered_set<wstring>();
    shared_ptr<Terms> terms = MultiFields::getTerms(indexReader, field);
    shared_ptr<CharsRefBuilder> spare = make_shared<CharsRefBuilder>();
    if (terms != nullptr) {
      shared_ptr<TermsEnum> te = terms->begin();
      shared_ptr<BytesRef> text;
      while ((text = te->next()) != nullptr) {
        if (te->docFreq() > maxDocFreq) {
          spare->copyUTF8Bytes(text);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          stopWords->add(spare->toString());
        }
      }
    }
    stopWordsPerField.emplace(field, stopWords);
  }
}

shared_ptr<Analyzer>
QueryAutoStopWordAnalyzer::getWrappedAnalyzer(const wstring &fieldName)
{
  return delegate_;
}

shared_ptr<Analyzer::TokenStreamComponents>
QueryAutoStopWordAnalyzer::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  shared_ptr<Set<wstring>> stopWords = stopWordsPerField[fieldName];
  if (stopWords == nullptr) {
    return components;
  }
  shared_ptr<StopFilter> stopFilter =
      make_shared<StopFilter>(components->getTokenStream(),
                              make_shared<CharArraySet>(stopWords, false));
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(), stopFilter);
}

std::deque<wstring>
QueryAutoStopWordAnalyzer::getStopWords(const wstring &fieldName)
{
  shared_ptr<Set<wstring>> stopWords = stopWordsPerField[fieldName];
  return stopWords != nullptr
             ? stopWords->toArray(std::deque<wstring>(stopWords->size()))
             : std::deque<wstring>(0);
}

std::deque<std::shared_ptr<Term>> QueryAutoStopWordAnalyzer::getStopWords()
{
  deque<std::shared_ptr<Term>> allStopWords = deque<std::shared_ptr<Term>>();
  for (auto fieldName : stopWordsPerField) {
    shared_ptr<Set<wstring>> stopWords = stopWordsPerField[fieldName.first];
    for (auto text : stopWords) {
      allStopWords.push_back(make_shared<Term>(fieldName.first, text));
    }
  }
  return allStopWords.toArray(
      std::deque<std::shared_ptr<Term>>(allStopWords.size()));
}
} // namespace org::apache::lucene::analysis::query