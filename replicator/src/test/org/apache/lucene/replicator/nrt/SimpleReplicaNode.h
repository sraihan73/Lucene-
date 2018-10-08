#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/nrt/Jobs.h"

#include  "core/src/java/org/apache/lucene/store/RateLimiter.h"
#include  "core/src/java/org/apache/lucene/search/SearcherFactory.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyJob.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/OnceDone.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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

using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RateLimiter = org::apache::lucene::store::RateLimiter;

class SimpleReplicaNode : public ReplicaNode
{
  GET_CLASS_NAME(SimpleReplicaNode)
public:
  const int tcpPort;
  const std::shared_ptr<Jobs> jobs;

  // Rate limits incoming bytes/sec when fetching files:
  const std::shared_ptr<RateLimiter> fetchRateLimiter;
  const std::shared_ptr<AtomicLong> bytesSinceLastRateLimiterCheck =
      std::make_shared<AtomicLong>();
  const std::shared_ptr<Random> random;

  /** Changes over time, as primary node crashes and moves around */
  int curPrimaryTCPPort = 0;

  SimpleReplicaNode(std::shared_ptr<Random> random, int id, int tcpPort,
                    std::shared_ptr<Path> indexPath, int64_t curPrimaryGen,
                    int primaryTCPPort,
                    std::shared_ptr<SearcherFactory> searcherFactory,
                    bool doCheckIndexOnClose) ;

protected:
  void launch(std::shared_ptr<CopyJob> job) override;

public:
  virtual ~SimpleReplicaNode();

protected:
  std::shared_ptr<CopyJob> newCopyJob(
      const std::wstring &reason,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &prevFiles,
      bool highPriority,
      std::shared_ptr<CopyJob::OnceDone> onceDone)  override;

public:
  static std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Random> random, int id,
               std::shared_ptr<Path> path,
               bool doCheckIndexOnClose) ;

  static constexpr char CMD_NEW_NRT_POINT = 0;

  // Sent by primary to replica to pre-copy merge files:
  static constexpr char CMD_PRE_COPY_MERGE = 17;

  /** Handles incoming request to the naive TCP server wrapping this node */
  virtual void handleOneConnection(
      std::shared_ptr<ServerSocket> ss, std::shared_ptr<AtomicBoolean> stop,
      std::shared_ptr<InputStream> is, std::shared_ptr<Socket> socket,
      std::shared_ptr<DataInput> in_, std::shared_ptr<DataOutput> out,
      std::shared_ptr<BufferedOutputStream> bos) throw(IOException,
                                                       InterruptedException);

protected:
  void sendNewReplica()  override;

public:
  std::shared_ptr<IndexOutput> createTempOutput(
      const std::wstring &prefix, const std::wstring &suffix,
      std::shared_ptr<IOContext> ioContext)  override;

protected:
  std::shared_ptr<SimpleReplicaNode> shared_from_this()
  {
    return std::static_pointer_cast<SimpleReplicaNode>(
        ReplicaNode::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
