#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::analysis::phonetic
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestPhoneticFilterFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPhoneticFilterFactory)

  /**
   * Case: default
   */
public:
  virtual void testFactoryDefaults() ;

  virtual void testInjectFalse() ;

  virtual void testMaxCodeLength() ;

  /**
   * Case: Failures and Exceptions
   */
  virtual void testMissingEncoder() ;

  virtual void testUnknownEncoder() ;

  virtual void testUnknownEncoderReflection() ;

  /**
   * Case: Reflection
   */
  virtual void testFactoryReflection() ;

  /**
   * we use "Caverphone2" as it is registered in the REGISTRY as Caverphone,
   * so this effectively tests reflection without package name
   */
  virtual void testFactoryReflectionCaverphone2() ;

  virtual void testFactoryReflectionCaverphone() ;

  virtual void testAlgorithms() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestPhoneticFilterFactory> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestPhoneticFilterFactory> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  static void assertAlgorithm(
      const std::wstring &algName, const std::wstring &inject,
      const std::wstring &input,
      std::deque<std::wstring> &expected) ;

protected:
  std::shared_ptr<TestPhoneticFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestPhoneticFilterFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/phonetic/
