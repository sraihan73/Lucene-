#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldPhraseList.h"

#include  "core/src/java/org/apache/lucene/search/vectorhighlight/WeightedPhraseInfo.h"

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

using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

/**
 * A weighted implementation of {@link FieldFragList}.
 */
class WeightedFieldFragList : public FieldFragList
{
  GET_CLASS_NAME(WeightedFieldFragList)

  /**
   * a constructor.
   *
   * @param fragCharSize the length (number of chars) of a fragment
   */
public:
  WeightedFieldFragList(int fragCharSize);

  /* (non-Javadoc)
   * @see org.apache.lucene.search.vectorhighlight.FieldFragList#add( int
   * startOffset, int endOffset, List<WeightedPhraseInfo> phraseInfoList )
   */
  void add(int startOffset, int endOffset,
           std::deque<std::shared_ptr<WeightedPhraseInfo>> &phraseInfoList)
      override;

protected:
  std::shared_ptr<WeightedFieldFragList> shared_from_this()
  {
    return std::static_pointer_cast<WeightedFieldFragList>(
        FieldFragList::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
