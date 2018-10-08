#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityQuery.h"

#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityQueryParser.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/benchmark/quality/Judge.h"
#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityStats.h"
#include  "core/src/java/org/apache/lucene/benchmark/quality/utils/SubmissionReport.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"

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
namespace org::apache::lucene::benchmark::quality
{

using SubmissionReport =
    org::apache::lucene::benchmark::quality::utils::SubmissionReport;
using Query = org::apache::lucene::search::Query;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * Main entry point for running a quality benchmark.
 * <p>
 * There are two main configurations for running a quality benchmark: <ul>
 * <li>Against existing judgements.</li>
 * <li>For submission (e.g. for a contest).</li>
 * </ul>
 * The first configuration requires a non null
 * {@link org.apache.lucene.benchmark.quality.Judge Judge}.
 * The second configuration requires a non null
 * {@link org.apache.lucene.benchmark.quality.utils.SubmissionReport
 * SubmissionLogger}.
 */
class QualityBenchmark : public std::enable_shared_from_this<QualityBenchmark>
{
  GET_CLASS_NAME(QualityBenchmark)

  /** Quality Queries that this quality benchmark would execute. */
protected:
  std::deque<std::shared_ptr<QualityQuery>> qualityQueries;

  /** Parser for turning QualityQueries into Lucene Queries. */
  std::shared_ptr<QualityQueryParser> qqParser;

  /** Index to be searched. */
  std::shared_ptr<IndexSearcher> searcher;

  /** index field to extract doc name for each search result; used for judging
   * the results. */
  std::wstring docNameField;

  /** maximal number of queries that this quality benchmark runs. Default:
   * maxint. Useful for debugging. */
private:
  int maxQueries = std::numeric_limits<int>::max();

  /** maximal number of results to collect for each query. Default: 1000. */
  int maxResults = 1000;

  /**
   * Create a QualityBenchmark.
   * @param qqs quality queries to run.
   * @param qqParser parser for turning QualityQueries into Lucene Queries.
   * @param searcher index to be searched.
   * @param docNameField name of field containing the document name.
   *        This allows to extract the doc name for search results,
   *        and is important for judging the results.
   */
public:
  QualityBenchmark(std::deque<std::shared_ptr<QualityQuery>> &qqs,
                   std::shared_ptr<QualityQueryParser> qqParser,
                   std::shared_ptr<IndexSearcher> searcher,
                   const std::wstring &docNameField);

  /**
   * Run the quality benchmark.
   * @param judge the judge that can tell if a certain result doc is relevant
   * for a certain quality query. If null, no judgements would be made. Usually
   * null for a submission run.
   * @param submitRep submission report is created if non null.
   * @param qualityLog If not null, quality run data would be printed for each
   * query.
   * @return QualityStats of each quality query that was executed.
   * @throws Exception if quality benchmark failed to run.
   */
  virtual std::deque<std::shared_ptr<QualityStats>>
  execute(std::shared_ptr<Judge> judge,
          std::shared_ptr<SubmissionReport> submitRep,
          std::shared_ptr<PrintWriter> qualityLog) ;

  /* Analyze/judge results for a single quality query; optionally log them. */
private:
  std::shared_ptr<QualityStats>
  analyzeQueryResults(std::shared_ptr<QualityQuery> qq,
                      std::shared_ptr<Query> q, std::shared_ptr<TopDocs> td,
                      std::shared_ptr<Judge> judge,
                      std::shared_ptr<PrintWriter> logger,
                      int64_t searchTime) ;

  /**
   * @return the maximum number of quality queries to run. Useful at debugging.
   */
public:
  virtual int getMaxQueries();

  /**
   * Set the maximum number of quality queries to run. Useful at debugging.
   */
  virtual void setMaxQueries(int maxQueries);

  /**
   * @return the maximum number of results to collect for each quality query.
   */
  virtual int getMaxResults();

  /**
   * set the maximum number of results to collect for each quality query.
   */
  virtual void setMaxResults(int maxResults);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/
