#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityQuery.h"

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

/**
 * Judge if a document is relevant for a quality query.
 */
class Judge
{
  GET_CLASS_NAME(Judge)

  /**
   * Judge if document <code>docName</code> is relevant for the given quality
   * query.
   * @param docName name of doc tested for relevancy.
   * @param query tested quality query.
   * @return true if relevant, false if not.
   */
public:
  virtual bool isRelevant(const std::wstring &docName,
                          std::shared_ptr<QualityQuery> query) = 0;

  /**
   * Validate that queries and this Judge match each other.
   * To be perfectly valid, this Judge must have some data for each and every
   * input quality query, and must not have any data on any other quality query.
   * <b>Note</b>: the quality benchmark run would not fail in case of imperfect
   * validity, just a warning message would be logged.
   * @param qq quality queries to be validated.
   * @param logger if not null, validation issues are logged.
   * @return true if perfectly valid, false if not.
   */
  virtual bool validateData(std::deque<std::shared_ptr<QualityQuery>> &qq,
                            std::shared_ptr<PrintWriter> logger) = 0;

  /**
   * Return the maximal recall for the input quality query.
   * It is the number of relevant docs this Judge "knows" for the query.
   * @param query the query whose maximal recall is needed.
   */
  virtual int maxRecall(std::shared_ptr<QualityQuery> query) = 0;
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/
