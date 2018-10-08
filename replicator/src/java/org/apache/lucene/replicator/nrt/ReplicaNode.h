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
#include  "core/src/java/org/apache/lucene/replicator/nrt/ReplicaFileDeleter.h"

#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyJob.h"
#include  "core/src/java/org/apache/lucene/store/Lock.h"
#include  "core/src/java/org/apache/lucene/search/SearcherFactory.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/OnceDone.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
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
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Lock = org::apache::lucene::store::Lock;

/** Replica node, that pulls index changes from the primary node by copying
 * newly flushed or merged index files.
 *
 *  @lucene.experimental */

class ReplicaNode : public Node
{
  GET_CLASS_NAME(ReplicaNode)

public:
  std::shared_ptr<ReplicaFileDeleter> deleter;

  /** IncRef'd files in the current commit point: */
private:
  const std::shared_ptr<std::deque<std::wstring>> lastCommitFiles =
      std::unordered_set<std::wstring>();

  /** IncRef'd files in the current NRT point: */
protected:
  const std::shared_ptr<std::deque<std::wstring>> lastNRTFiles =
      std::unordered_set<std::wstring>();

  /** Currently running merge pre-copy jobs */
  const std::shared_ptr<Set<std::shared_ptr<CopyJob>>> mergeCopyJobs =
      Collections::synchronizedSet(
          std::unordered_set<std::shared_ptr<CopyJob>>());

  /** Non-null when we are currently copying files from a new NRT point: */
  std::shared_ptr<CopyJob> curNRTCopy;

  /** We hold this to ensure an external IndexWriter cannot also open on our
   * directory: */
private:
  const std::shared_ptr<Lock> writeFileLock;

  /** Merged segment files that we pre-copied, but have not yet made visible in
   * a new NRT point. */
public:
  const std::shared_ptr<Set<std::wstring>> pendingMergeFiles =
      Collections::synchronizedSet(std::unordered_set<std::wstring>());

  /** Primary gen last time we successfully replicated: */
protected:
  int64_t lastPrimaryGen = 0;

public:
  ReplicaNode(int id, std::shared_ptr<Directory> dir,
              std::shared_ptr<SearcherFactory> searcherFactory,
              std::shared_ptr<PrintStream> printStream) ;

  /** Start up this replica, which possibly requires heavy copying of files from
   * the primary node, if we were down for a long time */
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void start(int64_t curPrimaryGen) ;

  std::mutex commitLock;

public:
  void commit()  override;

protected:
  virtual void finishNRTCopy(std::shared_ptr<CopyJob> job,
                             int64_t startNS) ;

  /** Start a background copying job, to copy the specified files from the
   * current primary node.  If files is null then the latest copy state should
   * be copied.  If prevJob is not null, then the new copy job is replacing it
   * and should 1) cancel the previous one, and 2) optionally salvage e.g.
   * partially copied and, shared with the new copy job, files. */
  virtual std::shared_ptr<CopyJob> newCopyJob(
      const std::wstring &reason,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &prevFiles,
      bool highPriority, std::shared_ptr<CopyJob::OnceDone> onceDone) = 0;

  /** Runs this job async'd */
  virtual void launch(std::shared_ptr<CopyJob> job) = 0;

  /** Tell primary we (replica) just started, so primary can tell us to warm any
   * already warming merges.  This lets us keep low nrt refresh time for the
   * first nrt sync after we started. */
  virtual void sendNewReplica() = 0;

  /** Call this to notify this replica node that a new NRT infos is available on
   * the primary. We kick off a job (runs in the background) to copy files
   * across, and open a new reader once that's done. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<CopyJob>
  newNRTPoint(int64_t newPrimaryGen, int64_t version) ;

private:
  class OnceDoneAnonymousInnerClass
      : public std::enable_shared_from_this<OnceDoneAnonymousInnerClass>,
        public CopyJob::OnceDone
  {
    GET_CLASS_NAME(OnceDoneAnonymousInnerClass)
  private:
    std::shared_ptr<ReplicaNode> outerInstance;

    int64_t startNS = 0;
    std::shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job;

  public:
    OnceDoneAnonymousInnerClass(
        std::shared_ptr<ReplicaNode> outerInstance, int64_t startNS,
        std::shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job);

    void run(std::shared_ptr<CopyJob> job) override;
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool isCopying();

  bool isClosed() override;

  virtual ~ReplicaNode();

  /** Called when the primary changed */
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void maybeNewPrimary(int64_t newPrimaryGen) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<CopyJob> launchPreCopyMerge(
      std::shared_ptr<AtomicBoolean> finished, int64_t newPrimaryGen,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
          &files) ;

private:
  class OnceDoneAnonymousInnerClass2
      : public std::enable_shared_from_this<OnceDoneAnonymousInnerClass2>,
        public CopyJob::OnceDone
  {
    GET_CLASS_NAME(OnceDoneAnonymousInnerClass2)
  private:
    std::shared_ptr<ReplicaNode> outerInstance;

    std::shared_ptr<AtomicBoolean> finished;
    std::shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job;
    int64_t primaryGenStart = 0;
    std::shared_ptr<Set<std::wstring>> fileNames;

  public:
    OnceDoneAnonymousInnerClass2(
        std::shared_ptr<ReplicaNode> outerInstance,
        std::shared_ptr<AtomicBoolean> finished,
        std::shared_ptr<org::apache::lucene::replicator::nrt::CopyJob> job,
        int64_t primaryGenStart,
        std::shared_ptr<Set<std::wstring>> fileNames);

    void run(std::shared_ptr<CopyJob> job)  override;
  };

public:
  virtual std::shared_ptr<IndexOutput>
  createTempOutput(const std::wstring &prefix, const std::wstring &suffix,
                   std::shared_ptr<IOContext> ioContext) ;

  /** Compares incoming per-file identity (id, checksum, header, footer) versus
   * what we have locally and returns the subset of the incoming
   *  files that need copying */
  virtual std::deque<
      std::unordered_map::Entry<std::wstring, std::shared_ptr<FileMetaData>>>
  getFilesToCopy(std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
                     &files) ;

  /** Carefully determine if the file on the primary, identified by its {@code
   * std::wstring fileName} along with the {@link FileMetaData} "summarizing" its
   * contents, is precisely the same file that we have locally.  If the file
   * does not exist locally, or if its its header (inclues the segment id),
   * length, footer (including checksum) differ, then this returns false, else
   * true. */
private:
  bool
  fileIsIdentical(const std::wstring &fileName,
                  std::shared_ptr<FileMetaData> srcMetaData) ;

  std::shared_ptr<ConcurrentMap<std::wstring, bool>> copying =
      std::make_shared<ConcurrentHashMap<std::wstring, bool>>();

  // Used only to catch bugs, ensuring a given file name is only ever being
  // copied bye one job:
public:
  virtual void startCopyFile(const std::wstring &name);

  virtual void finishCopyFile(const std::wstring &name);

protected:
  std::shared_ptr<ReplicaNode> shared_from_this()
  {
    return std::static_pointer_cast<ReplicaNode>(Node::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
