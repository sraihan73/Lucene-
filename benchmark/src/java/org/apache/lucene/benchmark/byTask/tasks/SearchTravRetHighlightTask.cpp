using namespace std;

#include "SearchTravRetHighlightTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/Encoder.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/Highlighter.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/QueryScorer.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/SimpleHTMLFormatter.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/TokenSources.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/BoundaryScanner.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/FastVectorHighlighter.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/FieldQuery.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/ScoreOrderFragmentsBuilder.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/WeightedFragListBuilder.h"
#include "../PerfRunData.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Document = org::apache::lucene::document::Document;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using DefaultEncoder = org::apache::lucene::search::highlight::DefaultEncoder;
using Encoder = org::apache::lucene::search::highlight::Encoder;
using Highlighter = org::apache::lucene::search::highlight::Highlighter;
using QueryScorer = org::apache::lucene::search::highlight::QueryScorer;
using SimpleHTMLFormatter =
    org::apache::lucene::search::highlight::SimpleHTMLFormatter;
using TokenSources = org::apache::lucene::search::highlight::TokenSources;
using UnifiedHighlighter =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter;
using BoundaryScanner =
    org::apache::lucene::search::vectorhighlight::BoundaryScanner;
using BreakIteratorBoundaryScanner =
    org::apache::lucene::search::vectorhighlight::BreakIteratorBoundaryScanner;
using FastVectorHighlighter =
    org::apache::lucene::search::vectorhighlight::FastVectorHighlighter;
using FieldQuery = org::apache::lucene::search::vectorhighlight::FieldQuery;
using ScoreOrderFragmentsBuilder =
    org::apache::lucene::search::vectorhighlight::ScoreOrderFragmentsBuilder;
using WeightedFragListBuilder =
    org::apache::lucene::search::vectorhighlight::WeightedFragListBuilder;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

SearchTravRetHighlightTask::SearchTravRetHighlightTask(
    shared_ptr<PerfRunData> runData)
    : SearchTravTask(runData)
{
}

void SearchTravRetHighlightTask::setParams(const wstring &params)
{
  // can't call super because super doesn't understand our params syntax
  this->params = params;
  // TODO consider instead using data.getConfig().get("highlighter.*")?
  std::deque<wstring> splits = params.split(L",");
  for (auto split : splits) {
    if (split.startsWith(L"type[") == true) {
      type =
          split.substr((wstring(L"type[")).length(),
                       (split.length() - 1) - ((wstring(L"type[")).length()));
    } else if (split.startsWith(L"maxFrags[") == true) {
      maxFrags = static_cast<int>(stof(split.substr(
          (wstring(L"maxFrags[")).length(),
          (split.length() - 1) - ((wstring(L"maxFrags[")).length()))));
    } else if (split.startsWith(L"fields[") == true) {
      wstring fieldNames =
          split.substr((wstring(L"fields[")).length(),
                       (split.length() - 1) - ((wstring(L"fields[")).length()));
      std::deque<wstring> fieldSplits = fieldNames.split(L";");
      hlFields = unordered_set<>(Arrays::asList(fieldSplits));
    }
  }
}

void SearchTravRetHighlightTask::setup() 
{
  SearchTravTask::setup();
  // check to make sure either the doc is being stored
  shared_ptr<PerfRunData> data = getRunData();
  if (data->getConfig()->get(L"doc.stored", false) == false) {
    throw runtime_error(L"doc.stored must be set to true");
  }
  maxDocCharsToAnalyze =
      data->getConfig()->get(L"highlighter.maxDocCharsToAnalyze",
                             Highlighter::DEFAULT_MAX_CHARS_TO_ANALYZE);
  analyzer = data->getAnalyzer();
  wstring type = this->type;
  if (type == L"") {
    type = data->getConfig()->get(L"highlighter", L"");
  }
  switch (type) {
  case L"NONE":
    hlImpl = make_shared<NoHLImpl>(shared_from_this());
    break;
  case L"SH_A":
    hlImpl = make_shared<StandardHLImpl>(shared_from_this(), false);
    break;
  case L"SH_V":
    hlImpl = make_shared<StandardHLImpl>(shared_from_this(), true);
    break;

  case L"FVH_V":
    hlImpl = make_shared<FastVectorHLImpl>(shared_from_this());
    break;

  case L"UH":
    hlImpl = make_shared<UnifiedHLImpl>(shared_from_this(), nullptr);
    break;
  case L"UH_A":
    hlImpl = make_shared<UnifiedHLImpl>(
        shared_from_this(), UnifiedHighlighter::OffsetSource::ANALYSIS);
    break;
  case L"UH_V":
    hlImpl = make_shared<UnifiedHLImpl>(
        shared_from_this(), UnifiedHighlighter::OffsetSource::TERM_VECTORS);
    break;
  case L"UH_P":
    hlImpl = make_shared<UnifiedHLImpl>(
        shared_from_this(), UnifiedHighlighter::OffsetSource::POSTINGS);
    break;
  case L"UH_PV":
    hlImpl = make_shared<UnifiedHLImpl>(
        shared_from_this(),
        UnifiedHighlighter::OffsetSource::POSTINGS_WITH_TERM_VECTORS);
    break;

  default:
    throw runtime_error(L"unrecognized highlighter type: " + type +
                        L" (try 'UH')");
  }
}

int SearchTravRetHighlightTask::withTopDocs(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    shared_ptr<TopDocs> hits) 
{
  hlImpl->withTopDocs(searcher, q, hits);
  // note: it'd be nice if we knew the sum kilobytes of text across these hits
  // so we could return that. It'd be a more
  //  useful number to gauge the amount of work. But given "average" document
  //  sizes and lots of queries, returning the number of docs is reasonable.
  return hits->scoreDocs.size(); // always return # scored docs.
}

SearchTravRetHighlightTask::StandardHLImpl::StandardHLImpl(
    shared_ptr<SearchTravRetHighlightTask> outerInstance, bool termVecs)
    : outerInstance(outerInstance)
{
  highlighter->setEncoder(make_shared<DefaultEncoder>());
  highlighter->setMaxDocCharsToAnalyze(outerInstance->maxDocCharsToAnalyze);
  this->termVecs = termVecs;
}

void SearchTravRetHighlightTask::StandardHLImpl::withTopDocs(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    shared_ptr<TopDocs> hits) 
{
  shared_ptr<IndexReader> reader = searcher->getIndexReader();
  highlighter->setFragmentScorer(make_shared<QueryScorer>(q));
  // highlighter.setTextFragmenter();  unfortunately no sentence mechanism, not
  // even regex. Default here is trivial
  for (auto scoreDoc : outerInstance->docIdOrder(hits->scoreDocs)) {
    shared_ptr<Document> document =
        reader->document(scoreDoc->doc, outerInstance->hlFields);
    shared_ptr<Fields> tvFields =
        termVecs ? reader->getTermVectors(scoreDoc->doc) : nullptr;
    for (auto indexableField : document) {
      shared_ptr<TokenStream> tokenStream;
      if (termVecs) {
        tokenStream = TokenSources::getTokenStream(
            indexableField->name(), tvFields, indexableField->stringValue(),
            outerInstance->analyzer, outerInstance->maxDocCharsToAnalyze);
      } else {
        tokenStream = outerInstance->analyzer->tokenStream(
            indexableField->name(), indexableField->stringValue());
      }
      // will close TokenStream:
      std::deque<wstring> fragments = highlighter->getBestFragments(
          tokenStream, indexableField->stringValue(), outerInstance->maxFrags);
      outerInstance->preventOptimizeAway = fragments.size();
    }
  }
}

SearchTravRetHighlightTask::FastVectorHLImpl::FastVectorHLImpl(
    shared_ptr<SearchTravRetHighlightTask> outerInstance)
    : outerInstance(outerInstance)
{
}

void SearchTravRetHighlightTask::FastVectorHLImpl::withTopDocs(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    shared_ptr<TopDocs> hits) 
{
  shared_ptr<IndexReader> reader = searcher->getIndexReader();
  shared_ptr<FieldQuery> *const fq = highlighter->getFieldQuery(q, reader);
  for (auto scoreDoc : outerInstance->docIdOrder(hits->scoreDocs)) {
    for (auto hlField : outerInstance->hlFields) {
      std::deque<wstring> fragments = highlighter->getBestFragments(
          fq, reader, scoreDoc->doc, hlField, fragSize, outerInstance->maxFrags,
          fragListBuilder, fragmentsBuilder, preTags, postTags, encoder);
      outerInstance->preventOptimizeAway = fragments.size();
    }
  }
}

std::deque<std::shared_ptr<ScoreDoc>> SearchTravRetHighlightTask::docIdOrder(
    std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs)
{
  std::deque<std::shared_ptr<ScoreDoc>> clone(scoreDocs.size());
  System::arraycopy(scoreDocs, 0, clone, 0, scoreDocs.size());
  ArrayUtil::introSort(clone, [&](a, b) { Integer::compare(a::doc, b::doc); });
  return clone;
}

SearchTravRetHighlightTask::UnifiedHLImpl::UnifiedHLImpl(
    shared_ptr<SearchTravRetHighlightTask> outerInstance,
    UnifiedHighlighter::OffsetSource const offsetSource)
    : outerInstance(outerInstance)
{
  this->offsetSource = offsetSource;
  maxPassages = std::deque<int>(outerInstance->hlFields->size());
  Arrays::fill(maxPassages, outerInstance->maxFrags);
}

void SearchTravRetHighlightTask::UnifiedHLImpl::reset(
    shared_ptr<IndexSearcher> searcher)
{
  if (lastSearcher == searcher) {
    return;
  }
  lastSearcher = searcher;
  highlighter = make_shared<UnifiedHighlighterAnonymousInnerClass>(
      shared_from_this(), searcher, outerInstance->analyzer);
  highlighter->setBreakIterator([&]() {
    java::text::BreakIterator::getSentenceInstance(java::util::Locale::ENGLISH);
  });
  highlighter->setMaxLength(outerInstance->maxDocCharsToAnalyze);
  highlighter->setHighlightPhrasesStrictly(true);
  highlighter->setHandleMultiTermQuery(true);
}

SearchTravRetHighlightTask::UnifiedHLImpl::
    UnifiedHighlighterAnonymousInnerClass::
        UnifiedHighlighterAnonymousInnerClass(
            shared_ptr<UnifiedHLImpl> outerInstance,
            shared_ptr<IndexSearcher> searcher, shared_ptr<Analyzer> analyzer)
    : org::apache::lucene::search::uhighlight::UnifiedHighlighter(searcher,
                                                                  analyzer)
{
  this->outerInstance = outerInstance;
}

OffsetSource SearchTravRetHighlightTask::UnifiedHLImpl::
    UnifiedHighlighterAnonymousInnerClass::getOffsetSource(const wstring &field)
{
  return outerInstance->offsetSource != nullptr
             ? outerInstance->offsetSource
             : outerInstance->outerInstance->super.getOffsetSource(field);
}

void SearchTravRetHighlightTask::UnifiedHLImpl::withTopDocs(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    shared_ptr<TopDocs> hits) 
{
  reset(searcher);
  unordered_map<wstring, std::deque<wstring>> result =
      highlighter->highlightFields(fields, q, hits, maxPassages);
  outerInstance->preventOptimizeAway = result.size();
}

SearchTravRetHighlightTask::NoHLImpl::NoHLImpl(
    shared_ptr<SearchTravRetHighlightTask> outerInstance)
    : outerInstance(outerInstance)
{
}

void SearchTravRetHighlightTask::NoHLImpl::withTopDocs(
    shared_ptr<IndexSearcher> searcher, shared_ptr<Query> q,
    shared_ptr<TopDocs> hits) 
{
  // just retrieve the HL fields
  for (auto scoreDoc : outerInstance->docIdOrder(hits->scoreDocs)) {
    outerInstance->preventOptimizeAway +=
        searcher->doc(scoreDoc->doc, outerInstance->hlFields).begin()->hasNext()
            ? 2
            : 1;
  }
}
} // namespace org::apache::lucene::benchmark::byTask::tasks