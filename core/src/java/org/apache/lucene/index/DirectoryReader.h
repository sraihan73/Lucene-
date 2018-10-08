#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReader;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class IndexCommit;
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

using Directory = org::apache::lucene::store::Directory;

/** DirectoryReader is an implementation of {@link CompositeReader}
 that can read indexes in a {@link Directory}.

 <p>DirectoryReader instances are usually constructed with a call to
 one of the static <code>open()</code> methods, e.g. {@link
 #open(Directory)}.

 <p> For efficiency, in this API documents are often referred to via
 <i>document numbers</i>, non-negative integers which each name a unique
 document in the index.  These document numbers are ephemeral -- they may change
 as documents are added to and deleted from an index.  Clients should thus not
 rely on a given document having the same number between sessions.

 <p>
 <a name="thread-safety"></a><p><b>NOTE</b>: {@link
 IndexReader} instances are completely thread
 safe, meaning multiple threads can call any of its methods,
 concurrently.  If your application requires external
 synchronization, you should <b>not</b> synchronize on the
 <code>IndexReader</code> instance; use your own
 (non-Lucene) objects instead.
*/
class DirectoryReader : public BaseCompositeReader<std::shared_ptr<LeafReader>>
{
  GET_CLASS_NAME(DirectoryReader)

  /** The index directory. */
protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<Directory> directory_;

  /** Returns a IndexReader reading the index in the given
   *  Directory
   * @param directory the index directory
   * @throws IOException if there is a low-level IO error
   */
public:
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<Directory> directory) ;

  /**
   * Open a near real time IndexReader from the {@link
   * org.apache.lucene.index.IndexWriter}.
   *
   * @param writer The IndexWriter to open from
   * @return The new IndexReader
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @see #openIfChanged(DirectoryReader,IndexWriter,bool)
   *
   * @lucene.experimental
   */
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<IndexWriter> writer) ;

  /**
   * Expert: open a near real time IndexReader from the {@link
   * org.apache.lucene.index.IndexWriter}, controlling whether past deletions
   * should be applied.
   *
   * @param writer The IndexWriter to open from
   * @param applyAllDeletes If true, all buffered deletes will
   * be applied (made visible) in the returned reader.  If
   * false, the deletes are not applied but remain buffered
   * (in IndexWriter) so that they will be applied in the
   * future.  Applying deletes can be costly, so if your app
   * can tolerate deleted documents being returned you might
   * gain some performance by passing false.
   * @param writeAllDeletes If true, new deletes will be written
   * down to index files instead of carried over from writer to
   * reader directly in heap
   *
   * @see #open(IndexWriter)
   *
   * @lucene.experimental
   */
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<IndexWriter> writer, bool applyAllDeletes,
       bool writeAllDeletes) ;

  /** Expert: returns an IndexReader reading the index in the given
   *  {@link IndexCommit}.
   * @param commit the commit point to open
   * @throws IOException if there is a low-level IO error
   */
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<IndexCommit> commit) ;

  /**
   * If the index has changed since the provided reader was
   * opened, open and return a new reader; else, return
   * null.  The new reader, if not null, will be the same
   * type of reader as the previous one, ie an NRT reader
   * will open a new NRT reader, a MultiReader will open a
   * new MultiReader,  etc.
   *
   * <p>This method is typically far less costly than opening a
   * fully new <code>DirectoryReader</code> as it shares
   * resources (for example sub-readers) with the provided
   * <code>DirectoryReader</code>, when possible.
   *
   * <p>The provided reader is not closed (you are responsible
   * for doing so); if a new reader is returned you also
   * must eventually close it.  Be sure to never close a
   * reader while other threads are still using it; see
   * {@link SearcherManager} to simplify managing this.
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @return null if there are no changes; else, a new
   * DirectoryReader instance which you must eventually close
   */
  static std::shared_ptr<DirectoryReader>
  openIfChanged(std::shared_ptr<DirectoryReader> oldReader) ;

  /**
   * If the IndexCommit differs from what the
   * provided reader is searching, open and return a new
   * reader; else, return null.
   *
   * @see #openIfChanged(DirectoryReader)
   */
  static std::shared_ptr<DirectoryReader>
  openIfChanged(std::shared_ptr<DirectoryReader> oldReader,
                std::shared_ptr<IndexCommit> commit) ;

  /**
   * Expert: If there changes (committed or not) in the
   * {@link IndexWriter} versus what the provided reader is
   * searching, then open and return a new
   * IndexReader searching both committed and uncommitted
   * changes from the writer; else, return null (though, the
   * current implementation never returns null).
   *
   * <p>This provides "near real-time" searching, in that
   * changes made during an {@link IndexWriter} session can be
   * quickly made available for searching without closing
   * the writer nor calling {@link IndexWriter#commit}.
   *
   * <p>It's <i>near</i> real-time because there is no hard
   * guarantee on how quickly you can get a new reader after
   * making changes with IndexWriter.  You'll have to
   * experiment in your situation to determine if it's
   * fast enough.  As this is a new and experimental
   * feature, please report back on your findings so we can
   * learn, improve and iterate.</p>
   *
   * <p>The very first time this method is called, this
   * writer instance will make every effort to pool the
   * readers that it opens for doing merges, applying
   * deletes, etc.  This means additional resources (RAM,
   * file descriptors, CPU time) will be consumed.</p>
   *
   * <p>For lower latency on reopening a reader, you should
   * call {@link IndexWriterConfig#setMergedSegmentWarmer} to
   * pre-warm a newly merged segment before it's committed
   * to the index.  This is important for minimizing
   * index-to-search delay after a large merge.  </p>
   *
   * <p>If an addIndexes* call is running in another thread,
   * then this reader will only search those segments from
   * the foreign index that have been successfully copied
   * over, so far.</p>
   *
   * <p><b>NOTE</b>: Once the writer is closed, any
   * outstanding readers may continue to be used.  However,
   * if you attempt to reopen any of those readers, you'll
   * hit an {@link org.apache.lucene.store.AlreadyClosedException}.</p>
   *
   * @return DirectoryReader that covers entire index plus all
   * changes made so far by this IndexWriter instance, or
   * null if there are no new changes
   *
   * @param writer The IndexWriter to open from
   *
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
  static std::shared_ptr<DirectoryReader>
  openIfChanged(std::shared_ptr<DirectoryReader> oldReader,
                std::shared_ptr<IndexWriter> writer) ;

  /**
   * Expert: Opens a new reader, if there are any changes, controlling whether
   * past deletions should be applied.
   *
   * @see #openIfChanged(DirectoryReader,IndexWriter)
   *
   * @param writer The IndexWriter to open from
   *
   * @param applyAllDeletes If true, all buffered deletes will
   * be applied (made visible) in the returned reader.  If
   * false, the deletes are not applied but remain buffered
   * (in IndexWriter) so that they will be applied in the
   * future.  Applying deletes can be costly, so if your app
   * can tolerate deleted documents being returned you might
   * gain some performance by passing false.
   *
   * @throws IOException if there is a low-level IO error
   *
   * @lucene.experimental
   */
  static std::shared_ptr<DirectoryReader>
  openIfChanged(std::shared_ptr<DirectoryReader> oldReader,
                std::shared_ptr<IndexWriter> writer,
                bool applyAllDeletes) ;

  /** Returns all commit points that exist in the Directory.
   *  Normally, because the default is {@link
   *  KeepOnlyLastCommitDeletionPolicy}, there would be only
   *  one commit point.  But if you're using a custom {@link
   *  IndexDeletionPolicy} then there could be many commits.
   *  Once you have a given commit, you can open a reader on
   *  it by calling {@link DirectoryReader#open(IndexCommit)}
   *  There must be at least one commit in
   *  the Directory, else this method throws {@link
   *  IndexNotFoundException}.  Note that if a commit is in
   *  progress while this method is running, that commit
   *  may or may not be returned.
   *
   *  @return a sorted deque of {@link IndexCommit}s, from oldest
   *  to latest. */
  static std::deque<std::shared_ptr<IndexCommit>>
  listCommits(std::shared_ptr<Directory> dir) ;

  /**
   * Returns <code>true</code> if an index likely exists at
   * the specified directory.  Note that if a corrupt index
   * exists, or if an index in the process of committing
   * @param  directory the directory to check for an index
   * @return <code>true</code> if an index exists; <code>false</code> otherwise
   */
  static bool
  indexExists(std::shared_ptr<Directory> directory) ;

  /**
   * Expert: Constructs a {@code DirectoryReader} on the given subReaders.
   * @param segmentReaders the wrapped atomic index segment readers. This array
is
   * returned by {@link #getSequentialSubReaders} and used to resolve the
correct
   * subreader for docID-based methods. <b>Please note:</b> This array is
<b>not</b>
   * cloned and not protected for modification outside of this reader.
   * Subclasses of {@code DirectoryReader} should take care to not allow
   * modification of this internal array, e.g. {@link #doOpenIfChanged()}.
GET_CLASS_NAME(es)
   */
protected:
  DirectoryReader(std::shared_ptr<Directory> directory,
                  std::deque<std::shared_ptr<LeafReader>>
                      &segmentReaders) ;

  /** Returns the directory this index resides in. */
public:
  std::shared_ptr<Directory> directory();

  /** Implement this method to support {@link #openIfChanged(DirectoryReader)}.
   * If this reader does not support reopen, return {@code null}, so
   * client code is happy. This should be consistent with {@link #isCurrent}
   * (should always return {@code true}) if reopen is not supported.
   * @throws IOException if there is a low-level IO error
   * @return null if there are no changes; else, a new
   * DirectoryReader instance.
   */
protected:
  virtual std::shared_ptr<DirectoryReader> doOpenIfChanged() = 0;

  /** Implement this method to support {@link
   * #openIfChanged(DirectoryReader,IndexCommit)}. If this reader does not
   * support reopen from a specific {@link IndexCommit}, throw {@link
   * UnsupportedOperationException}.
   * @throws IOException if there is a low-level IO error
   * @return null if there are no changes; else, a new
   * DirectoryReader instance.
   */
  virtual std::shared_ptr<DirectoryReader>
  doOpenIfChanged(std::shared_ptr<IndexCommit> commit) = 0;

  /** Implement this method to support {@link
   * #openIfChanged(DirectoryReader,IndexWriter,bool)}. If this reader does
   * not support reopen from {@link IndexWriter}, throw {@link
   * UnsupportedOperationException}.
   * @throws IOException if there is a low-level IO error
   * @return null if there are no changes; else, a new
   * DirectoryReader instance.
   */
  virtual std::shared_ptr<DirectoryReader>
  doOpenIfChanged(std::shared_ptr<IndexWriter> writer,
                  bool applyAllDeletes) = 0;

  /**
   * Version number when this IndexReader was opened.
   *
   * <p>This method
   * returns the version recorded in the commit that the
   * reader opened.  This version is advanced every time
   * a change is made with {@link IndexWriter}.</p>
   */
public:
  virtual int64_t getVersion() = 0;

  /**
   * Check whether any new changes have occurred to the
   * index since this reader was opened.
   *
   * <p>If this reader was created by calling {@link #open},
   * then this method checks if any further commits
   * (see {@link IndexWriter#commit}) have occurred in the
   * directory.</p>
   *
   * <p>If instead this reader is a near real-time reader
   * (ie, obtained by a call to {@link
   * DirectoryReader#open(IndexWriter)}, or by calling {@link #openIfChanged}
   * on a near real-time reader), then this method checks if
   * either a new commit has occurred, or any new
   * uncommitted changes have taken place via the writer.
   * Note that even if the writer has only performed
   * merging, this method will still return false.</p>
   *
   * <p>In any event, if this returns false, you should call
   * {@link #openIfChanged} to get a new reader that sees the
   * changes.</p>
   *
   * @throws IOException           if there is a low-level IO error
   */
  virtual bool isCurrent() = 0;

  /**
   * Expert: return the IndexCommit that this reader has opened.
   * @lucene.experimental
   */
  virtual std::shared_ptr<IndexCommit> getIndexCommit() = 0;

protected:
  std::shared_ptr<DirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<DirectoryReader>(
        BaseCompositeReader<LeafReader>::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
