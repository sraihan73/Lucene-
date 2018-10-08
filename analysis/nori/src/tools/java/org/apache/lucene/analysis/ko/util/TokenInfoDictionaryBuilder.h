#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/util/TokenInfoDictionaryWriter.h"

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

using com::ibm::icu::text::Normalizer2;

class TokenInfoDictionaryBuilder
    : public std::enable_shared_from_this<TokenInfoDictionaryBuilder>
{
  GET_CLASS_NAME(TokenInfoDictionaryBuilder)

  /** Internal word id - incrementally assigned as entries are read and added.
   * This will be byte offset of dictionary file */
private:
  int offset = 0;

  std::wstring encoding = L"utf-8";

  bool normalizeEntries = false;
  std::shared_ptr<Normalizer2> normalizer;

public:
  TokenInfoDictionaryBuilder(const std::wstring &encoding,
                             bool normalizeEntries);

  virtual std::shared_ptr<TokenInfoDictionaryWriter>
  build(const std::wstring &dirname) ;

  virtual std::shared_ptr<TokenInfoDictionaryWriter> buildDictionary(
      std::deque<std::shared_ptr<File>> &csvFiles) ;
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/util/
