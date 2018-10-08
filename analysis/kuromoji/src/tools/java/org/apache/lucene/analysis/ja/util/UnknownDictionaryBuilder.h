#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::util
{
class UnknownDictionaryWriter;
}

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
namespace org::apache::lucene::analysis::ja::util
{

class UnknownDictionaryBuilder
    : public std::enable_shared_from_this<UnknownDictionaryBuilder>
{
  GET_CLASS_NAME(UnknownDictionaryBuilder)
private:
  static const std::wstring NGRAM_DICTIONARY_ENTRY;

  std::wstring encoding = L"euc-jp";

public:
  UnknownDictionaryBuilder(const std::wstring &encoding);

  virtual std::shared_ptr<UnknownDictionaryWriter>
  build(const std::wstring &dirname) ;

  virtual std::shared_ptr<UnknownDictionaryWriter>
  readDictionaryFile(const std::wstring &filename) ;

  virtual std::shared_ptr<UnknownDictionaryWriter>
  readDictionaryFile(const std::wstring &filename,
                     const std::wstring &encoding) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<std::wstring>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<UnknownDictionaryBuilder> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<UnknownDictionaryBuilder> outerInstance);

    int compare(std::deque<std::wstring> &left,
                std::deque<std::wstring> &right);
  };

public:
  virtual void readCharacterDefinition(
      const std::wstring &filename,
      std::shared_ptr<UnknownDictionaryWriter> dictionary) ;
};

} // namespace org::apache::lucene::analysis::ja::util
