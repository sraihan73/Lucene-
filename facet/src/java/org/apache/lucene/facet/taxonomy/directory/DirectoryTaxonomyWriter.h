#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::facet::taxonomy::writercache
{
class TaxonomyWriterCache;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class SinglePositionTokenStream;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::index
{
class ReaderManager;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class TaxonomyIndexArrays;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class OrdinalMap;
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
namespace org::apache::lucene::facet::taxonomy::directory
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Field = org::apache::lucene::document::Field;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using ReaderManager = org::apache::lucene::index::ReaderManager;
using Directory = org::apache::lucene::store::Directory;

/**
 * {@link TaxonomyWriter} which uses a {@link Directory} to store the taxonomy
 * information on disk, and keeps an additional in-memory cache of some or all
 * categories.
 * <p>
 * In addition to the permanently-stored information in the {@link Directory},
 * efficiency dictates that we also keep an in-memory cache of <B>recently
 * seen</B> or <B>all</B> categories, so that we do not need to go back to disk
 * for every category addition to see which ordinal this category already has,
 * if any. A {@link TaxonomyWriterCache} object determines the specific caching
 * algorithm used.
 * <p>
 * This class offers some hooks for extending classes to control the
 * {@link IndexWriter} instance that is used. See {@link #openIndexWriter}.
GET_CLASS_NAME(offers)
 *
 * @lucene.experimental
 */
class DirectoryTaxonomyWriter
    : public std::enable_shared_from_this<DirectoryTaxonomyWriter>,
      public TaxonomyWriter
{
  GET_CLASS_NAME(DirectoryTaxonomyWriter)

  /**
   * Property name of user commit data that contains the index epoch. The epoch
   * changes whenever the taxonomy is recreated (i.e. opened with
   * {@link OpenMode#CREATE}.
   * <p>
   * Applications should not use this property in their commit data because it
   * will be overridden by this taxonomy writer.
   */
public:
  static const std::wstring INDEX_EPOCH;

private:
  const std::shared_ptr<Directory> dir;
  const std::shared_ptr<IndexWriter> indexWriter;
  const std::shared_ptr<TaxonomyWriterCache> cache;
  const std::shared_ptr<AtomicInteger> cacheMisses =
      std::make_shared<AtomicInteger>(0);

  // Records the taxonomy index epoch, updated on replaceTaxonomy as well.
  int64_t indexEpoch = 0;

  std::shared_ptr<SinglePositionTokenStream> parentStream =
      std::make_shared<SinglePositionTokenStream>(Consts::PAYLOAD_PARENT);
  std::shared_ptr<Field> parentStreamField;
  std::shared_ptr<Field> fullPathField;
  int cacheMissesUntilFill = 11;
  bool shouldFillCache = true;

  // even though lazily initialized, not volatile so that access to it is
  // faster. we keep a volatile bool init instead.
  std::shared_ptr<ReaderManager> readerManager;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool initializedReaderManager = false;
  bool initializedReaderManager = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool shouldRefreshReaderManager;
  bool shouldRefreshReaderManager = false;

  /**
   * We call the cache "complete" if we know that every category in our
   * taxonomy is in the cache. When the cache is <B>not</B> complete, and
   * we can't find a category in the cache, we still need to look for it
   * in the on-disk index; Therefore when the cache is not complete, we
   * need to open a "reader" to the taxonomy index.
   * The cache becomes incomplete if it was never filled with the existing
   * categories, or if a put() to the cache ever returned true (meaning
   * that some of the cached data was cleared).
   */
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool cacheIsComplete;
  bool cacheIsComplete = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool isClosed = false;
  bool isClosed = false;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile TaxonomyIndexArrays taxoArrays;
  std::shared_ptr<TaxonomyIndexArrays> taxoArrays;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile int nextID;
  int nextID = 0;

  /** Reads the commit data from a Directory. */
  static std::unordered_map<std::wstring, std::wstring>
  readCommitData(std::shared_ptr<Directory> dir) ;

  /**
   * Construct a Taxonomy writer.
   *
   * @param directory
   *    The {@link Directory} in which to store the taxonomy. Note that
   *    the taxonomy is written directly to that directory (not to a
   *    subdirectory of it).
   * @param openMode
   *    Specifies how to open a taxonomy for writing: <code>APPEND</code>
   *    means open an existing index for append (failing if the index does
   *    not yet exist). <code>CREATE</code> means create a new index (first
   *    deleting the old one if it already existed).
   *    <code>APPEND_OR_CREATE</code> appends to an existing index if there
   *    is one, otherwise it creates a new index.
   * @param cache
   *    A {@link TaxonomyWriterCache} implementation which determines
   *    the in-memory caching policy. See for example
   *    {@link LruTaxonomyWriterCache} and {@link UTF8TaxonomyWriterCache}.
   *    If null or missing, {@link #defaultTaxonomyWriterCache()} is used.
   * @throws CorruptIndexException
   *     if the taxonomy is corrupted.
   * @throws LockObtainFailedException
   *     if the taxonomy is locked by another writer.
   * @throws IOException
   *     if another error occurred.
   */
public:
  DirectoryTaxonomyWriter(
      std::shared_ptr<Directory> directory, OpenMode openMode,
      std::shared_ptr<TaxonomyWriterCache> cache) ;

  /**
   * Open internal index writer, which contains the taxonomy data.
   * <p>
   * Extensions may provide their own {@link IndexWriter} implementation or
   * instance. <br><b>NOTE:</b> the instance this method returns will be closed
   * upon calling to {@link #close()}. <br><b>NOTE:</b> the merge policy in
   * effect must not merge none adjacent segments. See comment in {@link
   * #createIndexWriterConfig(IndexWriterConfig.OpenMode)} for the logic behind
   * this.
   *
   * @see #createIndexWriterConfig(IndexWriterConfig.OpenMode)
   *
   * @param directory
   *          the {@link Directory} on top of which an {@link IndexWriter}
   *          should be opened.
   * @param config
   *          configuration for the internal index writer.
   */
protected:
  virtual std::shared_ptr<IndexWriter>
  openIndexWriter(std::shared_ptr<Directory> directory,
                  std::shared_ptr<IndexWriterConfig> config) ;

  /**
   * Create the {@link IndexWriterConfig} that would be used for opening the
   * internal index writer. <br>Extensions can configure the {@link IndexWriter}
   * as they see fit, including setting a {@link
   * org.apache.lucene.index.MergeScheduler merge-scheduler}, or
   * {@link org.apache.lucene.index.IndexDeletionPolicy deletion-policy},
   * different RAM size etc.<br> <br><b>NOTE:</b> internal docids of the
   * configured index must not be altered. For that, categories are never
   * deleted from the taxonomy index. In addition, merge policy in effect must
   * not merge none adjacent segments.
   *
   * @see #openIndexWriter(Directory, IndexWriterConfig)
   *
   * @param openMode see {@link OpenMode}
   */
  virtual std::shared_ptr<IndexWriterConfig>
  createIndexWriterConfig(OpenMode openMode);

  /** Opens a {@link ReaderManager} from the internal {@link IndexWriter}. */
private:
  void initReaderManager() ;

  /**
   * Creates a new instance with a default cache as defined by
   * {@link #defaultTaxonomyWriterCache()}.
   */
public:
  DirectoryTaxonomyWriter(std::shared_ptr<Directory> directory,
                          OpenMode openMode) ;

  /**
   * Defines the default {@link TaxonomyWriterCache} to use in constructors
   * which do not specify one.
   * <P>
   * The current default is {@link UTF8TaxonomyWriterCache}, i.e.,
   * the entire taxonomy is cached in memory while building it.
   */
  static std::shared_ptr<TaxonomyWriterCache> defaultTaxonomyWriterCache();

  /** Create this with {@code OpenMode.CREATE_OR_APPEND}. */
  DirectoryTaxonomyWriter(std::shared_ptr<Directory> d) ;

  /**
   * Frees used resources as well as closes the underlying {@link IndexWriter},
   * which commits whatever changes made to it to the underlying
   * {@link Directory}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~DirectoryTaxonomyWriter();

private:
  void doClose() ;

  /**
   * A hook for extending classes to close additional resources that were used.
   * The default implementation closes the {@link IndexReader} as well as the
GET_CLASS_NAME(es)
   * {@link TaxonomyWriterCache} instances that were used. <br>
   * <b>NOTE:</b> if you override this method, you should include a
   * <code>super.closeResources()</code> call in your implementation.
   */
protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void closeResources() ;

  /**
   * Look up the given category in the cache and/or the on-disk storage,
   * returning the category's ordinal, or a negative number in case the
   * category does not yet exist in the taxonomy.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int
  findCategory(std::shared_ptr<FacetLabel> categoryPath) ;

public:
  int addCategory(std::shared_ptr<FacetLabel> categoryPath) throw(
      IOException) override;

  /**
   * Add a new category into the index (and the cache), and return its new
   * ordinal.
   * <p>
   * Actually, we might also need to add some of the category's ancestors
   * before we can add the category itself (while keeping the invariant that a
   * parent is always added to the taxonomy before its child). We do this by
   * recursion.
   */
private:
  int internalAddCategory(std::shared_ptr<FacetLabel> cp) ;

  /**
   * Verifies that this instance wasn't closed, or throws
   * {@link AlreadyClosedException} if it is.
   */
protected:
  void ensureOpen();

  /**
   * Note that the methods calling addCategoryDocument() are synchornized, so
   * this method is effectively synchronized as well.
   */
private:
  int addCategoryDocument(std::shared_ptr<FacetLabel> categoryPath,
                          int parent) ;

private:
  class SinglePositionTokenStream : public TokenStream
  {
    GET_CLASS_NAME(SinglePositionTokenStream)
  private:
    std::shared_ptr<CharTermAttribute> termAtt;
    std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
    bool returned = false;
    int val = 0;
    const std::wstring word;

  public:
    SinglePositionTokenStream(const std::wstring &word);

    /**
     * Set the value we want to keep, as the position increment.
     * Note that when TermPositions.nextPosition() is later used to
     * retrieve this value, val-1 will be returned, not val.
     * <P>
     * IMPORTANT NOTE: Before Lucene 2.9, val&gt;=0 were safe (for val==0,
     * the retrieved position would be -1). But starting with Lucene 2.9,
     * this unfortunately changed, and only val&gt;0 are safe. val=0 can
     * still be used, but don't count on the value you retrieve later
     * (it could be 0 or -1, depending on circumstances or versions).
     * This change is described in Lucene's JIRA: LUCENE-1542.
     */
    virtual void set(int val);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<SinglePositionTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<SinglePositionTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

private:
  void addToCache(std::shared_ptr<FacetLabel> categoryPath,
                  int id) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void refreshReaderManager() ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t commit()  override;

  /** Combine original user data with the taxonomy epoch. */
private:
  std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
  combinedCommitData(
      std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
          &commitData);

public:
  void setLiveCommitData(
      std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
          &commitUserData) override;

  std::deque<std::unordered_map::Entry<std::wstring, std::wstring>>
  getLiveCommitData() override;

  /**
   * prepare most of the work needed for a two-phase commit.
   * See {@link IndexWriter#prepareCommit}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t prepareCommit()  override;

  int getSize() override;

  /**
   * Set the number of cache misses before an attempt is made to read the entire
   * taxonomy into the in-memory cache.
   * <p>
   * This taxonomy writer holds an in-memory cache of recently seen categories
   * to speed up operation. On each cache-miss, the on-disk index needs to be
   * consulted. When an existing taxonomy is opened, a lot of slow disk reads
   * like that are needed until the cache is filled, so it is more efficient to
   * read the entire taxonomy into memory at once. We do this complete read
   * after a certain number (defined by this method) of cache misses.
   * <p>
   * If the number is set to {@code 0}, the entire taxonomy is read into the
   * cache on first use, without fetching individual categories first.
   * <p>
   * NOTE: it is assumed that this method is called immediately after the
   * taxonomy writer has been created.
   */
  virtual void setCacheMissesUntilFill(int i);

  // we need to guarantee that if several threads call this concurrently, only
  // one executes it, and after it returns, the cache is updated and is either
  // complete or not.
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void perhapsFillCache() ;

  std::shared_ptr<TaxonomyIndexArrays> getTaxoArrays() ;

public:
  int getParent(int ordinal)  override;

  /**
   * Takes the categories from the given taxonomy directory, and adds the
   * missing ones to this taxonomy. Additionally, it fills the given
   * {@link OrdinalMap} with a mapping from the original ordinal to the new
   * ordinal.
   */
  virtual void addTaxonomy(std::shared_ptr<Directory> taxoDir,
                           std::shared_ptr<OrdinalMap> map_obj) ;

  /**
   * Mapping from old ordinal to new ordinals, used when merging indexes
   * with separate taxonomies.
   * <p>
   * addToTaxonomies() merges one or more taxonomies into the given taxonomy
   * (this). An OrdinalMap is filled for each of the added taxonomies,
   * containing the new ordinal (in the merged taxonomy) of each of the
   * categories in the old taxonomy.
   * <P>
   * There exist two implementations of OrdinalMap: MemoryOrdinalMap and
   * DiskOrdinalMap. As their names suggest, the former keeps the map_obj in
   * memory and the latter in a temporary disk file. Because these maps will
   * later be needed one by one (to remap the counting lists), not all at the
   * same time, it is recommended to put the first taxonomy's map_obj in memory,
   * and all the rest on disk (later to be automatically read into memory one
   * by one, when needed).
   */
public:
  class OrdinalMap
  {
    GET_CLASS_NAME(OrdinalMap)
    /**
     * Set the size of the map_obj. This MUST be called before addMapping().
     * It is assumed (but not verified) that addMapping() will then be
     * called exactly 'size' times, with different origOrdinals between 0
     * and size-1.
     */
  public:
    virtual void setSize(int size) = 0;

    /** Record a mapping. */
    virtual void addMapping(int origOrdinal, int newOrdinal) = 0;

    /**
     * Call addDone() to say that all addMapping() have been done.
     * In some implementations this might free some resources.
     */
    virtual void addDone() = 0;

    /**
     * Return the map_obj from the taxonomy's original (consecutive) ordinals
     * to the new taxonomy's ordinals. If the map_obj has to be read from disk
     * and ordered appropriately, it is done when getMap() is called.
     * getMap() should only be called once, and only when the map_obj is actually
     * needed. Calling it will also free all resources that the map_obj might
     * be holding (such as temporary disk space), other than the returned int[].
     */
    virtual std::deque<int> getMap() = 0;
  };

  /**
   * {@link OrdinalMap} maintained in memory
   */
public:
  class MemoryOrdinalMap final
      : public std::enable_shared_from_this<MemoryOrdinalMap>,
        public OrdinalMap
  {
    GET_CLASS_NAME(MemoryOrdinalMap)
  public:
    std::deque<int> map_obj;

    /** Sole constructor. */
    MemoryOrdinalMap();

    void setSize(int taxonomySize) override;
    void addMapping(int origOrdinal, int newOrdinal) override;
    void addDone() override;
    std::deque<int> getMap() override;
  };

  /**
   * {@link OrdinalMap} maintained on file system
   */
public:
  class DiskOrdinalMap final
      : public std::enable_shared_from_this<DiskOrdinalMap>,
        public OrdinalMap
  {
    GET_CLASS_NAME(DiskOrdinalMap)
  public:
    std::shared_ptr<Path> tmpfile;
    std::shared_ptr<DataOutputStream> out;

    /** Sole constructor. */
    DiskOrdinalMap(std::shared_ptr<Path> tmpfile) ;

    void addMapping(int origOrdinal,
                    int newOrdinal)  override;

    void setSize(int taxonomySize)  override;

    void addDone()  override;

    std::deque<int> map_obj;

    std::deque<int> getMap()  override;
  };

  /**
   * Rollback changes to the taxonomy writer and closes the instance. Following
   * this method the instance becomes unusable (calling any of its API methods
   * will yield an {@link AlreadyClosedException}).
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void rollback()  override;

  /**
   * Replaces the current taxonomy with the given one. This method should
   * generally be called in conjunction with
   * {@link IndexWriter#addIndexes(Directory...)} to replace both the taxonomy
   * as well as the search index content.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  replaceTaxonomy(std::shared_ptr<Directory> taxoDir) ;

  /** Returns the {@link Directory} of this taxonomy writer. */
  virtual std::shared_ptr<Directory> getDirectory();

  /**
   * Used by {@link DirectoryTaxonomyReader} to support NRT.
   * <p>
   * <b>NOTE:</b> you should not use the obtained {@link IndexWriter} in any
   * way, other than opening an IndexReader on it, or otherwise, the taxonomy
   * index may become corrupt!
   */
  std::shared_ptr<IndexWriter> getInternalIndexWriter();

  /** Expert: returns current index epoch, if this is a
   * near-real-time reader.  Used by {@link
   * DirectoryTaxonomyReader} to support NRT.
   *
   * @lucene.internal */
  int64_t getTaxonomyEpoch();
};

} // namespace org::apache::lucene::facet::taxonomy::directory
