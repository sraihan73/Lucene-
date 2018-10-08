#pragma once
#include "FacetSource.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}

namespace org::apache::lucene::facet
{
class FacetField;
}
namespace org::apache::lucene::facet
{
class FacetsConfig;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
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

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using FacetField = org::apache::lucene::facet::FacetField;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;

/**
 * Simple implementation of a random facet source
 * <p>
 * Supports the following parameters:
 * <ul>
 * <li><b>rand.seed</b> - defines the seed to initialize {@link Random} with
 * (default: <b>13</b>).
 * <li><b>max.doc.facet.dims</b> - Max number of random dimensions to
 * create (default: <b>5</b>); actual number of dimensions
 * would be anything between 1 and that number.
 * <li><b>max.doc.facets</b> - maximal #facets per doc (default: <b>10</b>).
 * Actual number of facets in a certain doc would be anything between 1 and that
 * number.
 * <li><b>max.facet.depth</b> - maximal #components in a facet (default:
 * <b>3</b>). Actual number of components in a certain facet would be anything
 * between 1 and that number.
 * </ul>
 */
class RandomFacetSource : public FacetSource
{
  GET_CLASS_NAME(RandomFacetSource)

private:
  std::shared_ptr<Random> random;
  int maxDocFacets = 0;
  int maxFacetDepth = 0;
  int maxDims = 0;
  int maxValue = maxDocFacets * maxFacetDepth;

public:
  void getNextFacets(std::deque<std::shared_ptr<FacetField>> &facets) throw(
      NoMoreDataException, IOException) override;

  void configure(std::shared_ptr<FacetsConfig> config) override;

  virtual ~RandomFacetSource();

  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<RandomFacetSource> shared_from_this()
  {
    return std::static_pointer_cast<RandomFacetSource>(
        FacetSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
