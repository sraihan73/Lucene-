#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/StandardDirectoryReader.h"

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
 * <p>Expert: represents a single commit into an index as seen by the
 * {@link IndexDeletionPolicy} or {@link IndexReader}.</p>
 *
 * <p> Changes to the content of an index are made visible
 * only after the writer who made that change commits by
 * writing a new segments file
 * (<code>segments_N</code>). This point in time, when the
 * action of writing of a new segments file to the directory
 * is completed, is an index commit.</p>
 *
 * <p>Each index commit point has a unique segments file
 * associated with it. The segments file associated with a
 * later index commit point would have a larger N.</p>
 *
 * @lucene.experimental
 */

// TODO: this is now a poor name, because this class also represents a
// point-in-time view from an NRT reader
class IndexCommit : public std::enable_shared_from_this<IndexCommit>,
                    public Comparable<std::shared_ptr<IndexCommit>>
{
  GET_CLASS_NAME(IndexCommit)

  /**
   * Get the segments file (<code>segments_N</code>) associated
   * with this commit point.
   */
public:
  virtual std::wstring getSegmentsFileName() = 0;

  /**
   * Returns all index files referenced by this commit point.
   */
  virtual std::shared_ptr<std::deque<std::wstring>> getFileNames() = 0;

  /**
   * Returns the {@link Directory} for the index.
   */
  virtual std::shared_ptr<Directory> getDirectory() = 0;

  /**
   * Delete this commit point.  This only applies when using
   * the commit point in the context of IndexWriter's
   * IndexDeletionPolicy.
   * <p>
   * Upon calling this, the writer is notified that this commit
   * point should be deleted.
   * <p>
   * Decision that a commit-point should be deleted is taken by the {@link
   * IndexDeletionPolicy} in effect and therefore this should only be called by
   * its {@link IndexDeletionPolicy#onInit onInit()} or
   * {@link IndexDeletionPolicy#onCommit onCommit()} methods.
   */
  virtual void delete_() = 0;

  /** Returns true if this commit should be deleted; this is
   *  only used by {@link IndexWriter} after invoking the
   *  {@link IndexDeletionPolicy}. */
  virtual bool isDeleted() = 0;

  /** Returns number of segments referenced by this commit. */
  virtual int getSegmentCount() = 0;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  IndexCommit();

  /** Two IndexCommits are equal if both their Directory and versions are equal.
   */
public:
  bool equals(std::any other) override;

  virtual int hashCode();

  /** Returns the generation (the _N in segments_N) for this
   *  IndexCommit */
  virtual int64_t getGeneration() = 0;

  /** Returns userData, previously passed to {@link
   *  IndexWriter#setLiveCommitData(Iterable)} for this commit.  Map is
   *  {@code std::wstring -> std::wstring}. */
  virtual std::unordered_map<std::wstring, std::wstring> getUserData() = 0;

  int compareTo(std::shared_ptr<IndexCommit> commit) override;

  /** Package-private API for IndexWriter to init from a commit-point pulled
   * from an NRT or non-NRT reader. */
  virtual std::shared_ptr<StandardDirectoryReader> getReader();
};

} // #include  "core/src/java/org/apache/lucene/index/
