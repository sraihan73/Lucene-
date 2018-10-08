#pragma once
#include "QueryMaker.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
 * Abstract base query maker.
 * Each query maker should just implement the {@link #prepareQueries()} method.
 **/
class AbstractQueryMaker
    : public std::enable_shared_from_this<AbstractQueryMaker>,
      public QueryMaker
{
  GET_CLASS_NAME(AbstractQueryMaker)

protected:
  int qnum = 0;
  std::deque<std::shared_ptr<Query>> queries;
  std::shared_ptr<Config> config;

public:
  void resetInputs()  override;

protected:
  virtual std::deque<std::shared_ptr<Query>> prepareQueries() = 0;

public:
  void
  setConfig(std::shared_ptr<Config> config)  override;

  std::wstring printQueries() override;

  std::shared_ptr<Query> makeQuery()  override;

  // return next qnum
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int nextQnum();

  /*
   *  (non-Javadoc)
   * @see org.apache.lucene.benchmark.byTask.feeds.QueryMaker#makeQuery(int)
   */
public:
  std::shared_ptr<Query> makeQuery(int size)  override;
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
