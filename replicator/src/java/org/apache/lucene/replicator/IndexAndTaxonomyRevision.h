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
class SnapshotDeletionPolicy;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::facet::taxonomy::writercache
{
class TaxonomyWriterCache;
}
namespace org::apache::lucene::index
{
class IndexCommit;
}
namespace org::apache::lucene::replicator
{
class RevisionFile;
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

using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using SnapshotDeletionPolicy =
    org::apache::lucene::index::SnapshotDeletionPolicy;
using Directory = org::apache::lucene::store::Directory;

/**
 * A {@link Revision} of a single index and taxonomy index files which comprises
 * the deque of files from both indexes. This revision should be used whenever a
 * pair of search and taxonomy indexes need to be replicated together to
 * guarantee consistency of both on the replicating (client) side.
 *
 * @see IndexRevision
 *
 * @lucene.experimental
 */
class IndexAndTaxonomyRevision
    : public std::enable_shared_from_this<IndexAndTaxonomyRevision>,
      public Revision
{
  GET_CLASS_NAME(IndexAndTaxonomyRevision)

  /**
   * A {@link DirectoryTaxonomyWriter} which sets the underlying
   * {@link IndexWriter}'s {@link IndexDeletionPolicy} to
   * {@link SnapshotDeletionPolicy}.
   */
public:
  class SnapshotDirectoryTaxonomyWriter final : public DirectoryTaxonomyWriter
  {
    GET_CLASS_NAME(SnapshotDirectoryTaxonomyWriter)

  private:
    std::shared_ptr<SnapshotDeletionPolicy> sdp;
    std::shared_ptr<IndexWriter> writer;

    /**
     * @see DirectoryTaxonomyWriter#DirectoryTaxonomyWriter(Directory,
     *      IndexWriterConfig.OpenMode, TaxonomyWriterCache)
     */
  public:
    SnapshotDirectoryTaxonomyWriter(
        std::shared_ptr<Directory> directory,
        IndexWriterConfig::OpenMode openMode,
        std::shared_ptr<TaxonomyWriterCache> cache) ;

    /** @see DirectoryTaxonomyWriter#DirectoryTaxonomyWriter(Directory,
     * IndexWriterConfig.OpenMode) */
    SnapshotDirectoryTaxonomyWriter(
        std::shared_ptr<Directory> directory,
        IndexWriterConfig::OpenMode openMode) ;

    /** @see DirectoryTaxonomyWriter#DirectoryTaxonomyWriter(Directory) */
    SnapshotDirectoryTaxonomyWriter(std::shared_ptr<Directory> d) throw(
        IOException);

  protected:
    std::shared_ptr<IndexWriterConfig>
    createIndexWriterConfig(IndexWriterConfig::OpenMode openMode) override;

    std::shared_ptr<IndexWriter> openIndexWriter(
        std::shared_ptr<Directory> directory,
        std::shared_ptr<IndexWriterConfig> config)  override;

    /** Returns the {@link SnapshotDeletionPolicy} used by the underlying {@link
     * IndexWriter}. */
  public:
    std::shared_ptr<SnapshotDeletionPolicy> getDeletionPolicy();

    /** Returns the {@link IndexWriter} used by this {@link
     * DirectoryTaxonomyWriter}. */
    std::shared_ptr<IndexWriter> getIndexWriter();

  protected:
    std::shared_ptr<SnapshotDirectoryTaxonomyWriter> shared_from_this()
    {
      return std::static_pointer_cast<SnapshotDirectoryTaxonomyWriter>(
          org.apache.lucene.facet.taxonomy.directory
              .DirectoryTaxonomyWriter::shared_from_this());
    }
  };

private:
  static constexpr int RADIX = 16;

public:
  static const std::wstring INDEX_SOURCE;
  static const std::wstring TAXONOMY_SOURCE;

private:
  const std::shared_ptr<IndexWriter> indexWriter;
  const std::shared_ptr<SnapshotDirectoryTaxonomyWriter> taxoWriter;
  const std::shared_ptr<IndexCommit> indexCommit, taxoCommit;
  const std::shared_ptr<SnapshotDeletionPolicy> indexSDP, taxoSDP;
  const std::wstring version;
  const std::unordered_map<std::wstring,
                           std::deque<std::shared_ptr<RevisionFile>>>
      sourceFiles;

  /** Returns a singleton map_obj of the revision files from the given {@link
   * IndexCommit}. */
public:
  static std::unordered_map<std::wstring,
                            std::deque<std::shared_ptr<RevisionFile>>>
  revisionFiles(std::shared_ptr<IndexCommit> indexCommit,
                std::shared_ptr<IndexCommit> taxoCommit) ;

  /**
   * Returns a std::wstring representation of a revision's version from the given
   * {@link IndexCommit}s of the search and taxonomy indexes.
   */
  static std::wstring revisionVersion(std::shared_ptr<IndexCommit> indexCommit,
                                      std::shared_ptr<IndexCommit> taxoCommit);

  /**
   * Constructor over the given {@link IndexWriter}. Uses the last
   * {@link IndexCommit} found in the {@link Directory} managed by the given
   * writer.
   */
  IndexAndTaxonomyRevision(std::shared_ptr<IndexWriter> indexWriter,
                           std::shared_ptr<SnapshotDirectoryTaxonomyWriter>
                               taxoWriter) ;

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
