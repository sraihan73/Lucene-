#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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
namespace org::apache::lucene::util
{

/**
 * Helper methods to ease implementing {@link Object#toString()}.
 */
class ToStringUtils final : public std::enable_shared_from_this<ToStringUtils>
{
  GET_CLASS_NAME(ToStringUtils)

private:
  ToStringUtils();

public:
  static void byteArray(std::shared_ptr<StringBuilder> buffer,
                        std::deque<char> &bytes);

private:
  static std::deque<wchar_t> const HEX;

public:
  static std::wstring longHex(int64_t x);
};

} // namespace org::apache::lucene::util
