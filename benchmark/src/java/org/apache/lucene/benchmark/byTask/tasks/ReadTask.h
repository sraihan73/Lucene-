#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class QueryMaker;
}

namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search
{
class Collector;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Sort;
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
namespace org::apache::lucene::benchmark::byTask::tasks
{

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using QueryMaker = org::apache::lucene::benchmark::byTask::feeds::QueryMaker;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using Collector = org::apache::lucene::search::Collector;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;

/**
 * Read index (abstract) task.
 * Sub classes implement withSearch(), withWarm(), withTraverse() and
 * withRetrieve() methods to configure the actual action. <p>Note: All ReadTasks
 * reuse the reader if it is already open. Otherwise a reader is opened at start
 * and closed at the end. <p> The <code>search.num.hits</code> config parameter
 * sets the top number of hits to collect during searching.  If
 * <code>print.hits.field</code> is set, then each hit is
 * printed along with the value of that field.</p>
 *
 * <p>Other side effects: none.
 */
class ReadTask : public PerfTask
{
  GET_CLASS_NAME(ReadTask)

private:
  const std::shared_ptr<QueryMaker> queryMaker;

public:
  ReadTask(std::shared_ptr<PerfRunData> runData);
  int doLogic()  override;

protected:
  virtual int
  withTopDocs(std::shared_ptr<IndexSearcher> searcher, std::shared_ptr<Query> q,
              std::shared_ptr<TopDocs> hits) ;

  virtual std::shared_ptr<Collector>
  createCollector() ;

  virtual std::shared_ptr<Document> retrieveDoc(std::shared_ptr<IndexReader> ir,
                                                int id) ;

  /**
   * Return query maker used for this task.
   */
public:
  virtual std::shared_ptr<QueryMaker> getQueryMaker() = 0;

  /**
   * Return true if search should be performed.
   */
  virtual bool withSearch() = 0;

  virtual bool withCollector();

  /**
   * Return true if warming should be performed.
   */
  virtual bool withWarm() = 0;

  /**
   * Return true if, with search, results should be traversed.
   */
  virtual bool withTraverse() = 0;

  /** Whether scores should be computed (only useful with
   *  field sort) */
  virtual bool withScore();

  /** Whether maxScores should be computed (only useful with
   *  field sort) */
  virtual bool withMaxScore();

  /** Whether totalHits should be computed (only useful with
   *  field sort) */
  virtual bool withTotalHits();

  /**
   * Specify the number of hits to traverse.  Tasks should override this if they
   * want to restrict the number of hits that are traversed when {@link
   * #withTraverse()} is true. Must be greater than 0. <p> Read task calculates
   * the traversal as: Math.min(hits.length(), traversalSize())
   *
   * @return Integer.MAX_VALUE
   */
  virtual int traversalSize();

  static constexpr int DEFAULT_SEARCH_NUM_HITS = 10;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int numHits_ = 0;

public:
  void setup()  override;

  /**
   * Specify the number of hits to retrieve.  Tasks should override this if they
   * want to restrict the number of hits that are collected during searching.
   * Must be greater than 0.
   *
   * @return 10 by default, or search.num.hits config if set.
   */
  virtual int numHits();

  /**
   * Return true if, with search and results traversing, docs should be
   * retrieved.
   */
  virtual bool withRetrieve() = 0;

protected:
  virtual std::shared_ptr<Sort> getSort();

protected:
  std::shared_ptr<ReadTask> shared_from_this()
  {
    return std::static_pointer_cast<ReadTask>(PerfTask::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
