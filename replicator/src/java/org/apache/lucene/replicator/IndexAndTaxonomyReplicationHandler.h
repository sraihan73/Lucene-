#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/replicator/RevisionFile.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"

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
namespace org::apache::lucene::replicator
{

using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * A {@link ReplicationHandler} for replication of an index and taxonomy pair.
 * See {@link IndexReplicationHandler} for more detail. This handler ensures
 * that the search and taxonomy indexes are replicated in a consistent way.
 * <p>
 * <b>NOTE:</b> if you intend to recreate a taxonomy index, you should make sure
 * to reopen an IndexSearcher and TaxonomyReader pair via the provided callback,
 * to guarantee that both indexes are in sync. This handler does not prevent
 * replicating such index and taxonomy pairs, and if they are reopened by a
 * different thread, unexpected errors can occur, as well as inconsistency
 * between the taxonomy and index readers.
 *
 * @see IndexReplicationHandler
 *
 * @lucene.experimental
 */
class IndexAndTaxonomyReplicationHandler
    : public std::enable_shared_from_this<IndexAndTaxonomyReplicationHandler>,
      public ReplicationHandler
{
  GET_CLASS_NAME(IndexAndTaxonomyReplicationHandler)

  /**
   * The component used to log messages to the {@link InfoStream#getDefault()
   * default} {@link InfoStream}.
   */
public:
  static const std::wstring INFO_STREAM_COMPONENT;

private:
  const std::shared_ptr<Directory> indexDir;
  const std::shared_ptr<Directory> taxoDir;
  const std::shared_ptr<Callable<bool>> callback;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile
  // java.util.Map<std::wstring,java.util.List<RevisionFile>> currentRevisionFiles; C++
  // NOTE: Fields cannot have the same name as methods:
  std::unordered_map<std::wstring, std::deque<std::shared_ptr<RevisionFile>>>
      currentRevisionFiles_;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile std::wstring currentVersion;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::wstring currentVersion_;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile org.apache.lucene.util.InfoStream
  // infoStream = org.apache.lucene.util.InfoStream.getDefault();
  std::shared_ptr<InfoStream> infoStream = InfoStream::getDefault();

  /**
   * Constructor with the given index directory and callback to notify when the
   * indexes were updated.
   */
public:
  IndexAndTaxonomyReplicationHandler(
      std::shared_ptr<Directory> indexDir, std::shared_ptr<Directory> taxoDir,
      std::shared_ptr<Callable<bool>> callback) ;

  std::wstring currentVersion() override;

  std::unordered_map<std::wstring, std::deque<std::shared_ptr<RevisionFile>>>
  currentRevisionFiles() override;

  void revisionReady(
      const std::wstring &version,
      std::unordered_map<std::wstring,
                         std::deque<std::shared_ptr<RevisionFile>>>
          &revisionFiles,
      std::unordered_map<std::wstring, std::deque<std::wstring>> &copiedFiles,
      std::unordered_map<std::wstring, std::shared_ptr<Directory>>
          &sourceDirectory)  override;

  /** Sets the {@link InfoStream} to use for logging messages. */
  virtual void setInfoStream(std::shared_ptr<InfoStream> infoStream);
};

} // #include  "core/src/java/org/apache/lucene/replicator/
