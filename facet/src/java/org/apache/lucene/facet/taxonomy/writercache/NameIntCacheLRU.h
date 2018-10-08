#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
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

/**
 * An an LRU cache of mapping from name to int.
 * Used to cache Ordinals of category paths.
 *
 * @lucene.experimental
 */
// Note: Nothing in this class is synchronized. The caller is assumed to be
// synchronized so that no two methods of this class are called concurrently.
class NameIntCacheLRU : public std::enable_shared_from_this<NameIntCacheLRU>
{
  GET_CLASS_NAME(NameIntCacheLRU)

private:
  std::unordered_map<std::any, int> cache;

public:
  int64_t nMisses = 0; // for debug
  int64_t nHits = 0;   // for debug
private:
  int maxCacheSize = 0;

public:
  NameIntCacheLRU(int maxCacheSize);

  /** Maximum number of cache entries before eviction. */
  virtual int getMaxSize();

  /** Number of entries currently in the cache. */
  virtual int getSize();

private:
  void createCache(int maxSize);

public:
  virtual std::optional<int> get(std::shared_ptr<FacetLabel> name);

  /** Subclasses can override this to provide caching by e.g. hash of the
   * string. */
  virtual std::any key(std::shared_ptr<FacetLabel> name);

  virtual std::any key(std::shared_ptr<FacetLabel> name, int prefixLen);

  /**
   * Add a new value to cache.
   * Return true if cache became full and some room need to be made.
   */
  virtual bool put(std::shared_ptr<FacetLabel> name, std::optional<int> &val);

  virtual bool put(std::shared_ptr<FacetLabel> name, int prefixLen,
                   std::optional<int> &val);

private:
  bool isCacheFull();

public:
  virtual void clear();

  virtual std::wstring stats();

  /**
   * If cache is full remove least recently used entries from cache. Return true
   * if anything was removed, false otherwise.
   *
   * See comment in DirectoryTaxonomyWriter.addToCache(CategoryPath, int) for an
   * explanation why we clean 2/3rds of the cache, and not just one entry.
   */
  virtual bool makeRoomLRU();
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/
