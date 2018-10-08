#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <mutex>
#include <string>
#include <deque>
#include <atomic>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class CacheKey;
}

namespace org::apache::lucene::store
{
class AlreadyClosedException;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::index
{
class IndexReaderContext;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class Term;
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

using Document = org::apache::lucene::document::Document;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

/**
 IndexReader is an abstract class, providing an interface for accessing a
 point-in-time view of an index.  Any changes made to the index
 via {@link IndexWriter} will not be visible until a new
 {@code IndexReader} is opened.  It's best to use {@link
 DirectoryReader#open(IndexWriter)} to obtain an
 {@code IndexReader}, if your {@link IndexWriter} is
 in-process.  When you need to re-open to see changes to the
 index, it's best to use {@link DirectoryReader#openIfChanged(DirectoryReader)}
 since the new reader will share resources with the previous
 one when possible.  Search of an index is done entirely
 through this abstract interface, so that any subclass which
 implements it is searchable.

 <p>There are two different types of IndexReaders:
 <ul>
  <li>{@link LeafReader}: These indexes do not consist of several sub-readers,
  they are atomic. They support retrieval of stored fields, doc values, terms,
  and postings.
  <li>{@link CompositeReader}: Instances (like {@link DirectoryReader})
  of this reader can only
  be used to get stored fields from the underlying LeafReaders,
  but it is not possible to directly retrieve postings. To do that, get
  the sub-readers via {@link CompositeReader#getSequentialSubReaders}.
 </ul>

 <p>IndexReader instances for indexes on disk are usually constructed
 with a call to one of the static <code>DirectoryReader.open()</code> methods,
 e.g. {@link DirectoryReader#open(org.apache.lucene.store.Directory)}. {@link
 DirectoryReader} implements the {@link CompositeReader} interface, it is not
 possible to directly get postings.

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
class IndexReader : public std::enable_shared_from_this<IndexReader>
{
  GET_CLASS_NAME(IndexReader)

private:
  bool closed = false;
  bool closedByChild = false;
  std::atomic_int64_t refCount = 1;

public:
  IndexReader();

  /**
   * A utility class that gives hooks in order to help build a cache based on
   * the data that is contained in this index.
   * @lucene.experimental
   */
public:
  class CacheHelper
  {
    GET_CLASS_NAME(CacheHelper)

  /** A cache key identifying a resource that is being cached on. */
public:
  class CacheKey final : public std::enable_shared_from_this<CacheKey>
  {
    GET_CLASS_NAME(CacheKey)
  public:
    CacheKey();
  };
    /**
     * Get a key that the resource can be cached on. The given entry can be
     * compared using identity, ie. {@link Object#equals} is implemented as
     * {@code ==} and {@link Object#hashCode} is implemented as
     * {@link System#identityHashCode}.
     */
  public:
    virtual std::shared_ptr<CacheKey> getKey() = 0;

  /**
   * A listener that is called when a resource gets closed.
   * @lucene.experimental
   */
  using ClosedListener = std::function<void(CacheKey key)>;
    /**
     * Add a {@link ClosedListener} which will be called when the resource
     * guarded by {@link #getKey()} is closed.
     */
    virtual void addClosedListener(ClosedListener listener) = 0;
  
private:
  // TODO: Needs mutex
  const std::shared_ptr<std::unordered_set<std::shared_ptr<IndexReader>>> parentReaders =
          std::make_shared<std::unordered_set<std::shared_ptr<IndexReader>>>();

  /** Expert: This method is called by {@code IndexReader}s which wrap other
   * readers (e.g. {@link CompositeReader} or {@link FilterLeafReader}) to
   * register the parent at the child (this reader) on construction of the
   * parent. When this reader is closed, it will mark all registered parents as
   * closed, too. The references to parent readers are weak only, so they can be
   * GCed once they are no longer in use.
   * @lucene.experimental */
public:
  void registerParentReader(std::shared_ptr<IndexReader> reader);

  // overridden by StandardDirectoryReader and SegmentReader
  virtual void notifyReaderClosedListeners() ;

private:
  void reportCloseToParentReaders() ;
  };

  /** Expert: returns the current refCount for this reader */
public:
  int getRefCount();

  /**
   * Expert: increments the refCount of this IndexReader
   * instance.  RefCounts are used to determine when a
   * reader can be closed safely, i.e. as soon as there are
   * no more references.  Be sure to always call a
   * corresponding {@link #decRef}, in a finally clause;
   * otherwise the reader may never be closed.  Note that
   * {@link #close} simply calls decRef(), which means that
   * the IndexReader will not really be closed until {@link
   * #decRef} has been called for all outstanding
   * references.
   *
   * @see #decRef
   * @see #tryIncRef
   */
  void incRef();

  /**
   * Expert: increments the refCount of this IndexReader
   * instance only if the IndexReader has not been closed yet
   * and returns <code>true</code> iff the refCount was
   * successfully incremented, otherwise <code>false</code>.
   * If this method returns <code>false</code> the reader is either
   * already closed or is currently being closed. Either way this
   * reader instance shouldn't be used by an application unless
   * <code>true</code> is returned.
   * <p>
   * RefCounts are used to determine when a
   * reader can be closed safely, i.e. as soon as there are
   * no more references.  Be sure to always call a
   * corresponding {@link #decRef}, in a finally clause;
   * otherwise the reader may never be closed.  Note that
   * {@link #close} simply calls decRef(), which means that
   * the IndexReader will not really be closed until {@link
   * #decRef} has been called for all outstanding
   * references.
   *
   * @see #decRef
   * @see #incRef
   */
  bool tryIncRef();

  /**
   * Expert: decreases the refCount of this IndexReader
   * instance.  If the refCount drops to 0, then this
   * reader is closed.  If an exception is hit, the refCount
   * is unchanged.
   *
   * @throws IOException in case an IOException occurs in  doClose()
   *
   * @see #incRef
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") public final void decRef() throws
  // java.io.IOException
  void decRef() ;

  /**
   * Throws AlreadyClosedException if this IndexReader or any
   * of its child readers is closed, otherwise returns.
   */
protected:
  void ensureOpen() ;

  /** {@inheritDoc}
   * <p>{@code IndexReader} subclasses are not allowed
   * to implement equals/hashCode, so methods are declared final.
   */
public:
  virtual bool equals(std::any obj);

  /** {@inheritDoc}
   * <p>{@code IndexReader} subclasses are not allowed
   * to implement equals/hashCode, so methods are declared final.
   */
  virtual int hashCode();

  /** Retrieve term vectors for this document, or null if
   *  term vectors were not indexed.  The returned Fields
   *  instance acts like a single-document inverted index
   *  (the docID will be 0). */
  virtual std::shared_ptr<Fields> getTermVectors(int docID) = 0;

  /** Retrieve term deque for this document and field, or
   *  null if term vectors were not indexed.  The returned
   *  Fields instance acts like a single-document inverted
   *  index (the docID will be 0). */
  std::shared_ptr<Terms>
  getTermVector(int docID, const std::wstring &field) ;

  /** Returns the number of documents in this index. */
  virtual int numDocs() = 0;

  /** Returns one greater than the largest possible document number.
   * This may be used to, e.g., determine how big to allocate an array which
   * will have an element for every document number in an index.
   */
  virtual int maxDoc() = 0;

  /** Returns the number of deleted documents. */
  int numDeletedDocs();

  /** Expert: visits the fields of a stored document, for
   *  custom processing/loading of each field.  If you
   *  simply want to load all fields, use {@link
   *  #document(int)}.  If you want to load a subset, use
   *  {@link DocumentStoredFieldVisitor}.  */
  virtual void document(int docID,
                        std::shared_ptr<StoredFieldVisitor> visitor) = 0;

  /**
   * Returns the stored fields of the <code>n</code><sup>th</sup>
   * <code>Document</code> in this index.  This is just
   * sugar for using {@link DocumentStoredFieldVisitor}.
   * <p>
   * <b>NOTE:</b> for performance reasons, this method does not check if the
   * requested document is deleted, and therefore asking for a deleted document
   * may yield unspecified results. Usually this is not required, however you
   * can test if the doc is deleted by checking the {@link
   * Bits} returned from {@link MultiFields#getLiveDocs}.
   *
   * <b>NOTE:</b> only the content of a field is returned,
   * if that field was stored during indexing.  Metadata
   * like boost, omitNorm, IndexOptions, tokenized, etc.,
   * are not preserved.
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  // TODO: we need a separate StoredField, so that the
  // Document returned here contains that class not
  // IndexableField
  std::shared_ptr<Document> document(int docID) ;

  /**
   * Like {@link #document(int)} but only loads the specified
   * fields.  Note that this is simply sugar for {@link
   * DocumentStoredFieldVisitor#DocumentStoredFieldVisitor(std::unordered_set)}.
   */
  std::shared_ptr<Document>
  document(int docID,
           std::shared_ptr<std::unordered_set<std::wstring>> fieldsToLoad) ;

  /** Returns true if any documents have been deleted. Implementers should
   *  consider overriding this method if {@link #maxDoc()} or {@link #numDocs()}
   *  are not constant-time operations. */
  virtual bool hasDeletions();

  /**
   * Closes files associated with this index.
   * Also saves any new deletions to disk.
   * No other methods should be called after this has been called.
   * @throws IOException if there is a low-level IO error
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  ~IndexReader();

  /** Implements close. */
protected:
  virtual void doClose() = 0;

  /**
   * Expert: Returns the root {@link IndexReaderContext} for this
   * {@link IndexReader}'s sub-reader tree.
   * <p>
   * Iff this reader is composed of sub
   * readers, i.e. this reader being a composite reader, this method returns a
   * {@link CompositeReaderContext} holding the reader's direct children as well
   * as a view of the reader tree's atomic leaf contexts. All sub-
   * {@link IndexReaderContext} instances referenced from this readers top-level
   * context are private to this reader and are not shared with another context
   * tree. For example, IndexSearcher uses this API to drive searching by one
   * atomic leaf reader at a time. If this reader is not composed of child
   * readers, this method returns an {@link LeafReaderContext}.
   * <p>
   * Note: Any of the sub-{@link CompositeReaderContext} instances referenced
   * from this top-level context do not support {@link
   * CompositeReaderContext#leaves()}. Only the top-level context maintains the
   * convenience leaf-view for performance reasons.
   */
public:
  virtual std::shared_ptr<IndexReaderContext> getContext() = 0;

  /**
   * Returns the reader's leaves, or itself if this reader is atomic.
   * This is a convenience method calling {@code this.getContext().leaves()}.
   * @see IndexReaderContext#leaves()
   */
  std::deque<std::shared_ptr<LeafReaderContext>> leaves();

  /**
   * Optional method: Return a {@link CacheHelper} that can be used to cache
   * based on the content of this reader. Two readers that have different data
   * or different sets of deleted documents will be considered different.
   * <p>A return value of {@code null} indicates that this reader is not suited
   * for caching, which is typically the case for short-lived wrappers that
   * alter the content of the wrapped reader.
   * @lucene.experimental
   */
  virtual std::shared_ptr<CacheHelper> getReaderCacheHelper() = 0;

  /** Returns the number of documents containing the
   * <code>term</code>.  This method returns 0 if the term or
   * field does not exists.  This method does not take into
   * account deleted documents that have not yet been merged
   * away.
   * @see TermsEnum#docFreq()
   */
  virtual int docFreq(std::shared_ptr<Term> term) = 0;

  /**
   * Returns the total number of occurrences of {@code term} across all
   * documents (the sum of the freq() for each doc that has this term). This
   * will be -1 if the codec doesn't support this measure. Note that, like other
   * term measures, this measure does not take deleted documents into account.
   */
  virtual int64_t totalTermFreq(std::shared_ptr<Term> term) = 0;

  /**
   * Returns the sum of {@link TermsEnum#docFreq()} for all terms in this field,
   * or -1 if this measure isn't stored by the codec. Note that, just like other
   * term measures, this measure does not take deleted documents into account.
   *
   * @see Terms#getSumDocFreq()
   */
  virtual int64_t getSumDocFreq(const std::wstring &field) = 0;

  /**
   * Returns the number of documents that have at least one term for this field,
   * or -1 if this measure isn't stored by the codec. Note that, just like other
   * term measures, this measure does not take deleted documents into account.
   *
   * @see Terms#getDocCount()
   */
  virtual int getDocCount(const std::wstring &field) = 0;

  /**
   * Returns the sum of {@link TermsEnum#totalTermFreq} for all terms in this
   * field, or -1 if this measure isn't stored by the codec (or if this fields
   * omits term freq and positions). Note that, just like other term measures,
   * this measure does not take deleted documents into account.
   *
   * @see Terms#getSumTotalTermFreq()
   */
  virtual int64_t getSumTotalTermFreq(const std::wstring &field) = 0;
};

} // namespace org::apache::lucene::index
