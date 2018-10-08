#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::en
{

/**
 * Minimal plural stemmer for English.
 * <p>
 * This stemmer implements the "S-Stemmer" from
 * <i>How Effective Is Suffixing?</i>
 * Donna Harman.
 */
class EnglishMinimalStemmer
    : public std::enable_shared_from_this<EnglishMinimalStemmer>
{
  GET_CLASS_NAME(EnglishMinimalStemmer)
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") public int stem(char s[],
  // int len)
  virtual int stem(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::en