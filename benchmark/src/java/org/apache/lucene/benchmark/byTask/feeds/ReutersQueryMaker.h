#pragma once
#include "AbstractQueryMaker.h"
#include "QueryMaker.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Query = org::apache::lucene::search::Query;

/**
 * A QueryMaker that makes queries devised manually (by Grant Ingersoll) for
 * searching in the Reuters collection.
 */
class ReutersQueryMaker : public AbstractQueryMaker, public QueryMaker
{
  GET_CLASS_NAME(ReutersQueryMaker)

private:
  static std::deque<std::wstring> STANDARD_QUERIES;

  static std::deque<std::shared_ptr<Query>>
  getPrebuiltQueries(const std::wstring &field);

  /**
   * Parse the strings containing Lucene queries.
   *
   * @param qs array of strings containing query expressions
   * @param a  analyzer to use when parsing queries
   * @return array of Lucene queries
   */
  static std::deque<std::shared_ptr<Query>>
  createQueries(std::deque<std::any> &qs, std::shared_ptr<Analyzer> a);

protected:
  std::deque<std::shared_ptr<Query>>
  prepareQueries()  override;

protected:
  std::shared_ptr<ReutersQueryMaker> shared_from_this()
  {
    return std::static_pointer_cast<ReutersQueryMaker>(
        AbstractQueryMaker::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds