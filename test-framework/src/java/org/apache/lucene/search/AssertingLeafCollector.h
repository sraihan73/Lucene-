#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Scorer;
}

namespace org::apache::lucene::search
{
class LeafCollector;
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
namespace org::apache::lucene::search
{

/** Wraps another Collector and checks that
 *  order is respected. */
class AssertingLeafCollector : public FilterLeafCollector
{
  GET_CLASS_NAME(AssertingLeafCollector)

private:
  const std::shared_ptr<Random> random;
  const int min;
  const int max;

  std::shared_ptr<Scorer> scorer;
  int lastCollected = -1;

public:
  AssertingLeafCollector(std::shared_ptr<Random> random,
                         std::shared_ptr<LeafCollector> collector, int min,
                         int max);

  void setScorer(std::shared_ptr<Scorer> scorer)  override;

  void collect(int doc)  override;

protected:
  std::shared_ptr<AssertingLeafCollector> shared_from_this()
  {
    return std::static_pointer_cast<AssertingLeafCollector>(
        FilterLeafCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
