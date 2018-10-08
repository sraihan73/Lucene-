#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class IndexCommit;
}
namespace org::apache::lucene::index
{
class SnapshotDeletionPolicy;
}
namespace org::apache::lucene::replicator
{
class RevisionFile;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::replicator
{
class Revision;
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
namespace org::apache::lucene::replicator
{

using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;

/**
 * A {@link Revision} of a single index files which comprises the deque of files
 * that are part of the current {@link IndexCommit}. To ensure the files are not
 * deleted by {@link IndexWriter} for as long as this revision stays alive (i.e.
 * until {@link #release()}), the current commit point is snapshotted, using
 * {@link SnapshotDeletionPolicy} (this means that the given writer's
 * {@link IndexWriterConfig#getIndexDeletionPolicy() config} should return
 * {@link SnapshotDeletionPolicy}).
 * <p>
 * When this revision is {@link #release() released}, it releases the obtained
 * snapshot as well as calls {@link IndexWriter#deleteUnusedFiles()} so that the
 * snapshotted files are deleted (if they are no longer needed).
 *
 * @lucene.experimental
 */
class IndexRevision : public std::enable_shared_from_this<IndexRevision>,
                      public Revision
{
  GET_CLASS_NAME(IndexRevision)

private:
  static constexpr int RADIX = 16;
  static const std::wstring SOURCE;

  const std::shared_ptr<IndexWriter> writer;
  const std::shared_ptr<IndexCommit> commit;
  const std::shared_ptr<SnapshotDeletionPolicy> sdp;
  const std::wstring version;
  const std::unordered_map<std::wstring,
                           std::deque<std::shared_ptr<RevisionFile>>>
      sourceFiles;

  // returns a RevisionFile with some metadata
  static std::shared_ptr<RevisionFile>
  newRevisionFile(const std::wstring &file,
                  std::shared_ptr<Directory> dir) ;

  /** Returns a singleton map_obj of the revision files from the given {@link
   * IndexCommit}. */
public:
  static std::unordered_map<std::wstring,
                            std::deque<std::shared_ptr<RevisionFile>>>
  revisionFiles(std::shared_ptr<IndexCommit> commit) ;

  /**
   * Returns a std::wstring representation of a revision's version from the given
   * {@link IndexCommit}.
   */
  static std::wstring revisionVersion(std::shared_ptr<IndexCommit> commit);

  /**
   * Constructor over the given {@link IndexWriter}. Uses the last
   * {@link IndexCommit} found in the {@link Directory} managed by the given
   * writer.
   */
  IndexRevision(std::shared_ptr<IndexWriter> writer) ;

  int compareTo(const std::wstring &version) override;

  int compareTo(std::shared_ptr<Revision> o) override;

  std::wstring getVersion() override;

  std::unordered_map<std::wstring, std::deque<std::shared_ptr<RevisionFile>>>
  getSourceFiles() override;

  std::shared_ptr<InputStream>
  open(const std::wstring &source,
       const std::wstring &fileName)  override;

  void release()  override;

  virtual std::wstring toString();
};

} // namespace org::apache::lucene::replicator
