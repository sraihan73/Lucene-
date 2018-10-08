#pragma once
#include "QueryMaker.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/classic/QueryParser.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using Query = org::apache::lucene::search::Query;
using com::ibm::icu::text::RuleBasedNumberFormat;

/**
 * Creates queries whose content is a spelled-out <code>long</code> number
 * starting from <code>{@link Long#MIN_VALUE} + 10</code>.
 */
class LongToEnglishQueryMaker
    : public std::enable_shared_from_this<LongToEnglishQueryMaker>,
      public QueryMaker
{
  GET_CLASS_NAME(LongToEnglishQueryMaker)
public:
  int64_t counter = std::numeric_limits<int64_t>::min() + 10;

protected:
  std::shared_ptr<QueryParser> parser;

  // TODO: we could take param to specify locale...
private:
  const std::shared_ptr<RuleBasedNumberFormat> rnbf =
      std::make_shared<RuleBasedNumberFormat>(Locale::ROOT,
                                              RuleBasedNumberFormat::SPELLOUT);

public:
  std::shared_ptr<Query> makeQuery(int size)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Query> makeQuery()  override;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t getNextCounter();

public:
  void
  setConfig(std::shared_ptr<Config> config)  override;

  void resetInputs() override;

  std::wstring printQueries() override;
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
