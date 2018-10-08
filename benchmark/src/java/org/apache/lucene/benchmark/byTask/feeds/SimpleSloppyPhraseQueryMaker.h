#pragma once
#include "SimpleQueryMaker.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
 * Create sloppy phrase queries for performance test, in an index created using
 * simple doc maker.
 */
class SimpleSloppyPhraseQueryMaker : public SimpleQueryMaker
{
  GET_CLASS_NAME(SimpleSloppyPhraseQueryMaker)

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.benchmark.byTask.feeds.SimpleQueryMaker#prepareQueries()
   */
protected:
  std::deque<std::shared_ptr<Query>>
  prepareQueries()  override;

protected:
  std::shared_ptr<SimpleSloppyPhraseQueryMaker> shared_from_this()
  {
    return std::static_pointer_cast<SimpleSloppyPhraseQueryMaker>(
        SimpleQueryMaker::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
