#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
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
namespace org::apache::lucene::util
{

/**
 * Converts numbers to english strings for testing.
 * @lucene.internal
 */
class English final : public std::enable_shared_from_this<English>
{
  GET_CLASS_NAME(English)

private:
  English();

public:
  static std::wstring longToEnglish(int64_t i);

  static void longToEnglish(int64_t i, std::shared_ptr<StringBuilder> result);

  static std::wstring intToEnglish(int i);

  static void intToEnglish(int i, std::shared_ptr<StringBuilder> result);
};

} // namespace org::apache::lucene::util
