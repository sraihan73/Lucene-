#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy
{
class SearcherAndTaxonomy;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class DirectoryTaxonomyReader;
}
namespace org::apache::lucene::search
{
class SearcherFactory;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class DirectoryTaxonomyWriter;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::facet::taxonomy
{

using DirectoryTaxonomyReader =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyReader;
using DirectoryTaxonomyWriter =
    org::apache::lucene::facet::taxonomy::directory::DirectoryTaxonomyWriter;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using Directory = org::apache::lucene::store::Directory;

/**
 * Manages near-real-time reopen of both an IndexSearcher
 * and a TaxonomyReader.
 *
 * <p><b>NOTE</b>: If you call {@link
 * DirectoryTaxonomyWriter#replaceTaxonomy} then you must
 * open a new {@code SearcherTaxonomyManager} afterwards.
 */
class SearcherTaxonomyManager
    : public ReferenceManager<
          std::shared_ptr<SearcherTaxonomyManager::SearcherAndTaxonomy>>
{
  GET_CLASS_NAME(SearcherTaxonomyManager)

  /** Holds a matched pair of {@link IndexSearcher} and
   *  {@link TaxonomyReader} */
public:
  class SearcherAndTaxonomy
      : public std::enable_shared_from_this<SearcherAndTaxonomy>
  {
    GET_CLASS_NAME(SearcherAndTaxonomy)
    /** Point-in-time {@link IndexSearcher}. */
  public:
    const std::shared_ptr<IndexSearcher> searcher;

    /** Matching point-in-time {@link DirectoryTaxonomyReader}. */
    const std::shared_ptr<DirectoryTaxonomyReader> taxonomyReader;

    /** Create a SearcherAndTaxonomy */
    SearcherAndTaxonomy(
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<DirectoryTaxonomyReader> taxonomyReader);
  };

private:
  const std::shared_ptr<SearcherFactory> searcherFactory;
  const int64_t taxoEpoch;
  const std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter;

  /** Creates near-real-time searcher and taxonomy reader
   *  from the corresponding writers. */
public:
  SearcherTaxonomyManager(
      std::shared_ptr<IndexWriter> writer,
      std::shared_ptr<SearcherFactory> searcherFactory,
      std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter) ;

  /** Expert: creates near-real-time searcher and taxonomy reader
   *  from the corresponding writers, controlling whether deletes should be
   * applied. */
  SearcherTaxonomyManager(
      std::shared_ptr<IndexWriter> writer, bool applyAllDeletes,
      std::shared_ptr<SearcherFactory> searcherFactory,
      std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter) ;

  /**
   * Creates search and taxonomy readers over the corresponding directories.
   *
   * <p>
   * <b>NOTE:</b> you should only use this constructor if you commit and call
   * {@link #maybeRefresh()} in the same thread. Otherwise it could lead to an
   * unsync'd {@link IndexSearcher} and {@link TaxonomyReader} pair.
   */
  SearcherTaxonomyManager(
      std::shared_ptr<Directory> indexDir, std::shared_ptr<Directory> taxoDir,
      std::shared_ptr<SearcherFactory> searcherFactory) ;

protected:
  void
  decRef(std::shared_ptr<SearcherAndTaxonomy> ref)  override;

  bool tryIncRef(std::shared_ptr<SearcherAndTaxonomy> ref) throw(
      IOException) override;

  std::shared_ptr<SearcherAndTaxonomy> refreshIfNeeded(
      std::shared_ptr<SearcherAndTaxonomy> ref)  override;

  int getRefCount(std::shared_ptr<SearcherAndTaxonomy> reference) override;

protected:
  std::shared_ptr<SearcherTaxonomyManager> shared_from_this()
  {
    return std::static_pointer_cast<SearcherTaxonomyManager>(
        org.apache.lucene.search.ReferenceManager<
            SearcherTaxonomyManager.SearcherAndTaxonomy>::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
