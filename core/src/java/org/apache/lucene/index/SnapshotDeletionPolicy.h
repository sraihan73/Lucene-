#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexCommit.h"

#include  "core/src/java/org/apache/lucene/index/IndexDeletionPolicy.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

using Directory = org::apache::lucene::store::Directory;

/**
 * An {@link IndexDeletionPolicy} that wraps any other
 * {@link IndexDeletionPolicy} and adds the ability to hold and later release
 * snapshots of an index. While a snapshot is held, the {@link IndexWriter} will
 * not remove any files associated with it even if the index is otherwise being
 * actively, arbitrarily changed. Because we wrap another arbitrary
 * {@link IndexDeletionPolicy}, this gives you the freedom to continue using
 * whatever {@link IndexDeletionPolicy} you would normally want to use with your
 * index.
 *
 * <p>
 * This class maintains all snapshots in-memory, and so the information is not
 * persisted and not protected against system failures. If persistence is
 * important, you can use {@link PersistentSnapshotDeletionPolicy}.
 *
 * @lucene.experimental
 */
class SnapshotDeletionPolicy : public IndexDeletionPolicy
{
  GET_CLASS_NAME(SnapshotDeletionPolicy)

  /** Records how many snapshots are held against each
   *  commit generation */
protected:
  const std::unordered_map<int64_t, int> refCounts =
      std::unordered_map<int64_t, int>();

  /** Used to map_obj gen to IndexCommit. */
  const std::unordered_map<int64_t, std::shared_ptr<IndexCommit>>
      indexCommits =
          std::unordered_map<int64_t, std::shared_ptr<IndexCommit>>();

  /** Wrapped {@link IndexDeletionPolicy} */
private:
  const std::shared_ptr<IndexDeletionPolicy> primary;

  /** Most recently committed {@link IndexCommit}. */
protected:
  std::shared_ptr<IndexCommit> lastCommit;

  /** Used to detect misuse */
private:
  bool initCalled = false;

  /** Sole constructor, taking the incoming {@link
   *  IndexDeletionPolicy} to wrap. */
public:
  SnapshotDeletionPolicy(std::shared_ptr<IndexDeletionPolicy> primary);

  template <typename T1>
  // C++ WARNING: The following method was originally marked 'synchronized':
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public synchronized void
  // onCommit(java.util.List<? extends IndexCommit> commits) throws
  // java.io.IOException
  void onCommit(std::deque<T1> commits) ;

  template <typename T1>
  // C++ WARNING: The following method was originally marked 'synchronized':
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: @Override public synchronized void
  // onInit(java.util.List<? extends IndexCommit> commits) throws
  // java.io.IOException
  void onInit(std::deque<T1> commits) ;

  /**
   * Release a snapshotted commit.
   *
   * @param commit
   *          the commit previously returned by {@link #snapshot}
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void release(std::shared_ptr<IndexCommit> commit) ;

  /** Release a snapshot by generation. */
protected:
  virtual void releaseGen(int64_t gen) ;

  /** Increments the refCount for this {@link IndexCommit}. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void incRef(std::shared_ptr<IndexCommit> ic);

  /**
   * Snapshots the last commit and returns it. Once a commit is 'snapshotted,'
   * it is protected from deletion (as long as this {@link IndexDeletionPolicy}
   * is used). The snapshot can be removed by calling {@link
   * #release(IndexCommit)} followed by a call to {@link
   * IndexWriter#deleteUnusedFiles()}.
   *
   * <p>
   * <b>NOTE:</b> while the snapshot is held, the files it references will not
   * be deleted, which will consume additional disk space in your index. If you
   * take a snapshot at a particularly bad time (say just before you call
   * forceMerge) then in the worst case this could consume an extra 1X of your
   * total index size, until you release the snapshot.
   *
   * @throws IllegalStateException
   *           if this index does not have any commits yet
   * @return the {@link IndexCommit} that was snapshotted.
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<IndexCommit> snapshot() ;

  /** Returns all IndexCommits held by at least one snapshot. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::deque<std::shared_ptr<IndexCommit>> getSnapshots();

  /** Returns the total number of snapshots currently held. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int getSnapshotCount();

  /** Retrieve an {@link IndexCommit} from its generation;
   *  returns null if this IndexCommit is not currently
   *  snapshotted  */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<IndexCommit> getIndexCommit(int64_t gen);

  /** Wraps each {@link IndexCommit} as a {@link
   *  SnapshotCommitPoint}. */
private:
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: private java.util.List<IndexCommit>
  // wrapCommits(java.util.List<? extends IndexCommit> commits)
  std::deque<std::shared_ptr<IndexCommit>>
  wrapCommits(std::deque<T1> commits);

  /** Wraps a provided {@link IndexCommit} and prevents it
   *  from being deleted. */
private:
  class SnapshotCommitPoint : public IndexCommit
  {
    GET_CLASS_NAME(SnapshotCommitPoint)
  private:
    std::shared_ptr<SnapshotDeletionPolicy> outerInstance;

    /** The {@link IndexCommit} we are preventing from deletion. */
  protected:
    std::shared_ptr<IndexCommit> cp;

    /** Creates a {@code SnapshotCommitPoint} wrapping the provided
     *  {@link IndexCommit}. */
    SnapshotCommitPoint(std::shared_ptr<SnapshotDeletionPolicy> outerInstance,
                        std::shared_ptr<IndexCommit> cp);

  public:
    virtual std::wstring toString();

    void delete_() override;

    std::shared_ptr<Directory> getDirectory() override;

    std::shared_ptr<std::deque<std::wstring>>
    getFileNames()  override;

    int64_t getGeneration() override;

    std::wstring getSegmentsFileName() override;

    std::unordered_map<std::wstring, std::wstring>
    getUserData()  override;

    bool isDeleted() override;

    int getSegmentCount() override;

  protected:
    std::shared_ptr<SnapshotCommitPoint> shared_from_this()
    {
      return std::static_pointer_cast<SnapshotCommitPoint>(
          IndexCommit::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SnapshotDeletionPolicy> shared_from_this()
  {
    return std::static_pointer_cast<SnapshotDeletionPolicy>(
        IndexDeletionPolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
