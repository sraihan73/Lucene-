#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/NameIntCacheLRU.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/FacetLabel.h"

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
namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;

/**
 * LRU {@link TaxonomyWriterCache} - good choice for huge taxonomies.
 *
 * @lucene.experimental
 */
class LruTaxonomyWriterCache
    : public std::enable_shared_from_this<LruTaxonomyWriterCache>,
      public TaxonomyWriterCache
{
  GET_CLASS_NAME(LruTaxonomyWriterCache)

  /**
   * Determines cache type.
   * For guaranteed correctness - not relying on no-collisions in the hash
   * function, LRU_STRING should be used.
   */
public:
  enum class LRUType {
    GET_CLASS_NAME(LRUType)
    /** Use only the label's 64 bit longHashCode as the hash key. Do not
     *  check equals, unlike most hash maps.
     *  Note that while these hashes are very likely to be unique, the chance
     *  of a collision is still greater than zero. If such an unlikely event
     *  occurs, your document will get an incorrect facet.
     */
    LRU_HASHED,

    /** Use the label as the hash key; this is always
     *  correct but will usually use more RAM. */
    LRU_STRING
  };

private:
  std::shared_ptr<NameIntCacheLRU> cache;

  /** Creates this with {@link LRUType#LRU_STRING} method. */
public:
  LruTaxonomyWriterCache(int cacheSize);

  /** Creates this with the specified method. */
  LruTaxonomyWriterCache(int cacheSize, LRUType lruType);

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool isFull() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void clear() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~LruTaxonomyWriterCache();

  int size() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  int get(std::shared_ptr<FacetLabel> categoryPath) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool put(std::shared_ptr<FacetLabel> categoryPath, int ordinal) override;
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/
