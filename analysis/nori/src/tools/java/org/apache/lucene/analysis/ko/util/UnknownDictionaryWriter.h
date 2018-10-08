#pragma once
#include "BinaryDictionaryWriter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/util/CharacterDefinitionWriter.h"

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

class UnknownDictionaryWriter : public BinaryDictionaryWriter
{
  GET_CLASS_NAME(UnknownDictionaryWriter)

private:
  const std::shared_ptr<CharacterDefinitionWriter> characterDefinition =
      std::make_shared<CharacterDefinitionWriter>();

public:
  UnknownDictionaryWriter(int size);

  int put(std::deque<std::wstring> &entry) override;

  /**
   * Put mapping from unicode code point to character class.
   *
   * @param codePoint code point
   * @param characterClassName character class name
   */
  virtual void putCharacterCategory(int codePoint,
                                    const std::wstring &characterClassName);

  virtual void putInvokeDefinition(const std::wstring &characterClassName,
                                   int invoke, int group, int length);

  void write(const std::wstring &baseDir)  override;

protected:
  std::shared_ptr<UnknownDictionaryWriter> shared_from_this()
  {
    return std::static_pointer_cast<UnknownDictionaryWriter>(
        BinaryDictionaryWriter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/util/
