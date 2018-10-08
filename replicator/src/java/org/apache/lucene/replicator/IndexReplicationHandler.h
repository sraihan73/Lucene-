#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/replicator/RevisionFile.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/IndexCommit.h"

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

using IndexCommit = org::apache::lucene::index::IndexCommit;
using ReplicationHandler =
    org::apache::lucene::replicator::ReplicationClient::ReplicationHandler;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * A {@link ReplicationHandler} for replication of an index. Implements
 * {@link #revisionReady} by copying the files pointed by the client resolver to
 * the index {@link Directory} and then touches the index with
 * {@link IndexWriter} to make sure any unused files are deleted.
 * <p>
 * <b>NOTE:</b> this handler assumes that {@link IndexWriter} is not opened by
 * another process on the index directory. In fact, opening an
 * {@link IndexWriter} on the same directory to which files are copied can lead
 * to undefined behavior, where some or all the files will be deleted, override
 * other files or simply create a mess. When you replicate an index, it is best
 * if the index is never modified by {@link IndexWriter}, except the one that is
 * open on the source index, from which you replicate.
 * <p>
 * This handler notifies the application via a provided {@link Callable} when an
 * updated index commit was made available for it.
 *
 * @lucene.experimental
 */
class IndexReplicationHandler
    : public std::enable_shared_from_this<IndexReplicationHandler>,
      public ReplicationHandler
{
  GET_CLASS_NAME(IndexReplicationHandler)

  /**
   * The component used to log messages to the {@link InfoStream#getDefault()
   * default} {@link InfoStream}.
   */
public:
  static const std::wstring INFO_STREAM_COMPONENT;

private:
  const std::shared_ptr<Directory> indexDir;
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
   * Returns the last {@link IndexCommit} found in the {@link Directory}, or
   * {@code null} if there are no commits.
   */
public:
  static std::shared_ptr<IndexCommit>
  getLastCommit(std::shared_ptr<Directory> dir) ;

  /**
   * Verifies that the last file is segments_N and fails otherwise. It also
   * removes and returns the file from the deque, because it needs to be handled
   * last, after all files. This is important in order to guarantee that if a
   * reader sees the new segments_N, all other segment files are already on
   * stable storage.
   * <p>
   * The reason why the code fails instead of putting segments_N file last is
   * that this indicates an error in the Revision implementation.
   */
  static std::wstring getSegmentsFile(std::deque<std::wstring> &files,
                                      bool allowEmpty);

  /**
   * Cleanup the index directory by deleting all given files. Called when file
   * copy or sync failed.
   */
  static void cleanupFilesOnFailure(std::shared_ptr<Directory> dir,
                                    std::deque<std::wstring> &files);

  /**
   * Cleans up the index directory from old index files. This method uses the
   * last commit found by {@link #getLastCommit(Directory)}. If it matches the
   * expected segmentsFile, then all files not referenced by this commit point
   * are deleted.
   * <p>
   * <b>NOTE:</b> this method does a best effort attempt to clean the index
   * directory. It suppresses any exceptions that occur, as this can be retried
   * the next time.
   */
  static void cleanupOldIndexFiles(std::shared_ptr<Directory> dir,
                                   const std::wstring &segmentsFile,
                                   std::shared_ptr<InfoStream> infoStream);

  /**
   * Copies the files from the source directory to the target one, if they are
   * not the same.
   */
  static void copyFiles(std::shared_ptr<Directory> source,
                        std::shared_ptr<Directory> target,
                        std::deque<std::wstring> &files) ;

  /**
   * Constructor with the given index directory and callback to notify when the
   * indexes were updated.
   */
  IndexReplicationHandler(
      std::shared_ptr<Directory> indexDir,
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
