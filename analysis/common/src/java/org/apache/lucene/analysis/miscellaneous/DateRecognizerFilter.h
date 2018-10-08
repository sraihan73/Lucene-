#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::miscellaneous
{

using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/** Filters all tokens that cannot be parsed to a date, using the provided
 * {@link DateFormat}. */
class DateRecognizerFilter : public FilteringTokenFilter
{
  GET_CLASS_NAME(DateRecognizerFilter)

public:
  static const std::wstring DATE_TYPE;

private:
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<DateFormat> dateFormat;

  /**
   * Uses {@link DateFormat#DEFAULT} and {@link Locale#ENGLISH} to create a
   * {@link DateFormat} instance.
   */
public:
  DateRecognizerFilter(std::shared_ptr<TokenStream> input);

  DateRecognizerFilter(std::shared_ptr<TokenStream> input,
                       std::shared_ptr<DateFormat> dateFormat);

  bool accept() override;

protected:
  std::shared_ptr<DateRecognizerFilter> shared_from_this()
  {
    return std::static_pointer_cast<DateRecognizerFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
