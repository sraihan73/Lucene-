#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/SegToken.h"

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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * <p>
 * Filters a {@link SegToken} by converting full-width latin to half-width, then
 * lowercasing latin. Additionally, all punctuation is converted into {@link
 * Utility#COMMON_DELIMITER}
 * </p>
 * @lucene.experimental
 */
class SegTokenFilter : public std::enable_shared_from_this<SegTokenFilter>
{
  GET_CLASS_NAME(SegTokenFilter)

  /**
   * Filter an input {@link SegToken}
   * <p>
   * Full-width latin will be converted to half-width, then all latin will be
   * lowercased. All punctuation is converted into {@link
   * Utility#COMMON_DELIMITER}
   * </p>
   *
   * @param token input {@link SegToken}
   * @return normalized {@link SegToken}
   */
public:
  virtual std::shared_ptr<SegToken> filter(std::shared_ptr<SegToken> token);
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
