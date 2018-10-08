#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityQuery.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
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
namespace org::apache::lucene::benchmark::quality::utils
{

using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * Create a log ready for submission.
 * Extend this class and override
 * {@link #report(QualityQuery, TopDocs, std::wstring, IndexSearcher)}
GET_CLASS_NAME(and)
 * to create different reports.
 */
class SubmissionReport : public std::enable_shared_from_this<SubmissionReport>
{
  GET_CLASS_NAME(SubmissionReport)

private:
  std::shared_ptr<NumberFormat> nf;
  std::shared_ptr<PrintWriter> logger;
  std::wstring name;

  /**
   * Constructor for SubmissionReport.
   * @param logger if null, no submission data is created.
   * @param name name of this run.
   */
public:
  SubmissionReport(std::shared_ptr<PrintWriter> logger,
                   const std::wstring &name);

  /**
   * Report a search result for a certain quality query.
   * @param qq quality query for which the results are reported.
   * @param td search results for the query.
   * @param docNameField stored field used for fetching the result doc name.
   * @param searcher index access for fetching doc name.
   * @throws IOException in case of a problem.
   */
  virtual void
  report(std::shared_ptr<QualityQuery> qq, std::shared_ptr<TopDocs> td,
         const std::wstring &docNameField,
         std::shared_ptr<IndexSearcher> searcher) ;

  virtual void flush();

private:
  static std::wstring padd;
  std::wstring format(const std::wstring &s, int minLen);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/utils/
