#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::queryparser::surround::query
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class Test01Exceptions : public LuceneTestCase
{
  GET_CLASS_NAME(Test01Exceptions)
  /** Main for running test case by itself. */
  static void main(std::deque<std::wstring> &args);

public:
  bool verbose = false; // to show actual parsing error messages
  const std::wstring fieldName = L"bi";

  std::deque<std::wstring> exceptionQueries = {
      L"*",
      L"a*",
      L"ab*",
      L"?",
      L"a?",
      L"ab?",
      L"a???b",
      L"a?",
      L"a*b?",
      L"word1 word2",
      L"word2 AND",
      L"word1 OR",
      L"AND(word2)",
      L"AND(word2,)",
      L"AND(word2,word1,)",
      L"OR(word2)",
      L"OR(word2 ,",
      L"OR(word2 , word1 ,)",
      L"xx NOT",
      L"xx (a AND b)",
      L"(a AND b",
      L"a OR b)",
      L"or(word2+ not ord+, and xyz,def)",
      L""};

  virtual void test01Exceptions() ;

protected:
  std::shared_ptr<Test01Exceptions> shared_from_this()
  {
    return std::static_pointer_cast<Test01Exceptions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
