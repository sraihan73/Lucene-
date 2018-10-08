#pragma once
#include "stringhelper.h"
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
 * Set of strategies for suggesting related terms
 * @lucene.experimental
 */
enum class SuggestMode {
  GET_CLASS_NAME(SuggestMode)
  /**
   * Generate suggestions only for terms not in the index (default)
   */
  SUGGEST_WHEN_NOT_IN_INDEX,

  /**
   * Return only suggested words that are as frequent or more frequent than the
   * searched word
   */
  SUGGEST_MORE_POPULAR,

  /**
   * Always attempt to offer suggestions (however, other parameters may limit
   * suggestions. For example, see
   * {@link DirectSpellChecker#setMaxQueryFrequency(float)} ).
   */
  SUGGEST_ALWAYS
};

} // namespace org::apache::lucene::search::spell
