#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::charfilter
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestMappingCharFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMappingCharFilter)

public:
  std::shared_ptr<NormalizeCharMap> normMap;

  void setUp()  override;

  virtual void testReaderReset() ;

  virtual void testNothingChange() ;

  virtual void test1to1() ;

  virtual void test1to2() ;

  virtual void test1to3() ;

  virtual void test2to4() ;

  virtual void test2to1() ;

  virtual void test3to1() ;

  virtual void test4to2() ;

  virtual void test5to0() ;

  virtual void testNonBMPChar() ;

  virtual void testFullWidthChar() ;

  //
  //                1111111111222
  //      01234567890123456789012
  //(in)  h i j k ll cccc bbb aa
  //
  //                1111111111222
  //      01234567890123456789012
  //(out) i i jj kkk llll cc b a
  //
  //    h, 0, 1 =>    i, 0, 1
  //    i, 2, 3 =>    i, 2, 3
  //    j, 4, 5 =>   jj, 4, 5
  //    k, 6, 7 =>  kkk, 6, 7
  //   ll, 8,10 => llll, 8,10
  // cccc,11,15 =>   cc,11,15
  //  bbb,16,19 =>    b,16,19
  //   aa,20,22 =>    a,20,22
  //
  virtual void testTokenStream() ;

  //
  //
  //        0123456789
  //(in)    aaaa ll h
  //(out-1) aa llll i
  //(out-2) a llllllll i
  //
  // aaaa,0,4 => a,0,4
  //   ll,5,7 => llllllll,5,7
  //    h,8,9 => i,8,9
  virtual void testChained() ;

  virtual void testRandom() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMappingCharFilter> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestMappingCharFilter> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  //@Ignore("wrong finalOffset:
  //https://issues.apache.org/jira/browse/LUCENE-3971")
public:
  virtual void testFinalOffsetSpecialCase() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestMappingCharFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::charfilter::NormalizeCharMap>
        map_obj;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestMappingCharFilter> outerInstance,
        std::shared_ptr<
            org::apache::lucene::analysis::charfilter::NormalizeCharMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  //@Ignore("wrong finalOffset:
  //https://issues.apache.org/jira/browse/LUCENE-3971")
public:
  virtual void testRandomMaps() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestMappingCharFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::charfilter::NormalizeCharMap>
        map_obj;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestMappingCharFilter> outerInstance,
        std::shared_ptr<
            org::apache::lucene::analysis::charfilter::NormalizeCharMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  std::shared_ptr<NormalizeCharMap> randomMap();

public:
  virtual void testRandomMaps2() ;

protected:
  std::shared_ptr<TestMappingCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestMappingCharFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/charfilter/
