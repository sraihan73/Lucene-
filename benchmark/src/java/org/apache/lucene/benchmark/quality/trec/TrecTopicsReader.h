#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::benchmark::quality::trec
{

using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;

/**
 * Read TREC topics.
 * <p>
 * Expects this topic format -
 * <pre>
 *   &lt;top&gt;
 *   &lt;num&gt; Number: nnn
 *
 *   &lt;title&gt; title of the topic
 *
 *   &lt;desc&gt; Description:
 *   description of the topic
 *
 *   &lt;narr&gt; Narrative:
 *   "story" composed by assessors.
 *
 *   &lt;/top&gt;
 * </pre>
 * Comment lines starting with '#' are ignored.
 */
class TrecTopicsReader : public std::enable_shared_from_this<TrecTopicsReader>
{
  GET_CLASS_NAME(TrecTopicsReader)

private:
  static const std::wstring newline;

  /**
   *  Constructor for Trec's TopicsReader
   */
public:
  // C++ TODO: No base class can be determined:
  TrecTopicsReader(); // super();

  /**
   * Read quality queries from trec format topics file.
   * @param reader where queries are read from.
   * @return the result quality queries.
   * @throws IOException if cannot read the queries.
   */
  virtual std::deque<std::shared_ptr<QualityQuery>>
  readQueries(std::shared_ptr<BufferedReader> reader) ;

  // read until finding a line that starts with the specified prefix
private:
  std::shared_ptr<StringBuilder> read(std::shared_ptr<BufferedReader> reader,
                                      const std::wstring &prefix,
                                      std::shared_ptr<StringBuilder> sb,
                                      bool collectMatchLine,
                                      bool collectAll) ;
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/trec/
