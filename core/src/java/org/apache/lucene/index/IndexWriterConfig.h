#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class IndexDeletionPolicy;
}
namespace org::apache::lucene::index
{
class IndexCommit;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::index
{
class MergeScheduler;
}
namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class DocumentsWriterPerThreadPool;
}
namespace org::apache::lucene::index
{
class FlushPolicy;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::index
{
class IndexReaderWarmer;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::search
{
class Sort;
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Codec = org::apache::lucene::codecs::Codec;
using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using InfoStream = org::apache::lucene::util::InfoStream;
using SetOnce = org::apache::lucene::util::SetOnce;

/**
 * Holds all the configuration that is used to create an {@link IndexWriter}.
 * Once {@link IndexWriter} has been created with this object, changes to this
 * object will not affect the {@link IndexWriter} instance. For that, use
 * {@link LiveIndexWriterConfig} that is returned from {@link
 * IndexWriter#getConfig()}.
 *
 * <p>
 * All setter methods return {@link IndexWriterConfig} to allow chaining
 * settings conveniently, for example:
 *
 * <pre class="prettyprint">
 * IndexWriterConfig conf = new IndexWriterConfig(analyzer);
 * conf.setter1().setter2();
 * </pre>
 *
 * @see IndexWriter#getConfig()
 *
 * @since 3.1
 */
class IndexWriterConfig final : public LiveIndexWriterConfig
{
  GET_CLASS_NAME(IndexWriterConfig)

  /**
   * Specifies the open mode for {@link IndexWriter}.
   */
public:
  enum class OpenMode {
    GET_CLASS_NAME(OpenMode)
    /**
     * Creates a new index or overwrites an existing one.
     */
    CREATE,

    /**
     * Opens an existing index.
     */
    APPEND,

    /**
     * Creates a new index if one does not exist,
     * otherwise it opens the index and documents will be appended.
     */
    CREATE_OR_APPEND
  };

  /** Denotes a flush trigger is disabled. */
public:
  static constexpr int DISABLE_AUTO_FLUSH = -1;

  /** Disabled by default (because IndexWriter flushes by RAM usage by default).
   */
  static constexpr int DEFAULT_MAX_BUFFERED_DELETE_TERMS = DISABLE_AUTO_FLUSH;

  /** Disabled by default (because IndexWriter flushes by RAM usage by default).
   */
  static constexpr int DEFAULT_MAX_BUFFERED_DOCS = DISABLE_AUTO_FLUSH;

  /**
   * Default value is 16 MB (which means flush when buffered docs consume
   * approximately 16 MB RAM).
   */
  static constexpr double DEFAULT_RAM_BUFFER_SIZE_MB = 16.0;

  /** Default setting (true) for {@link #setReaderPooling}. */
  // We changed this default to true with concurrent deletes/updates
  // (LUCENE-7868), because we will otherwise need to open and close segment
  // readers more frequently. False is still supported, but will have worse
  // performance since readers will be forced to aggressively move all state to
  // disk.
  static constexpr bool DEFAULT_READER_POOLING = true;

  /** Default value is 1945. Change using {@link
   * #setRAMPerThreadHardLimitMB(int)} */
  static constexpr int DEFAULT_RAM_PER_THREAD_HARD_LIMIT_MB = 1945;

  /** Default value for compound file system for newly written segments
   *  (set to <code>true</code>). For batch indexing with very large
   *  ram buffers use <code>false</code> */
  static constexpr bool DEFAULT_USE_COMPOUND_FILE_SYSTEM = true;

  /** Default value for whether calls to {@link IndexWriter#close()} include a
   * commit. */
  static constexpr bool DEFAULT_COMMIT_ON_CLOSE = true;

  // indicates whether this config instance is already attached to a writer.
  // not final so that it can be cloned properly.
private:
  std::shared_ptr<SetOnce<std::shared_ptr<IndexWriter>>> writer =
      std::make_shared<SetOnce<std::shared_ptr<IndexWriter>>>();

  /**
   * Sets the {@link IndexWriter} this config is attached to.
   *
   * @throws AlreadySetException
   *           if this config is already attached to a writer.
   */
public:
  std::shared_ptr<IndexWriterConfig>
  setIndexWriter(std::shared_ptr<IndexWriter> writer);

  /**
   * Creates a new config, using {@link StandardAnalyzer} as the
   * analyzer.  By default, {@link TieredMergePolicy} is used
   * for merging;
   * Note that {@link TieredMergePolicy} is free to select
   * non-contiguous merges, which means docIDs may not
   * remain monotonic over time.  If this is a problem you
   * should switch to {@link LogByteSizeMergePolicy} or
   * {@link LogDocMergePolicy}.
   */
  IndexWriterConfig();

  /**
   * Creates a new config that with the provided {@link
   * Analyzer}. By default, {@link TieredMergePolicy} is used
   * for merging;
   * Note that {@link TieredMergePolicy} is free to select
   * non-contiguous merges, which means docIDs may not
   * remain monotonic over time.  If this is a problem you
   * should switch to {@link LogByteSizeMergePolicy} or
   * {@link LogDocMergePolicy}.
   */
  IndexWriterConfig(std::shared_ptr<Analyzer> analyzer);

  /** Specifies {@link OpenMode} of the index.
   *
   * <p>Only takes effect when IndexWriter is first created. */
  std::shared_ptr<IndexWriterConfig> setOpenMode(OpenMode openMode);

  OpenMode getOpenMode() override;

  /**
   * Expert: allows an optional {@link IndexDeletionPolicy} implementation to be
   * specified. You can use this to control when prior commits are deleted from
   * the index. The default policy is {@link KeepOnlyLastCommitDeletionPolicy}
   * which removes all prior commits as soon as a new commit is done (this
   * matches behavior before 2.2). Creating your own policy can allow you to
   * explicitly keep previous "point in time" commits alive in the index for
   * some time, to allow readers to refresh to the new commit without having the
   * old commit deleted out from under them. This is necessary on filesystems
   * like NFS that do not support "delete on last close" semantics, which
   * Lucene's "point in time" search normally relies on.
   * <p>
   * <b>NOTE:</b> the deletion policy must not be null.
   *
   * <p>Only takes effect when IndexWriter is first created.
   */
  std::shared_ptr<IndexWriterConfig>
  setIndexDeletionPolicy(std::shared_ptr<IndexDeletionPolicy> delPolicy);

  std::shared_ptr<IndexDeletionPolicy> getIndexDeletionPolicy() override;

  /**
   * Expert: allows to open a certain commit point. The default is null which
   * opens the latest commit point.  This can also be used to open {@link
   * IndexWriter} from a near-real-time reader, if you pass the reader's
   * {@link DirectoryReader#getIndexCommit}.
   *
   * <p>Only takes effect when IndexWriter is first created. */
  std::shared_ptr<IndexWriterConfig>
  setIndexCommit(std::shared_ptr<IndexCommit> commit);

  std::shared_ptr<IndexCommit> getIndexCommit() override;

  /**
   * Expert: set the {@link Similarity} implementation used by this IndexWriter.
   * <p>
   * <b>NOTE:</b> the similarity must not be null.
   *
   * <p>Only takes effect when IndexWriter is first created. */
  std::shared_ptr<IndexWriterConfig>
  setSimilarity(std::shared_ptr<Similarity> similarity);

  std::shared_ptr<Similarity> getSimilarity() override;

  /**
   * Expert: sets the merge scheduler used by this writer. The default is
   * {@link ConcurrentMergeScheduler}.
   * <p>
   * <b>NOTE:</b> the merge scheduler must not be null.
   *
   * <p>Only takes effect when IndexWriter is first created. */
  std::shared_ptr<IndexWriterConfig>
  setMergeScheduler(std::shared_ptr<MergeScheduler> mergeScheduler);

  std::shared_ptr<MergeScheduler> getMergeScheduler() override;

  /**
   * Set the {@link Codec}.
   *
   * <p>
   * Only takes effect when IndexWriter is first created.
   */
  std::shared_ptr<IndexWriterConfig> setCodec(std::shared_ptr<Codec> codec);

  std::shared_ptr<Codec> getCodec() override;

  std::shared_ptr<MergePolicy> getMergePolicy() override;

  /** Expert: Sets the {@link DocumentsWriterPerThreadPool} instance used by the
   * IndexWriter to assign thread-states to incoming indexing threads.
   * <p>
   * NOTE: The given {@link DocumentsWriterPerThreadPool} instance must not be
   * used with other {@link IndexWriter} instances once it has been initialized
   * / associated with an
   * {@link IndexWriter}.
   * </p>
   * <p>
   * NOTE: This only takes effect when IndexWriter is first created.</p>*/
  std::shared_ptr<IndexWriterConfig> setIndexerThreadPool(
      std::shared_ptr<DocumentsWriterPerThreadPool> threadPool);

  std::shared_ptr<DocumentsWriterPerThreadPool> getIndexerThreadPool() override;

  /** By default, IndexWriter does not pool the
   *  SegmentReaders it must open for deletions and
   *  merging, unless a near-real-time reader has been
   *  obtained by calling {@link DirectoryReader#open(IndexWriter)}.
   *  This method lets you enable pooling without getting a
   *  near-real-time reader.  NOTE: if you set this to
   *  false, IndexWriter will still pool readers once
   *  {@link DirectoryReader#open(IndexWriter)} is called.
   *
   * <p>Only takes effect when IndexWriter is first created. */
  std::shared_ptr<IndexWriterConfig> setReaderPooling(bool readerPooling);

  bool getReaderPooling() override;

  /**
   * Expert: Controls when segments are flushed to disk during indexing.
   * The {@link FlushPolicy} initialized during {@link IndexWriter}
   * instantiation and once initialized the given instance is bound to this
   * {@link IndexWriter} and should not be used with another writer.
   * @see #setMaxBufferedDocs(int)
   * @see #setRAMBufferSizeMB(double)
   */
  std::shared_ptr<IndexWriterConfig>
  setFlushPolicy(std::shared_ptr<FlushPolicy> flushPolicy);

  /**
   * Expert: Sets the maximum memory consumption per thread triggering a forced
   * flush if exceeded. A {@link DocumentsWriterPerThread} is forcefully flushed
   * once it exceeds this limit even if the {@link #getRAMBufferSizeMB()} has
   * not been exceeded. This is a safety limit to prevent a
   * {@link DocumentsWriterPerThread} from address space exhaustion due to its
   * internal 32 bit signed integer based memory addressing.
   * The given value must be less that 2GB (2048MB)
   *
   * @see #DEFAULT_RAM_PER_THREAD_HARD_LIMIT_MB
   */
  std::shared_ptr<IndexWriterConfig>
  setRAMPerThreadHardLimitMB(int perThreadHardLimitMB);

  int getRAMPerThreadHardLimitMB() override;

  std::shared_ptr<FlushPolicy> getFlushPolicy() override;

  std::shared_ptr<InfoStream> getInfoStream() override;

  std::shared_ptr<Analyzer> getAnalyzer() override;

  int getMaxBufferedDocs() override;

  IndexReaderWarmer getMergedSegmentWarmer() override;

  double getRAMBufferSizeMB() override;

  /**
   * Information about merges, deletes and a
   * message when maxFieldLength is reached will be printed
   * to this. Must not be null, but {@link InfoStream#NO_OUTPUT}
   * may be used to supress output.
   */
  std::shared_ptr<IndexWriterConfig>
  setInfoStream(std::shared_ptr<InfoStream> infoStream);

  /**
   * Convenience method that uses {@link PrintStreamInfoStream}.  Must not be
   * null.
   */
  std::shared_ptr<IndexWriterConfig>
  setInfoStream(std::shared_ptr<PrintStream> printStream);

  std::shared_ptr<IndexWriterConfig>
  setMergePolicy(std::shared_ptr<MergePolicy> mergePolicy) override;

  std::shared_ptr<IndexWriterConfig>
  setMaxBufferedDocs(int maxBufferedDocs) override;

  std::shared_ptr<IndexWriterConfig>
  setMergedSegmentWarmer(IndexReaderWarmer mergeSegmentWarmer) override;

  std::shared_ptr<IndexWriterConfig>
  setRAMBufferSizeMB(double ramBufferSizeMB) override;

  std::shared_ptr<IndexWriterConfig>
  setUseCompoundFile(bool useCompoundFile) override;

  /**
   * Sets if calls {@link IndexWriter#close()} should first commit
   * before closing.  Use <code>true</code> to match behavior of Lucene 4.x.
   */
  std::shared_ptr<IndexWriterConfig> setCommitOnClose(bool commitOnClose);

  /** We only allow sorting on these types */
private:
  static const std::shared_ptr<EnumSet<SortField::Type>>
      ALLOWED_INDEX_SORT_TYPES;

  /**
   * Set the {@link Sort} order to use for all (flushed and merged) segments.
   */
public:
  std::shared_ptr<IndexWriterConfig> setIndexSort(std::shared_ptr<Sort> sort);

  virtual std::wstring toString();

  std::shared_ptr<IndexWriterConfig>
  setCheckPendingFlushUpdate(bool checkPendingFlushOnUpdate) override;

  /**
   * Sets the soft deletes field. A soft delete field in lucene is a doc-values
   * field that marks a document as soft-deleted if a document has at least one
   * value in that field. If a document is marked as soft-deleted the document
   * is treated as if it has been hard-deleted through the IndexWriter API
   * ({@link IndexWriter#deleteDocuments(Term...)}. Merges will reclaim
   * soft-deleted as well as hard-deleted documents and index readers obtained
   * from the IndexWriter will reflect all deleted documents in it's live docs.
   * If soft-deletes are used documents must be indexed via
   * {@link IndexWriter#softUpdateDocument(Term, Iterable, Field...)}. Deletes
   * are applied via
   * {@link IndexWriter#updateDocValues(Term, Field...)}.
   *
   * Soft deletes allow to retain documents across merges if the merge policy
   * modifies the live docs of a merge reader.
   * {@link SoftDeletesRetentionMergePolicy} for instance allows to specify an
   * arbitrary query to mark all documents that should survive the merge. This
   * can be used to for example keep all document modifications for a certain
   * time interval or the last N operations if some kind of sequence ID is
   * available in the index.
   *
   * Currently there is no API support to un-delete a soft-deleted document. In
   * oder to un-delete the document must be re-indexed using {@link
   * IndexWriter#softUpdateDocument(Term, Iterable, Field...)}.
   *
   * The default value for this is <code>null</code> which disables
   * soft-deletes. If soft-deletes are enabled documents can still be
   * hard-deleted. Hard-deleted documents will won't considered as soft-deleted
   * even if they have a value in the soft-deletes field.
   *
   * @see #getSoftDeletesField()
   */
  std::shared_ptr<IndexWriterConfig>
  setSoftDeletesField(const std::wstring &softDeletesField);

protected:
  std::shared_ptr<IndexWriterConfig> shared_from_this()
  {
    return std::static_pointer_cast<IndexWriterConfig>(
        LiveIndexWriterConfig::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
