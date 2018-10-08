#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::replicator::nrt
{
class Node;
}
namespace org::apache::lucene::search
{
class SearcherFactory;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

namespace org::apache::lucene::replicator::nrt
{

using IndexReader = org::apache::lucene::index::IndexReader;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using Directory = org::apache::lucene::store::Directory;

/** A SearcherManager that refreshes via an externally provided (NRT)
 * SegmentInfos, either from {@link IndexWriter} or via nrt replication to
 * another index.
 *
 * @lucene.experimental */
class SegmentInfosSearcherManager
    : public ReferenceManager<std::shared_ptr<IndexSearcher>>
{
  GET_CLASS_NAME(SegmentInfosSearcherManager)
private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile org.apache.lucene.index.SegmentInfos
  // currentInfos;
  std::shared_ptr<SegmentInfos> currentInfos;
  const std::shared_ptr<Directory> dir;
  const std::shared_ptr<Node> node;
  const std::shared_ptr<AtomicInteger> openReaderCount =
      std::make_shared<AtomicInteger>();
  const std::shared_ptr<SearcherFactory> searcherFactory;

public:
  SegmentInfosSearcherManager(
      std::shared_ptr<Directory> dir, std::shared_ptr<Node> node,
      std::shared_ptr<SegmentInfos> infosIn,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

protected:
  int getRefCount(std::shared_ptr<IndexSearcher> s) override;

  bool tryIncRef(std::shared_ptr<IndexSearcher> s) override;

  void decRef(std::shared_ptr<IndexSearcher> s)  override;

public:
  virtual std::shared_ptr<SegmentInfos> getCurrentInfos();

  /** Switch to new segments, refreshing if necessary.  Note that it's the
   * caller job to ensure there's a held refCount for the
   *  incoming infos, so all files exist. */
  virtual void
  setCurrentInfos(std::shared_ptr<SegmentInfos> infos) ;

protected:
  std::shared_ptr<IndexSearcher> refreshIfNeeded(
      std::shared_ptr<IndexSearcher> old)  override;

private:
  void addReaderClosedListener(std::shared_ptr<IndexReader> r);

  /** Tracks how many readers are still open, so that when we are closed,
   *  we can additionally wait until all in-flight searchers are
   *  closed. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void onReaderClosed();

protected:
  std::shared_ptr<SegmentInfosSearcherManager> shared_from_this()
  {
    return std::static_pointer_cast<SegmentInfosSearcherManager>(
        org.apache.lucene.search.ReferenceManager<
            org.apache.lucene.search.IndexSearcher>::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator::nrt
