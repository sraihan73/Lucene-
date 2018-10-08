#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/surround/query/SingleFieldTestDb.h"

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

class Test03Distance : public LuceneTestCase
{
  GET_CLASS_NAME(Test03Distance)
  static void main(std::deque<std::wstring> &args);

public:
  bool verbose = false;
  int maxBasicQueries = 16;

  std::deque<std::wstring> exceptionQueries = {
      L"(aa and bb) w cc",        L"(aa or bb) w (cc and dd)",
      L"(aa opt bb) w cc",        L"(aa not bb) w cc",
      L"(aa or bb) w (bi:cc)",    L"(aa or bb) w bi:cc",
      L"(aa or bi:bb) w cc",      L"(aa or (bi:bb)) w cc",
      L"(aa or (bb and dd)) w cc"};

  virtual void test00Exceptions() ;

  const std::wstring fieldName = L"bi";

  std::deque<std::wstring> docs1 = {
      L"word1 word2 word3", L"word4 word5", L"ord1 ord2 ord3",
      L"orda1 orda2 orda3 word2 worda3", L"a c e a b c"};

  std::shared_ptr<SingleFieldTestDb> db1;

  void setUp()  override;

private:
  void
  distanceTst(const std::wstring &query, std::deque<int> &expdnrs,
              std::shared_ptr<SingleFieldTestDb> db) ;

public:
  virtual void
  distanceTest1(const std::wstring &query,
                std::deque<int> &expdnrs) ;

  virtual void test0W01() ;
  virtual void test0N01() ;
  virtual void test0N01r() ;

  virtual void test0W02() ;

  virtual void test0W03() ;
  virtual void test0N03() ;
  virtual void test0N03r() ;

  virtual void test0W04() ;

  virtual void test0N04() ;
  virtual void test0N04r() ;

  virtual void test0W05() ;
  virtual void test0W06() ;

  virtual void test1Wtrunc01() ;
  virtual void test1Wtrunc02() ;
  virtual void test1Wtrunc02r() ;
  virtual void test1Ntrunc02() ;
  virtual void test1Ntrunc02r() ;

  virtual void test1Wtrunc03() ;
  virtual void test1Ntrunc03() ;

  virtual void test1Wtrunc04() ;
  virtual void test1Ntrunc04() ;

  virtual void test1Wtrunc05() ;
  virtual void test1Ntrunc05() ;

  virtual void test1Wtrunc06() ;
  virtual void test1Ntrunc06() ;
  virtual void test1Ntrunc06r() ;

  virtual void test1Wtrunc07() ;
  virtual void test1Wtrunc08() ;
  virtual void test1Wtrunc09() ;
  virtual void test1Ntrunc09() ;

  std::deque<std::wstring> docs2 = {L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3", L""};

  std::shared_ptr<SingleFieldTestDb> db2;

  virtual void
  distanceTest2(const std::wstring &query,
                std::deque<int> &expdnrs) ;

  virtual void test2Wprefix01() ;
  virtual void test2Nprefix01a() ;
  virtual void test2Nprefix01b() ;

  virtual void test2Wprefix02() ;

  virtual void test2Nprefix02a() ;
  virtual void test2Nprefix02b() ;

  virtual void test2Wnested01() ;
  virtual void test2Nnested01() ;

  virtual void test2Wnested02() ;
  virtual void test2Nnested02() ;

  std::deque<std::wstring> docs3 = {
      L"low pressure temperature inversion and rain",
      L"when the temperature has a negative height above a depression no "
      L"precipitation gradient is expected",
      L"when the temperature has a negative height gradient above a depression "
      L"no precipitation is expected",
      L""};

  std::shared_ptr<SingleFieldTestDb> db3;

  virtual void
  distanceTest3(const std::wstring &query,
                std::deque<int> &expdnrs) ;

  virtual void test3Example01() ;

protected:
  std::shared_ptr<Test03Distance> shared_from_this()
  {
    return std::static_pointer_cast<Test03Distance>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/query/
