#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"

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
namespace org::apache::lucene::facet
{

using CollectorManager = org::apache::lucene::search::CollectorManager;

/**
 * A {@link CollectorManager} implementation which produces FacetsCollector and
 * produces a merged FacetsCollector. This is used for concurrent
 * FacetsCollection.
 */
class FacetsCollectorManager
    : public std::enable_shared_from_this<FacetsCollectorManager>,
      public CollectorManager<std::shared_ptr<FacetsCollector>,
                              std::shared_ptr<FacetsCollector>>
{
  GET_CLASS_NAME(FacetsCollectorManager)

  /** Sole constructor. */
public:
  FacetsCollectorManager();

  std::shared_ptr<FacetsCollector> newCollector()  override;

  std::shared_ptr<FacetsCollector>
  reduce(std::shared_ptr<std::deque<std::shared_ptr<FacetsCollector>>>
             collectors)  override;

private:
  class ReducedFacetsCollector : public FacetsCollector
  {
    GET_CLASS_NAME(ReducedFacetsCollector)

  public:
    ReducedFacetsCollector(
        std::shared_ptr<std::deque<std::shared_ptr<FacetsCollector>>>
            facetsCollectors);

  protected:
    std::shared_ptr<ReducedFacetsCollector> shared_from_this()
    {
      return std::static_pointer_cast<ReducedFacetsCollector>(
          FacetsCollector::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/facet/
