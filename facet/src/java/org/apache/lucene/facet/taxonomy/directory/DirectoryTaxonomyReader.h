#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy::directory
{
class DirectoryTaxonomyWriter;
}

namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
}
namespace org::apache::lucene::facet::taxonomy::directory
{
class TaxonomyIndexArrays;
}
namespace org::apache::lucene::facet::taxonomy
{
template <typename Ktypename V>
class LRUHashMap;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::facet::taxonomy
{
class ParallelTaxonomyArrays;
}
namespace org::apache::lucene::util
{
class Accountable;
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

using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using LRUHashMap = org::apache::lucene::facet::taxonomy::LRUHashMap;
using ParallelTaxonomyArrays =
    org::apache::lucene::facet::taxonomy::ParallelTaxonomyArrays;
using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * A {@link TaxonomyReader} which retrieves stored taxonomy information from a
 * {@link Directory}.
 * <P>
 * Reading from the on-disk index on every method call is too slow, so this
 * implementation employs caching: Some methods cache recent requests and their
 * results, while other methods prefetch all the data into memory and then
 * provide answers directly from in-memory tables. See the documentation of
 * individual methods for comments on their performance.
 *
 * @lucene.experimental
 */
class DirectoryTaxonomyReader : public TaxonomyReader, public Accountable
{
  GET_CLASS_NAME(DirectoryTaxonomyReader)

private:
  static const std::shared_ptr<Logger> logger;

  static constexpr int DEFAULT_CACHE_VALUE = 4000;

  // NOTE: very coarse estimate!
  static const int BYTES_PER_CACHE_ENTRY =
      4 * RamUsageEstimator::NUM_BYTES_OBJECT_REF +
      4 * RamUsageEstimator::NUM_BYTES_OBJECT_HEADER +
      8 * RamUsageEstimator::NUM_BYTES_CHAR;

  const std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter;
  const int64_t taxoEpoch; // used in doOpenIfChanged
  const std::shared_ptr<DirectoryReader> indexReader;

  // TODO: test DoubleBarrelLRUCache and consider using it instead
  std::shared_ptr<LRUHashMap<std::shared_ptr<FacetLabel>, int>> ordinalCache;
  std::shared_ptr<LRUHashMap<int, std::shared_ptr<FacetLabel>>> categoryCache;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile TaxonomyIndexArrays taxoArrays;
  std::shared_ptr<TaxonomyIndexArrays> taxoArrays;

  /**
   * Called only from {@link #doOpenIfChanged()}. If the taxonomy has been
   * recreated, you should pass {@code null} as the caches and parent/children
   * arrays.
   */
public:
  DirectoryTaxonomyReader(
      std::shared_ptr<DirectoryReader> indexReader,
      std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter,
      std::shared_ptr<LRUHashMap<std::shared_ptr<FacetLabel>, int>>
          ordinalCache,
      std::shared_ptr<LRUHashMap<int, std::shared_ptr<FacetLabel>>>
          categoryCache,
      std::shared_ptr<TaxonomyIndexArrays> taxoArrays) ;

  /**
   * Open for reading a taxonomy stored in a given {@link Directory}.
   *
   * @param directory
   *          The {@link Directory} in which the taxonomy resides.
   * @throws CorruptIndexException
   *           if the Taxonomy is corrupt.
   * @throws IOException
   *           if another error occurred.
   */
  DirectoryTaxonomyReader(std::shared_ptr<Directory> directory) throw(
      IOException);

  /**
   * Opens a {@link DirectoryTaxonomyReader} over the given
   * {@link DirectoryTaxonomyWriter} (for NRT).
   *
   * @param taxoWriter
   *          The {@link DirectoryTaxonomyWriter} from which to obtain newly
   *          added categories, in real-time.
   */
  DirectoryTaxonomyReader(
      std::shared_ptr<DirectoryTaxonomyWriter> taxoWriter) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void initTaxoArrays() ;

protected:
  void doClose()  override;

  /**
   * Implements the opening of a new {@link DirectoryTaxonomyReader} instance if
   * the taxonomy has changed.
   *
   * <p>
   * <b>NOTE:</b> the returned {@link DirectoryTaxonomyReader} shares the
   * ordinal and category caches with this reader. This is not expected to cause
   * any issues, unless the two instances continue to live. The reader
   * guarantees that the two instances cannot affect each other in terms of
   * correctness of the caches, however if the size of the cache is changed
   * through {@link #setCacheSize(int)}, it will affect both reader instances.
   */
  std::shared_ptr<DirectoryTaxonomyReader>
  doOpenIfChanged()  override;

  /** Open the {@link DirectoryReader} from this {@link
   *  Directory}. */
  virtual std::shared_ptr<DirectoryReader>
  openIndexReader(std::shared_ptr<Directory> directory) ;

  /** Open the {@link DirectoryReader} from this {@link
   *  IndexWriter}. */
  virtual std::shared_ptr<DirectoryReader>
  openIndexReader(std::shared_ptr<IndexWriter> writer) ;

  /**
   * Expert: returns the underlying {@link DirectoryReader} instance that is
   * used by this {@link TaxonomyReader}.
   */
public:
  virtual std::shared_ptr<DirectoryReader> getInternalIndexReader();

  std::shared_ptr<ParallelTaxonomyArrays>
  getParallelTaxonomyArrays()  override;

  std::unordered_map<std::wstring, std::wstring>
  getCommitUserData()  override;

  int getOrdinal(std::shared_ptr<FacetLabel> cp)  override;

  std::shared_ptr<FacetLabel> getPath(int ordinal)  override;

  int getSize() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  /**
   * setCacheSize controls the maximum allowed size of each of the caches
   * used by {@link #getPath(int)} and {@link #getOrdinal(FacetLabel)}.
   * <P>
   * Currently, if the given size is smaller than the current size of
   * a cache, it will not shrink, and rather we be limited to its current
   * size.
   * @param size the new maximum cache size, in number of entries.
   */
  virtual void setCacheSize(int size);

  /** Returns ordinal -&gt; label mapping, up to the provided
   *  max ordinal or number of ordinals, whichever is
   *  smaller. */
  virtual std::wstring toString(int max);

protected:
  std::shared_ptr<DirectoryTaxonomyReader> shared_from_this()
  {
    return std::static_pointer_cast<DirectoryTaxonomyReader>(
        org.apache.lucene.facet.taxonomy.TaxonomyReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy::directory
