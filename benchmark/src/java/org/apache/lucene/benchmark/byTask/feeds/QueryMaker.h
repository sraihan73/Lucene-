#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
}

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
using Query = org::apache::lucene::search::Query;

/**
 * Create queries for the test.
 */
class QueryMaker
{
  GET_CLASS_NAME(QueryMaker)

  /**
   * Create the next query, of the given size.
   * @param size the size of the query - number of terms, etc.
   * @exception Exception if cannot make the query, or if size &gt; 0 was
   * specified but this feature is not supported.
   */
public:
  virtual std::shared_ptr<Query> makeQuery(int size) = 0;

  /** Create the next query */
  virtual std::shared_ptr<Query> makeQuery() = 0;

  /** Set the properties */
  virtual void setConfig(std::shared_ptr<Config> config) = 0;

  /** Reset inputs so that the test run would behave, input wise, as if it just
   * started. */
  virtual void resetInputs() = 0;

  /** Print the queries */
  virtual std::wstring printQueries() = 0;
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
