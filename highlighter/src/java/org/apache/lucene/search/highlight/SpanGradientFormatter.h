#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/TokenGroup.h"

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
namespace org::apache::lucene::search::highlight
{
/**
 * Formats text with different color intensity depending on the score of the
 * term using the span tag.  GradientFormatter uses a bgcolor argument to the
 * font tag which doesn't work in Mozilla, thus this class.
 *
 * @see GradientFormatter
 */

class SpanGradientFormatter : public GradientFormatter
{
  GET_CLASS_NAME(SpanGradientFormatter)
public:
  SpanGradientFormatter(float maxScore, const std::wstring &minForegroundColor,
                        const std::wstring &maxForegroundColor,
                        const std::wstring &minBackgroundColor,
                        const std::wstring &maxBackgroundColor);

  std::wstring highlightTerm(const std::wstring &originalText,
                             std::shared_ptr<TokenGroup> tokenGroup) override;

  // guess how much extra text we'll add to the text we're highlighting to try
  // to avoid a  StringBuilder resize
private:
  static const std::wstring TEMPLATE;
  static const int EXTRA = TEMPLATE.length();

protected:
  std::shared_ptr<SpanGradientFormatter> shared_from_this()
  {
    return std::static_pointer_cast<SpanGradientFormatter>(
        GradientFormatter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
