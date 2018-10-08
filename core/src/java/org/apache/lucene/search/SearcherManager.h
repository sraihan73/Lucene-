#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/SearcherFactory.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;

/**
 * Utility class to safely share {@link IndexSearcher} instances across multiple
 * threads, while periodically reopening. This class ensures each searcher is
 * closed only once all threads have finished using it.
 *
 * <p>
 * Use {@link #acquire} to obtain the current searcher, and {@link #release} to
 * release it, like this:
 *
 * <pre class="prettyprint">
 * IndexSearcher s = manager.acquire();
 * try {
 *   // Do searching, doc retrieval, etc. with s
 * } finally {
 *   manager.release(s);
 * }
 * // Do not use s after this!
 * s = null;
 * </pre>
 *
 * <p>
 * In addition you should periodically call {@link #maybeRefresh}. While it's
 * possible to call this just before running each query, this is discouraged
 * since it penalizes the unlucky queries that need to refresh. It's better to
 * use a separate background thread, that periodically calls {@link
 * #maybeRefresh}. Finally, be sure to call {@link #close} once you are done.
 *
 * @see SearcherFactory
 *
 * @lucene.experimental
 */
class SearcherManager final
    : public ReferenceManager<std::shared_ptr<IndexSearcher>>
{
  GET_CLASS_NAME(SearcherManager)

private:
  const std::shared_ptr<SearcherFactory> searcherFactory;

  /**
   * Creates and returns a new SearcherManager from the given
   * {@link IndexWriter}.
   *
   * @param writer
   *          the IndexWriter to open the IndexReader from.
   * @param searcherFactory
   *          An optional {@link SearcherFactory}. Pass <code>null</code> if you
   *          don't require the searcher to be warmed before going live or other
   *          custom behavior.
   *
   * @throws IOException if there is a low-level I/O error
   */
public:
  SearcherManager(
      std::shared_ptr<IndexWriter> writer,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

  /**
   * Expert: creates and returns a new SearcherManager from the given
   * {@link IndexWriter}, controlling whether past deletions should be applied.
   *
   * @param writer
   *          the IndexWriter to open the IndexReader from.
   * @param applyAllDeletes
   *          If <code>true</code>, all buffered deletes will be applied (made
   *          visible) in the {@link IndexSearcher} / {@link DirectoryReader}.
   *          If <code>false</code>, the deletes may or may not be applied, but
   *          remain buffered (in IndexWriter) so that they will be applied in
   *          the future. Applying deletes can be costly, so if your app can
   *          tolerate deleted documents being returned you might gain some
   *          performance by passing <code>false</code>. See
   *          {@link DirectoryReader#openIfChanged(DirectoryReader, IndexWriter,
   * bool)}.
   * @param writeAllDeletes
   *          If <code>true</code>, new deletes will be forcefully written to
   * index files.
   * @param searcherFactory
   *          An optional {@link SearcherFactory}. Pass <code>null</code> if you
   *          don't require the searcher to be warmed before going live or other
   *          custom behavior.
   *
   * @throws IOException if there is a low-level I/O error
   */
  SearcherManager(
      std::shared_ptr<IndexWriter> writer, bool applyAllDeletes,
      bool writeAllDeletes,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

  /**
   * Creates and returns a new SearcherManager from the given {@link Directory}.
   * @param dir the directory to open the DirectoryReader on.
   * @param searcherFactory An optional {@link SearcherFactory}. Pass
   *        <code>null</code> if you don't require the searcher to be warmed
   *        before going live or other custom behavior.
   *
   * @throws IOException if there is a low-level I/O error
   */
  SearcherManager(
      std::shared_ptr<Directory> dir,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

  /**
   * Creates and returns a new SearcherManager from an existing {@link
   * DirectoryReader}.  Note that this steals the incoming reference.
   *
   * @param reader the DirectoryReader.
   * @param searcherFactory An optional {@link SearcherFactory}. Pass
   *        <code>null</code> if you don't require the searcher to be warmed
   *        before going live or other custom behavior.
   *
   * @throws IOException if there is a low-level I/O error
   */
  SearcherManager(
      std::shared_ptr<DirectoryReader> reader,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

protected:
  void
  decRef(std::shared_ptr<IndexSearcher> reference)  override;

  std::shared_ptr<IndexSearcher>
  refreshIfNeeded(std::shared_ptr<IndexSearcher> referenceToRefresh) throw(
      IOException) override;

  bool tryIncRef(std::shared_ptr<IndexSearcher> reference) override;

  int getRefCount(std::shared_ptr<IndexSearcher> reference) override;

  /**
   * Returns <code>true</code> if no changes have occured since this searcher
   * ie. reader was opened, otherwise <code>false</code>.
   * @see DirectoryReader#isCurrent()
   */
public:
  bool isSearcherCurrent() ;

  /** Expert: creates a searcher from the provided {@link
   *  IndexReader} using the provided {@link
   *  SearcherFactory}.  NOTE: this decRefs incoming reader
   * on throwing an exception. */
  static std::shared_ptr<IndexSearcher>
  getSearcher(std::shared_ptr<SearcherFactory> searcherFactory,
              std::shared_ptr<IndexReader> reader,
              std::shared_ptr<IndexReader> previousReader) ;

protected:
  std::shared_ptr<SearcherManager> shared_from_this()
  {
    return std::static_pointer_cast<SearcherManager>(
        ReferenceManager<IndexSearcher>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
