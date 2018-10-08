#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class RefCount;
}

namespace org::apache::lucene::index
{
class CommitPoint;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexDeletionPolicy;
}
namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::store
{
class AlreadyClosedException;
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
namespace org::apache::lucene::index
{

using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using InfoStream = org::apache::lucene::util::InfoStream;

/*
 * This class keeps track of each SegmentInfos instance that
 * is still "live", either because it corresponds to a
 * segments_N file in the Directory (a "commit", i.e. a
 * committed SegmentInfos) or because it's an in-memory
 * SegmentInfos that a writer is actively updating but has
 * not yet committed.  This class uses simple reference
 * counting to map_obj the live SegmentInfos instances to
 * individual files in the Directory.
 *
 * The same directory file may be referenced by more than
 * one IndexCommit, i.e. more than one SegmentInfos.
 * Therefore we count how many commits reference each file.
 * When all the commits referencing a certain file have been
 * deleted, the refcount for that file becomes zero, and the
 * file is deleted.
 *
 * A separate deletion policy interface
 * (IndexDeletionPolicy) is consulted on creation (onInit)
 * and once per commit (onCommit), to decide when a commit
 * should be removed.
 *
 * It is the business of the IndexDeletionPolicy to choose
 * when to delete commit points.  The actual mechanics of
 * file deletion, retrying, etc, derived from the deletion
 * of commit points is the business of the IndexFileDeleter.
 *
 * The current default deletion policy is {@link
 * KeepOnlyLastCommitDeletionPolicy}, which removes all
 * prior commits when a new commit has completed.  This
 * matches the behavior before 2.2.
 *
 * Note that you must hold the write.lock before
 * instantiating this class.  It opens segments_N file(s)
 * directly with no retry logic.
 */

class IndexFileDeleter final
    : public std::enable_shared_from_this<IndexFileDeleter>
{
  GET_CLASS_NAME(IndexFileDeleter)

  /* Reference count for all files in the index.
   * Counts how many existing commits reference a file.
   **/
private:
  std::unordered_map<std::wstring, std::shared_ptr<RefCount>> refCounts =
      std::unordered_map<std::wstring, std::shared_ptr<RefCount>>();

  /* Holds all commits (segments_N) currently in the index.
   * This will have just 1 commit if you are using the
   * default delete policy (KeepOnlyLastCommitDeletionPolicy).
   * Other policies may leave commit points live for longer
   * in which case this deque would be longer than 1: */
  std::deque<std::shared_ptr<CommitPoint>> commits =
      std::deque<std::shared_ptr<CommitPoint>>();

  /* Holds files we had incref'd from the previous
   * non-commit checkpoint: */
  const std::deque<std::wstring> lastFiles = std::deque<std::wstring>();

  /* Commits that the IndexDeletionPolicy have decided to delete: */
  std::deque<std::shared_ptr<CommitPoint>> commitsToDelete =
      std::deque<std::shared_ptr<CommitPoint>>();

  const std::shared_ptr<InfoStream> infoStream;
  const std::shared_ptr<Directory> directoryOrig; // for commit point metadata
  const std::shared_ptr<Directory> directory;
  const std::shared_ptr<IndexDeletionPolicy> policy;

public:
  const bool startingCommitDeleted;

private:
  std::shared_ptr<SegmentInfos> lastSegmentInfos;

  /** Change to true to see details of reference counts when
   *  infoStream is enabled */
public:
  static bool VERBOSE_REF_COUNTS;

private:
  const std::shared_ptr<IndexWriter> writer;

  // called only from assert
  bool locked();

  /**
   * Initialize the deleter: find all previous commits in
   * the Directory, incref the files they reference, call
   * the policy to let it delete commits.  This will remove
   * any files not referenced by any of the commits.
   * @throws IOException if there is a low-level IO error
   */
public:
  IndexFileDeleter(std::deque<std::wstring> &files,
                   std::shared_ptr<Directory> directoryOrig,
                   std::shared_ptr<Directory> directory,
                   std::shared_ptr<IndexDeletionPolicy> policy,
                   std::shared_ptr<SegmentInfos> segmentInfos,
                   std::shared_ptr<InfoStream> infoStream,
                   std::shared_ptr<IndexWriter> writer, bool initialIndexExists,
                   bool isReaderInit) ;

  /** Set all gens beyond what we currently see in the directory, to avoid
   * double-write in cases where the previous IndexWriter did not gracefully
   * close/rollback (e.g. os/machine crashed or lost power). */
  static void inflateGens(std::shared_ptr<SegmentInfos> infos,
                          std::shared_ptr<std::deque<std::wstring>> files,
                          std::shared_ptr<InfoStream> infoStream);

  void ensureOpen() ;

  // for testing
  bool isClosed();

  /**
   * Remove the CommitPoints in the commitsToDelete List by
   * DecRef'ing all files from each SegmentInfos.
   */
private:
  void deleteCommits() ;

  /**
   * Writer calls this when it has hit an error and had to
   * roll back, to tell us that there may now be
   * unreferenced files in the filesystem.  So we re-deque
   * the filesystem and delete such files.  If segmentName
   * is non-null, we will only delete files corresponding to
   * that segment.
   */
public:
  void refresh() ;

  virtual ~IndexFileDeleter();

  /**
   * Revisits the {@link IndexDeletionPolicy} by calling its
   * {@link IndexDeletionPolicy#onCommit(List)} again with the known commits.
   * This is useful in cases where a deletion policy which holds onto index
   * commits is used. The application may know that some commits are not held by
   * the deletion policy anymore and call
   * {@link IndexWriter#deleteUnusedFiles()}, which will attempt to delete the
   * unused commits again.
   */
  void revisitPolicy() ;

  /**
   * For definition of "check point" see IndexWriter comments:
   * "Clarification: Check Points (and commits)".
   *
   * Writer calls this when it has made a "consistent
   * change" to the index, meaning new files are written to
   * the index and the in-memory SegmentInfos have been
   * modified to point to those files.
   *
   * This may or may not be a commit (segments_N may or may
   * not have been written).
   *
   * We simply incref the files referenced by the new
   * SegmentInfos and decref the files we had previously
   * seen (if any).
   *
   * If this is a commit, we also call the policy to give it
   * a chance to remove other commits.  If any commits are
   * removed, we decref their files as well.
   */
  void checkpoint(std::shared_ptr<SegmentInfos> segmentInfos,
                  bool isCommit) ;

  void incRef(std::shared_ptr<SegmentInfos> segmentInfos,
              bool isCommit) ;

  void incRef(std::shared_ptr<std::deque<std::wstring>> files);

  void incRef(const std::wstring &fileName);

  /** Decrefs all provided files, even on exception; throws first exception hit,
   * if any. */
  void
  decRef(std::shared_ptr<std::deque<std::wstring>> files) ;

  /** Returns true if the file should now be deleted. */
private:
  bool decRef(const std::wstring &fileName);

public:
  void decRef(std::shared_ptr<SegmentInfos> segmentInfos) ;

  bool exists(const std::wstring &fileName);

private:
  std::shared_ptr<RefCount> getRefCount(const std::wstring &fileName);

  /** Deletes the specified files, but only if they are new
   *  (have not yet been incref'd). */
public:
  void deleteNewFiles(std::shared_ptr<std::deque<std::wstring>> files) throw(
      IOException);

private:
  void deleteFiles(std::shared_ptr<std::deque<std::wstring>> names) throw(
      IOException);

  void deleteFile(const std::wstring &fileName) ;

  /**
   * Tracks the reference count for a single index file:
   */
private:
  class RefCount final : public std::enable_shared_from_this<RefCount>
  {
    GET_CLASS_NAME(RefCount)

    // fileName used only for better assert error messages
  public:
    const std::wstring fileName;
    bool initDone = false;
    RefCount(const std::wstring &fileName);

    int count = 0;

    int IncRef();

    int DecRef();
  };

  /**
   * Holds details for each commit point.  This class is
   * also passed to the deletion policy.  Note: this class
   * has a natural ordering that is inconsistent with
   * equals.
   */

private:
  class CommitPoint final : public IndexCommit
  {
    GET_CLASS_NAME(CommitPoint)

  public:
    std::shared_ptr<std::deque<std::wstring>> files;
    std::wstring segmentsFileName;
    bool deleted = false;
    std::shared_ptr<Directory> directoryOrig;
    std::shared_ptr<std::deque<std::shared_ptr<CommitPoint>>> commitsToDelete;
    int64_t generation = 0;
    const std::unordered_map<std::wstring, std::wstring> userData;

  private:
    const int segmentCount;

  public:
    CommitPoint(std::shared_ptr<std::deque<std::shared_ptr<CommitPoint>>>
                    commitsToDelete,
                std::shared_ptr<Directory> directoryOrig,
                std::shared_ptr<SegmentInfos> segmentInfos) ;

    virtual std::wstring toString();

    int getSegmentCount() override;

    std::wstring getSegmentsFileName() override;

    std::shared_ptr<std::deque<std::wstring>> getFileNames() override;

    std::shared_ptr<Directory> getDirectory() override;

    int64_t getGeneration() override;

    std::unordered_map<std::wstring, std::wstring> getUserData() override;

    /**
     * Called only be the deletion policy, to remove this
     * commit point from the index.
     */
    void delete_() override;

    bool isDeleted() override;

  protected:
    std::shared_ptr<CommitPoint> shared_from_this()
    {
      return std::static_pointer_cast<CommitPoint>(
          IndexCommit::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
