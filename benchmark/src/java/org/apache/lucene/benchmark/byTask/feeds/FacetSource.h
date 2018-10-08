#pragma once
#include "ContentItemsSource.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class FacetField;
}

namespace org::apache::lucene::facet
{
class FacetsConfig;
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/**
 * Source items for facets.
 * <p>
 * For supported configuration parameters see {@link ContentItemsSource}.
 */
class FacetSource : public ContentItemsSource
{
  GET_CLASS_NAME(FacetSource)

  /**
   * Fills the next facets content items in the given deque. Implementations must
   * account for multi-threading, as multiple threads can call this method
   * simultaneously.
   */
public:
  virtual void
  getNextFacets(std::deque<std::shared_ptr<FacetField>> &facets) = 0;

  virtual void configure(std::shared_ptr<FacetsConfig> config) = 0;

  void resetInputs()  override;

protected:
  std::shared_ptr<FacetSource> shared_from_this()
  {
    return std::static_pointer_cast<FacetSource>(
        ContentItemsSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
