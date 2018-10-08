#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::vectorhighlight
{
class FieldPhraseList;
}

namespace org::apache::lucene::search::vectorhighlight
{
class WeightedPhraseInfo;
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
namespace org::apache::lucene::search::vectorhighlight
{

using WeightedPhraseInfo = org::apache::lucene::search::vectorhighlight::
    FieldPhraseList::WeightedPhraseInfo;

/**
 * A simple implementation of {@link FieldFragList}.
 */
class SimpleFieldFragList : public FieldFragList
{
  GET_CLASS_NAME(SimpleFieldFragList)

  /**
   * a constructor.
   *
   * @param fragCharSize the length (number of chars) of a fragment
   */
public:
  SimpleFieldFragList(int fragCharSize);

  /* (non-Javadoc)
   * @see org.apache.lucene.search.vectorhighlight.FieldFragList#add( int
   * startOffset, int endOffset, List<WeightedPhraseInfo> phraseInfoList )
   */
  void add(int startOffset, int endOffset,
           std::deque<std::shared_ptr<WeightedPhraseInfo>> &phraseInfoList)
      override;

protected:
  std::shared_ptr<SimpleFieldFragList> shared_from_this()
  {
    return std::static_pointer_cast<SimpleFieldFragList>(
        FieldFragList::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::vectorhighlight
