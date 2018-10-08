#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::surround::query
{
class SingleFieldTestDb;
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
namespace org::apache::lucene::queryparser::surround::query
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class Test02Boolean : public LuceneTestCase
{
  GET_CLASS_NAME(Test02Boolean)
  static void main(std::deque<std::wstring> &args);

public:
  const std::wstring fieldName = L"bi";
  bool verbose = false;
  int maxBasicQueries = 16;

  std::deque<std::wstring> docs1 = {
      L"word1 word2 word3", L"word4 word5", L"ord1 ord2 ord3",
      L"orda1 orda2 orda3 word2 worda3", L"a c e a b c"};

  void setUp()  override;

  std::shared_ptr<SingleFieldTestDb> db1;

  virtual void normalTest1(const std::wstring &query,
                           std::deque<int> &expdnrs) ;

  virtual void test02Terms01() ;
  virtual void test02Terms02() ;
  virtual void test02Terms03() ;
  virtual void test02Terms04() ;
  virtual void test02Terms05() ;
  virtual void test02Terms06() ;

  virtual void test02Terms10() ;
  virtual void test02Terms13() ;
  virtual void test02Terms14() ;
  virtual void test02Terms20() ;
  virtual void test02Terms21() ;
  virtual void test02Terms22() ;
  virtual void test02Terms23() ;

  virtual void test03And01() ;
  virtual void test03And02() ;
  virtual void test03And03() ;
  virtual void test04Or01() ;
  virtual void test04Or02() ;
  virtual void test04Or03() ;
  virtual void test05Not01() ;
  virtual void test05Not02() ;
  virtual void test06AndOr01() ;
  virtual void test07AndOrNot02() ;

protected:
  std::shared_ptr<Test02Boolean> shared_from_this()
  {
    return std::static_pointer_cast<Test02Boolean>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
