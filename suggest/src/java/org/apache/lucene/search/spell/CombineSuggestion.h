#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spell/SuggestWord.h"

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
 * <p>A suggestion generated by combining one or more original query terms</p>
 */
class CombineSuggestion : public std::enable_shared_from_this<CombineSuggestion>
{
  GET_CLASS_NAME(CombineSuggestion)
  /**
   * <p>The indexes from the passed-in array of terms used to make this word
   * combination</p>
   */
public:
  std::deque<int> const originalTermIndexes;
  /**
   * <p>The word combination suggestion</p>
   */
  const std::shared_ptr<SuggestWord> suggestion;

  /**
   * Creates a new CombineSuggestion from a <code>suggestion</code> and
   * an array of term ids (referencing the indexes to the original terms that
   * form this combined suggestion)
   */
  CombineSuggestion(std::shared_ptr<SuggestWord> suggestion,
                    std::deque<int> &originalTermIndexes);
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
