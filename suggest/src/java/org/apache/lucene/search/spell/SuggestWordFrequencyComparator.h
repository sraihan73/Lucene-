#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spell
{
class SuggestWord;
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
namespace org::apache::lucene::search::spell
{

/**
 *  Frequency first, then score.
 *
 **/
class SuggestWordFrequencyComparator
    : public std::enable_shared_from_this<SuggestWordFrequencyComparator>,
      public Comparator<std::shared_ptr<SuggestWord>>
{
  GET_CLASS_NAME(SuggestWordFrequencyComparator)

  /**
   * Creates a new comparator that will compare by {@link SuggestWord#freq},
   * then by {@link SuggestWord#score}, then by {@link SuggestWord#string}.
   */
public:
  SuggestWordFrequencyComparator();

  int compare(std::shared_ptr<SuggestWord> first,
              std::shared_ptr<SuggestWord> second) override;
};

} // namespace org::apache::lucene::search::spell
