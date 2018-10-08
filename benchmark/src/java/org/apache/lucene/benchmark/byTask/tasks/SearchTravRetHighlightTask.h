#pragma once
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/highlight/DefaultEncoder.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"
#include "../../../../../../../../../highlighter/src/java/org/apache/lucene/search/vectorhighlight/BreakIteratorBoundaryScanner.h"
#include "SearchTravTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/HLImpl.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"
#include  "core/src/java/org/apache/lucene/search/highlight/SimpleHTMLFormatter.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Encoder.h"
#include  "core/src/java/org/apache/lucene/search/highlight/Highlighter.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedFragListBuilder.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/BoundaryScanner.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/ScoreOrderFragmentsBuilder.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FastVectorHighlighter.h"
#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace org::apache::lucene::benchmark::byTask::tasks
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using DefaultEncoder = org::apache::lucene::search::highlight::DefaultEncoder;
using Encoder = org::apache::lucene::search::highlight::Encoder;
using Highlighter = org::apache::lucene::search::highlight::Highlighter;
using SimpleHTMLFormatter =
    org::apache::lucene::search::highlight::SimpleHTMLFormatter;
using UnifiedHighlighter =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter;
using BoundaryScanner =
    org::apache::lucene::search::vectorhighlight::BoundaryScanner;
using BreakIteratorBoundaryScanner =
    org::apache::lucene::search::vectorhighlight::BreakIteratorBoundaryScanner;
using FastVectorHighlighter =
    org::apache::lucene::search::vectorhighlight::FastVectorHighlighter;
using ScoreOrderFragmentsBuilder =
    org::apache::lucene::search::vectorhighlight::ScoreOrderFragmentsBuilder;
using WeightedFragListBuilder =
    org::apache::lucene::search::vectorhighlight::WeightedFragListBuilder;

/**
 * Search and Traverse and Retrieve docs task.  Highlight the fields in the
 * retrieved documents.
 *
 * <p>Note: This task reuses the reader if it is already open.
 * Otherwise a reader is opened at start and closed at the end.
 * </p>
 *
 * <p>Takes optional multivalued, comma separated param string as:
 * type[&lt;enum&gt;],maxFrags[&lt;int&gt;],fields[name1;name2;...]</p> <ul>
 * <li>type - the highlighter implementation, e.g. "UH"</li>
 * <li>maxFrags - The maximum number of fragments to score by the
 * highlighter</li> <li>fields - The fields to highlight.  If not specified all
 * fields will be highlighted (or at least attempted)</li>
 * </ul>
 * Example:
 * <pre>"SearchHlgtSameRdr"
 * SearchTravRetHighlight(type[UH],maxFrags[3],fields[body]) &gt; : 1000
 * </pre>
 *
 * Documents must be stored in order for this task to work.  Additionally, term
 * deque positions can be used as well, and offsets in postings is another
 * option.
 *
 * <p>Other side effects: counts additional 1 (record) for each traversed hit,
 * and 1 more for each retrieved (non null) document and 1 for each fragment
 * returned.</p>
 */
class SearchTravRetHighlightTask : public SearchTravTask
{
  GET_CLASS_NAME(SearchTravRetHighlightTask)
private:
  int maxDocCharsToAnalyze = 0; // max leading content chars to highlight
  int maxFrags = 1;             // aka passages
  std::shared_ptr<Set<std::wstring>> hlFields = Collections::singleton(L"body");
  std::wstring type;
  std::shared_ptr<HLImpl> hlImpl;
  std::shared_ptr<Analyzer> analyzer;

public:
  SearchTravRetHighlightTask(std::shared_ptr<PerfRunData> runData);

  void setParams(const std::wstring &params) override;

  void setup()  override;

  // here is where we intercept ReadTask's logic to do the highlighting, and
  // nothing else (no retrieval of all field vals)
protected:
  int withTopDocs(
      std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
      std::shared_ptr<TopDocs> hits)  override;

private:
  class HLImpl
  {
    GET_CLASS_NAME(HLImpl)
  public:
    virtual void withTopDocs(std::shared_ptr<IndexSearcher> searcher,
                             std::shared_ptr<Query> q,
                             std::shared_ptr<TopDocs> hits) = 0;
  };

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile int preventOptimizeAway = 0;
  int preventOptimizeAway = 0;

private:
  class StandardHLImpl : public std::enable_shared_from_this<StandardHLImpl>,
                         public HLImpl
  {
    GET_CLASS_NAME(StandardHLImpl)
  private:
    std::shared_ptr<SearchTravRetHighlightTask> outerInstance;

  public:
    std::shared_ptr<SimpleHTMLFormatter> formatter =
        std::make_shared<SimpleHTMLFormatter>(L"<em>", L"</em>");
    std::shared_ptr<DefaultEncoder> encoder =
        std::make_shared<DefaultEncoder>();
    std::shared_ptr<Highlighter> highlighter =
        std::make_shared<Highlighter>(formatter, encoder, nullptr);
    bool termVecs = false;

    StandardHLImpl(std::shared_ptr<SearchTravRetHighlightTask> outerInstance,
                   bool termVecs);

    void withTopDocs(
        std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
        std::shared_ptr<TopDocs> hits)  override;
  };

private:
  class FastVectorHLImpl
      : public std::enable_shared_from_this<FastVectorHLImpl>,
        public HLImpl
  {
    GET_CLASS_NAME(FastVectorHLImpl)
  private:
    std::shared_ptr<SearchTravRetHighlightTask> outerInstance;

  public:
    FastVectorHLImpl(std::shared_ptr<SearchTravRetHighlightTask> outerInstance);

    int fragSize = 100;
    std::shared_ptr<WeightedFragListBuilder> fragListBuilder =
        std::make_shared<WeightedFragListBuilder>();
    std::shared_ptr<BoundaryScanner> bs =
        std::make_shared<BreakIteratorBoundaryScanner>(
            BreakIterator::getSentenceInstance(Locale::ENGLISH));
    std::shared_ptr<ScoreOrderFragmentsBuilder> fragmentsBuilder =
        std::make_shared<ScoreOrderFragmentsBuilder>(bs);
    std::deque<std::wstring> preTags = {L"<em>"};
    std::deque<std::wstring> postTags = {L"</em>"};
    std::shared_ptr<Encoder> encoder =
        std::make_shared<DefaultEncoder>(); // new SimpleHTMLEncoder();
    std::shared_ptr<FastVectorHighlighter> highlighter =
        std::make_shared<FastVectorHighlighter>(
            true, false); // requireFieldMatch -- not pertinent to our benchmark

    void withTopDocs(
        std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
        std::shared_ptr<TopDocs> hits)  override;
  };

private:
  std::deque<std::shared_ptr<ScoreDoc>>
  docIdOrder(std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs);

private:
  class UnifiedHLImpl : public std::enable_shared_from_this<UnifiedHLImpl>,
                        public HLImpl
  {
    GET_CLASS_NAME(UnifiedHLImpl)
  private:
    std::shared_ptr<SearchTravRetHighlightTask> outerInstance;

  public:
    std::shared_ptr<UnifiedHighlighter> highlighter;
    std::shared_ptr<IndexSearcher> lastSearcher;
    UnifiedHighlighter::OffsetSource offsetSource =
        static_cast<UnifiedHighlighter::OffsetSource>(
            0); // null means auto select
    std::deque<std::wstring> fields = outerInstance->hlFields->toArray(
        std::deque<std::wstring>(outerInstance->hlFields->size()));
    std::deque<int> maxPassages;

    UnifiedHLImpl(std::shared_ptr<SearchTravRetHighlightTask> outerInstance,
                  UnifiedHighlighter::OffsetSource const offsetSource);

  private:
    void reset(std::shared_ptr<IndexSearcher> searcher);

  private:
    class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
    {
      GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
    private:
      std::shared_ptr<UnifiedHLImpl> outerInstance;

    public:
      UnifiedHighlighterAnonymousInnerClass(
          std::shared_ptr<UnifiedHLImpl> outerInstance,
          std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<Analyzer> analyzer);

    protected:
      OffsetSource getOffsetSource(const std::wstring &field) override;

    protected:
      std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
            org.apache.lucene.search.uhighlight
                .UnifiedHighlighter::shared_from_this());
      }
    };

  public:
    void withTopDocs(
        std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
        std::shared_ptr<TopDocs> hits)  override;
  };

private:
  class NoHLImpl : public std::enable_shared_from_this<NoHLImpl>, public HLImpl
  {
    GET_CLASS_NAME(NoHLImpl)
  private:
    std::shared_ptr<SearchTravRetHighlightTask> outerInstance;

  public:
    NoHLImpl(std::shared_ptr<SearchTravRetHighlightTask> outerInstance);

    void withTopDocs(
        std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
        std::shared_ptr<TopDocs> hits)  override;
  };

protected:
  std::shared_ptr<SearchTravRetHighlightTask> shared_from_this()
  {
    return std::static_pointer_cast<SearchTravRetHighlightTask>(
        SearchTravTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
