#pragma once
#include "AbstractQueryMaker.h"
#include "QueryMaker.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
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

using Query = org::apache::lucene::search::Query;

/**
 * A QueryMaker that makes queries for a collection created
 * using {@link org.apache.lucene.benchmark.byTask.feeds.SingleDocSource}.
 */
class SimpleQueryMaker : public AbstractQueryMaker, public QueryMaker
{
  GET_CLASS_NAME(SimpleQueryMaker)

  /**
   * Prepare the queries for this test.
   * Extending classes can override this method for preparing different queries.
   * @return prepared queries.
   * @throws Exception if cannot prepare the queries.
   */
protected:
  std::deque<std::shared_ptr<Query>>
  prepareQueries()  override;

protected:
  std::shared_ptr<SimpleQueryMaker> shared_from_this()
  {
    return std::static_pointer_cast<SimpleQueryMaker>(
        AbstractQueryMaker::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
