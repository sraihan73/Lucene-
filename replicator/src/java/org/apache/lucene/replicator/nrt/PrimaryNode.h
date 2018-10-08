#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentInfos;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::replicator::nrt
{
class CopyState;
}
namespace org::apache::lucene::search
{
class SearcherFactory;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::replicator::nrt
{
class FileMetaData;
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

using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;

/*
 * This just asks IndexWriter to open new NRT reader, in order to publish a new
 * NRT point.  This could be improved, if we separated out 1) nrt flush (and
 * incRef the SIS) from 2) opening a new reader, but this is tricky with IW's
 * concurrency, and it would also be hard-ish to share IW's reader pool with our
 * searcher manager.  So we do the simpler solution now, but that adds some
 * unecessary latency to NRT refresh on replicas since step 2) could otherwise
 * be done concurrently with replicas copying files over.
 */

/** Node that holds an IndexWriter, indexing documents into its local index.
 *
 * @lucene.experimental */

class PrimaryNode : public Node
{
  GET_CLASS_NAME(PrimaryNode)

  // Current NRT segment infos, incRef'd with IndexWriter.deleter:
private:
  std::shared_ptr<SegmentInfos> curInfos;

protected:
  const std::shared_ptr<IndexWriter> writer;

  // IncRef'd state of the last published NRT point; when a replica comes
  // asking, we give it this as the current NRT point:
private:
  std::shared_ptr<CopyState> copyState;

protected:
  const int64_t primaryGen;

  /** Contains merged segments that have been copied to all running replicas (as
   * of when that merge started warming). */
public:
  const std::shared_ptr<Set<std::wstring>> finishedMergedFiles =
      Collections::synchronizedSet(std::unordered_set<std::wstring>());

private:
  const std::shared_ptr<AtomicInteger> copyingCount =
      std::make_shared<AtomicInteger>();

public:
  PrimaryNode(std::shared_ptr<IndexWriter> writer, int id, int64_t primaryGen,
              int64_t forcePrimaryVersion,
              std::shared_ptr<SearcherFactory> searcherFactory,
              std::shared_ptr<PrintStream> printStream) ;

  /** Returns the current primary generation, which is incremented each time a
   * new primary is started for this index */
  virtual int64_t getPrimaryGen();

  // TODO: in the future, we should separate "flush" (returns an incRef'd
  // SegmentInfos) from "refresh" (open new NRT reader from IndexWriter) so that
  // the latter can be done concurrently while copying files out to replicas,
  // minimizing the refresh time from the replicas.  But fixing this is tricky
  // because e.g. IndexWriter may complete a big merge just after returning the
  // incRef'd SegmentInfos and before we can open a new reader causing us to
  // close the just-merged readers only to then open them again from the (now
  // stale) SegmentInfos.  To fix this "properly" I think IW.inc/decRefDeleter
  // must also incread the ReaderPool entry

  /** Flush all index operations to disk and opens a new near-real-time reader.
   *  new NRT point, to make the changes visible to searching.  Returns true if
   * there were changes. */
  virtual bool flushAndRefresh() ;

  virtual int64_t getCopyStateVersion();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t getLastCommitVersion();

  void commit()  override;

  /** IncRef the current CopyState and return it */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<CopyState> getCopyState() ;

  /** Called once replica is done (or failed) copying an NRT point */
  virtual void
  releaseCopyState(std::shared_ptr<CopyState> copyState) ;

  bool isClosed() override;

  virtual bool isClosed(bool allowClosing);

private:
  void ensureOpen(bool allowClosing);

  /** Steals incoming infos refCount; returns true if there were changes. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool
  setCurrentInfos(std::shared_ptr<Set<std::wstring>> completedMergeFiles) throw(
      IOException);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void waitForAllRemotesToClose() ;

public:
  virtual ~PrimaryNode();

  /** Called when a merge has finished, but before IW switches to the merged
   * segment */
protected:
  virtual void preCopyMergedSegmentFiles(
      std::shared_ptr<SegmentCommitInfo> info,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &files) = 0;

protected:
  std::shared_ptr<PrimaryNode> shared_from_this()
  {
    return std::static_pointer_cast<PrimaryNode>(Node::shared_from_this());
  }
};

} // namespace org::apache::lucene::replicator::nrt
