#pragma once
#include "DictionaryBuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::util
{
class DictionaryBuilder;
}

namespace org::apache::lucene::analysis::ja::util
{
class TokenInfoDictionaryWriter;
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

using DictionaryFormat = org::apache::lucene::analysis::ja::util::
    DictionaryBuilder::DictionaryFormat;

using com::ibm::icu::text::Normalizer2;

/**
 */
class TokenInfoDictionaryBuilder
    : public std::enable_shared_from_this<TokenInfoDictionaryBuilder>
{
  GET_CLASS_NAME(TokenInfoDictionaryBuilder)

  /** Internal word id - incrementally assigned as entries are read and added.
   * This will be byte offset of dictionary file */
private:
  int offset = 0;

  std::wstring encoding = L"euc-jp";

  bool normalizeEntries = false;
  std::shared_ptr<Normalizer2> normalizer;

  DictionaryFormat format = DictionaryFormat::IPADIC;

public:
  TokenInfoDictionaryBuilder(DictionaryFormat format,
                             const std::wstring &encoding,
                             bool normalizeEntries);

  virtual std::shared_ptr<TokenInfoDictionaryWriter>
  build(const std::wstring &dirname) ;

private:
  class FilenameFilterAnonymousInnerClass : public FilenameFilter
  {
    GET_CLASS_NAME(FilenameFilterAnonymousInnerClass)
  private:
    std::shared_ptr<TokenInfoDictionaryBuilder> outerInstance;

  public:
    FilenameFilterAnonymousInnerClass(
        std::shared_ptr<TokenInfoDictionaryBuilder> outerInstance);

    bool accept(std::shared_ptr<File> dir, const std::wstring &name) override;

  protected:
    std::shared_ptr<FilenameFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilenameFilterAnonymousInnerClass>(
          java.io.FilenameFilter::shared_from_this());
    }
  };

public:
  virtual std::shared_ptr<TokenInfoDictionaryWriter> buildDictionary(
      std::deque<std::shared_ptr<File>> &csvFiles) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<std::wstring>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TokenInfoDictionaryBuilder> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TokenInfoDictionaryBuilder> outerInstance);

    int compare(std::deque<std::wstring> &left,
                std::deque<std::wstring> &right);
  };

  /*
   * IPADIC features
   *
   * 0   - surface
   * 1   - left cost
   * 2   - right cost
   * 3   - word cost
   * 4-9 - pos
   * 10  - base form
   * 11  - reading
   * 12  - pronounciation
   *
   * UniDic features
   *
   * 0   - surface
   * 1   - left cost
   * 2   - right cost
   * 3   - word cost
   * 4-9 - pos
   * 10  - base form reading
   * 11  - base form
   * 12  - surface form
   * 13  - surface reading
   */

public:
  virtual std::deque<std::wstring>
  formatEntry(std::deque<std::wstring> &features);
};

} // namespace org::apache::lucene::analysis::ja::util
