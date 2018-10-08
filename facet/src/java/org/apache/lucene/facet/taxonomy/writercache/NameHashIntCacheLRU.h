#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
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
namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

/**
 * An an LRU cache of mapping from name to int.
 * Used to cache Ordinals of category paths.
 * It uses as key, hash of the path instead of the path.
 * This way the cache takes less RAM, but correctness depends on
 * assuming no collisions.
 *
 * @lucene.experimental
 */
class NameHashIntCacheLRU : public NameIntCacheLRU
{
  GET_CLASS_NAME(NameHashIntCacheLRU)

public:
  NameHashIntCacheLRU(int maxCacheSize);

  std::any key(std::shared_ptr<FacetLabel> name) override;

  std::any key(std::shared_ptr<FacetLabel> name, int prefixLen) override;

protected:
  std::shared_ptr<NameHashIntCacheLRU> shared_from_this()
  {
    return std::static_pointer_cast<NameHashIntCacheLRU>(
        NameIntCacheLRU::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy::writercache
