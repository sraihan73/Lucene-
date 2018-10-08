#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/BoundaryScanner.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldFragList.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedFragInfo.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;

/**
 * An implementation of FragmentsBuilder that outputs score-order fragments.
 */
class ScoreOrderFragmentsBuilder : public BaseFragmentsBuilder
{
  GET_CLASS_NAME(ScoreOrderFragmentsBuilder)

  /**
   * a constructor.
   */
public:
  ScoreOrderFragmentsBuilder();

  /**
   * a constructor.
   *
   * @param preTags array of pre-tags for markup terms.
   * @param postTags array of post-tags for markup terms.
   */
  ScoreOrderFragmentsBuilder(std::deque<std::wstring> &preTags,
                             std::deque<std::wstring> &postTags);

  ScoreOrderFragmentsBuilder(std::shared_ptr<BoundaryScanner> bs);

  ScoreOrderFragmentsBuilder(std::deque<std::wstring> &preTags,
                             std::deque<std::wstring> &postTags,
                             std::shared_ptr<BoundaryScanner> bs);

  /**
   * Sort by score the deque of WeightedFragInfo
   */
  std::deque<std::shared_ptr<WeightedFragInfo>> getWeightedFragInfoList(
      std::deque<std::shared_ptr<WeightedFragInfo>> &src) override;

  /**
   * Comparator for {@link WeightedFragInfo} by boost, breaking ties
   * by offset.
   */
public:
  class ScoreComparator : public std::enable_shared_from_this<ScoreComparator>,
                          public Comparator<std::shared_ptr<WeightedFragInfo>>
  {
    GET_CLASS_NAME(ScoreComparator)

  public:
    int compare(std::shared_ptr<WeightedFragInfo> o1,
                std::shared_ptr<WeightedFragInfo> o2) override;
  };

protected:
  std::shared_ptr<ScoreOrderFragmentsBuilder> shared_from_this()
  {
    return std::static_pointer_cast<ScoreOrderFragmentsBuilder>(
        BaseFragmentsBuilder::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
