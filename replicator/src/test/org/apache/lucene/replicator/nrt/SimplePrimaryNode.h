#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/nrt/MergePreCopy.h"

#include  "core/src/java/org/apache/lucene/replicator/nrt/Connection.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
#include  "core/src/java/org/apache/lucene/search/SearcherFactory.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyState.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

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

using FieldType = org::apache::lucene::document::FieldType;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;

/** A primary node that uses simple TCP connections to send commands and copy
 * files */

class SimplePrimaryNode : public PrimaryNode
{
  GET_CLASS_NAME(SimplePrimaryNode)

public:
  const int tcpPort;

  const std::shared_ptr<Random> random;

  // These are updated by parent test process whenever replicas change:
  std::deque<int> replicaTCPPorts = std::deque<int>(0);
  std::deque<int> replicaIDs = std::deque<int>(0);

  // So we only flip a bit once per file name:
  const std::shared_ptr<Set<std::wstring>> bitFlipped =
      Collections::synchronizedSet(std::unordered_set<std::wstring>());

  const std::deque<std::shared_ptr<MergePreCopy>> warmingSegments =
      Collections::synchronizedList(
          std::deque<std::shared_ptr<MergePreCopy>>());

  const bool doFlipBitsDuringCopy;

public:
  class MergePreCopy : public std::enable_shared_from_this<MergePreCopy>
  {
    GET_CLASS_NAME(MergePreCopy)
  public:
    const std::deque<std::shared_ptr<Connection>> connections =
        Collections::synchronizedList(
            std::deque<std::shared_ptr<Connection>>());
    const std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> files;

  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    bool finished_ = false;

  public:
    MergePreCopy(
        std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual bool tryAddConnection(std::shared_ptr<Connection> c);

    // C++ WARNING: The following method was originally marked 'synchronized':
    virtual bool finished();
  };

public:
  SimplePrimaryNode(std::shared_ptr<Random> random,
                    std::shared_ptr<Path> indexPath, int id, int tcpPort,
                    int64_t primaryGen, int64_t forcePrimaryVersion,
                    std::shared_ptr<SearcherFactory> searcherFactory,
                    bool doFlipBitsDuringCopy,
                    bool doCheckIndexOnClose) ;

  /** Records currently alive replicas. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setReplicas(std::deque<int> &replicaIDs,
                           std::deque<int> &replicaTCPPorts);

private:
  static std::shared_ptr<IndexWriter>
  initWriter(int id, std::shared_ptr<Random> random,
             std::shared_ptr<Path> indexPath,
             bool doCheckIndexOnClose) ;

protected:
  void preCopyMergedSegmentFiles(
      std::shared_ptr<SegmentCommitInfo> info,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &files)  override;

  /** Flushes all indexing ops to disk and notifies all replicas that they
   * should now copy */
private:
  void
  handleFlush(std::shared_ptr<DataInput> topIn,
              std::shared_ptr<DataOutput> topOut,
              std::shared_ptr<BufferedOutputStream> bos) ;

  /** Pushes CopyState on the wire */
  static void
  writeCopyState(std::shared_ptr<CopyState> state,
                 std::shared_ptr<DataOutput> out) ;

  /** Called when another node (replica) wants to copy files from us */
  bool handleFetchFiles(
      std::shared_ptr<Random> random, std::shared_ptr<Socket> socket,
      std::shared_ptr<DataInput> destIn, std::shared_ptr<DataOutput> destOut,
      std::shared_ptr<BufferedOutputStream> bos) ;

public:
  static const std::shared_ptr<FieldType> tokenizedWithTermVectors;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static SimplePrimaryNode::StaticConstructor staticConstructor;

  void handleIndexing(
      std::shared_ptr<Socket> socket, std::shared_ptr<AtomicBoolean> stop,
      std::shared_ptr<InputStream> is, std::shared_ptr<DataInput> in_,
      std::shared_ptr<DataOutput> out,
      std::shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                       InterruptedException);

  void handleAddDocument(std::shared_ptr<DataInput> in_,
                         std::shared_ptr<DataOutput> out) ;

  void handleUpdateDocument(std::shared_ptr<DataInput> in_,
                            std::shared_ptr<DataOutput> out) ;

  void handleDeleteDocument(std::shared_ptr<DataInput> in_,
                            std::shared_ptr<DataOutput> out) ;

  // Sent to primary to cutover new SIS:
public:
  static constexpr char CMD_FLUSH = 10;

  // Sent by replica to primary asking to copy a set of files over:
  static constexpr char CMD_FETCH_FILES = 1;
  static constexpr char CMD_GET_SEARCHING_VERSION = 12;
  static constexpr char CMD_SEARCH = 2;
  static constexpr char CMD_MARKER_SEARCH = 3;
  static constexpr char CMD_COMMIT = 4;
  static constexpr char CMD_CLOSE = 5;
  static constexpr char CMD_SEARCH_ALL = 21;

  // Send (to primary) the deque of currently running replicas:
  static constexpr char CMD_SET_REPLICAS = 16;

  // Multiple indexing ops
  static constexpr char CMD_INDEXING = 18;
  static constexpr char CMD_ADD_DOC = 6;
  static constexpr char CMD_UPDATE_DOC = 7;
  static constexpr char CMD_DELETE_DOC = 8;
  static constexpr char CMD_INDEXING_DONE = 19;
  static constexpr char CMD_DELETE_ALL_DOCS = 22;
  static constexpr char CMD_FORCE_MERGE = 23;

  // Sent by replica to primary when replica first starts up, so primary can add
  // it to any warming merges:
  static constexpr char CMD_NEW_REPLICA = 20;

  /** Handles incoming request to the naive TCP server wrapping this node */
  virtual void handleOneConnection(
      std::shared_ptr<Random> random, std::shared_ptr<ServerSocket> ss,
      std::shared_ptr<AtomicBoolean> stop, std::shared_ptr<InputStream> is,
      std::shared_ptr<Socket> socket, std::shared_ptr<DataInput> in_,
      std::shared_ptr<DataOutput> out,
      std::shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                       InterruptedException);

private:
  void
  verifyAtLeastMarkerCount(int expectedAtLeastCount,
                           std::shared_ptr<DataOutput> out) ;

protected:
  std::shared_ptr<SimplePrimaryNode> shared_from_this()
  {
    return std::static_pointer_cast<SimplePrimaryNode>(
        PrimaryNode::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
