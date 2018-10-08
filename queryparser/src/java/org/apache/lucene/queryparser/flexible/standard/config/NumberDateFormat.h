#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

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
namespace org::apache::lucene::queryparser::flexible::standard::config
{

/**
 * This {@link Format} parses {@link Long} into date strings and vice-versa. It
 * uses the given {@link DateFormat} to parse and format dates, but before, it
 * converts {@link Long} to {@link Date} objects or vice-versa.
 */
class NumberDateFormat : public NumberFormat
{
  GET_CLASS_NAME(NumberDateFormat)

private:
  static constexpr int64_t serialVersionUID = 964823936071308283LL;

  const std::shared_ptr<DateFormat> dateFormat;

  /**
   * Constructs a {@link NumberDateFormat} object using the given {@link
   * DateFormat}.
   *
   * @param dateFormat {@link DateFormat} used to parse and format dates
   */
public:
  NumberDateFormat(std::shared_ptr<DateFormat> dateFormat);

  std::shared_ptr<StringBuilder>
  format(double number, std::shared_ptr<StringBuilder> toAppendTo,
         std::shared_ptr<FieldPosition> pos) override;

  std::shared_ptr<StringBuilder>
  format(int64_t number, std::shared_ptr<StringBuilder> toAppendTo,
         std::shared_ptr<FieldPosition> pos) override;

  std::shared_ptr<Number>
  parse(const std::wstring &source,
        std::shared_ptr<ParsePosition> parsePosition) override;

  std::shared_ptr<StringBuilder>
  format(std::any number, std::shared_ptr<StringBuilder> toAppendTo,
         std::shared_ptr<FieldPosition> pos) override;

protected:
  std::shared_ptr<NumberDateFormat> shared_from_this()
  {
    return std::static_pointer_cast<NumberDateFormat>(
        java.text.NumberFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/standard/config/
