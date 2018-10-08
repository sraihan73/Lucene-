#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class BulkScorer;
}

namespace org::apache::lucene::search
{
class LeafCollector;
}
namespace org::apache::lucene::util
{
class Bits;
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

using Bits = org::apache::lucene::util::Bits;

/** Wraps a Scorer with additional checks */
class AssertingBulkScorer final : public BulkScorer
{
  GET_CLASS_NAME(AssertingBulkScorer)

public:
  static std::shared_ptr<BulkScorer> wrap(std::shared_ptr<Random> random,
                                          std::shared_ptr<BulkScorer> other,
                                          int maxDoc);

  const std::shared_ptr<Random> random;
  const std::shared_ptr<BulkScorer> in_;
  const int maxDoc;
  int max = 0;

private:
  AssertingBulkScorer(std::shared_ptr<Random> random,
                      std::shared_ptr<BulkScorer> in_, int maxDoc);

public:
  std::shared_ptr<BulkScorer> getIn();

  int64_t cost() override;

  void score(std::shared_ptr<LeafCollector> collector,
             std::shared_ptr<Bits> acceptDocs)  override;

  int score(std::shared_ptr<LeafCollector> collector,
            std::shared_ptr<Bits> acceptDocs, int min,
            int const max)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<AssertingBulkScorer> shared_from_this()
  {
    return std::static_pointer_cast<AssertingBulkScorer>(
        BulkScorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
