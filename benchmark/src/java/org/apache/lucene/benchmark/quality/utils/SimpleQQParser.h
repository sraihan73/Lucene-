#pragma once
#include "../QualityQueryParser.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"

#include  "core/src/java/org/apache/lucene/benchmark/quality/QualityQuery.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/ParseException.h"

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
using QualityQueryParser =
    org::apache::lucene::benchmark::quality::QualityQueryParser;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;

/**
 * Simplistic quality query parser. A Lucene query is created by passing
 * the value of the specified QualityQuery name-value pair(s) into
 * a Lucene's QueryParser using StandardAnalyzer. */
class SimpleQQParser : public std::enable_shared_from_this<SimpleQQParser>,
                       public QualityQueryParser
{
  GET_CLASS_NAME(SimpleQQParser)

private:
  std::deque<std::wstring> qqNames;
  std::wstring indexField;

public:
  std::shared_ptr<ThreadLocal<std::shared_ptr<QueryParser>>> queryParser =
      std::make_shared<ThreadLocal<std::shared_ptr<QueryParser>>>();

  /**
   * Constructor of a simple qq parser.
   * @param qqNames name-value pairs of quality query to use for creating the
   * query
   * @param indexField corresponding index field
   */
  SimpleQQParser(std::deque<std::wstring> &qqNames,
                 const std::wstring &indexField);

  /**
   * Constructor of a simple qq parser.
   * @param qqName name-value pair of quality query to use for creating the
   * query
   * @param indexField corresponding index field
   */
  SimpleQQParser(const std::wstring &qqName, const std::wstring &indexField);

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.benchmark.quality.QualityQueryParser#parse(org.apache.lucene.benchmark.quality.QualityQuery)
   */
  std::shared_ptr<Query>
  parse(std::shared_ptr<QualityQuery> qq)  override;
};

} // #include  "core/src/java/org/apache/lucene/benchmark/quality/utils/
