#pragma once
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/dict/CharacterDefinition.h"
#include "stringhelper.h"
#include "tangible_filesystem.h"
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
namespace org::apache::lucene::analysis::ko::util
{

using CharacterDefinition =
    org::apache::lucene::analysis::ko::dict::CharacterDefinition;

class CharacterDefinitionWriter final
    : public std::enable_shared_from_this<CharacterDefinitionWriter>
{
  GET_CLASS_NAME(CharacterDefinitionWriter)

private:
  std::deque<char> const characterCategoryMap = std::deque<char>(0x10000);

  std::deque<bool> const invokeMap =
      std::deque<bool>(CharacterDefinition::CLASS_COUNT);
  std::deque<bool> const groupMap =
      std::deque<bool>(CharacterDefinition::CLASS_COUNT);

  /**
   * Constructor for building. TODO: remove write access
   */
public:
  CharacterDefinitionWriter();

  /**
   * Put mapping from unicode code point to character class.
   *
   * @param codePoint
   *            code point
   * @param characterClassName character class name
   */
  void putCharacterCategory(int codePoint,
                            const std::wstring &characterClassName);

  void putInvokeDefinition(const std::wstring &characterClassName, int invoke,
                           int group, int length);

  void write(const std::wstring &baseDir) ;
};

} // namespace org::apache::lucene::analysis::ko::util
