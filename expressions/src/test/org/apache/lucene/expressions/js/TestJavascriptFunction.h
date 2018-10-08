#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::expressions::js
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestJavascriptFunction : public LuceneTestCase
{
  GET_CLASS_NAME(TestJavascriptFunction)
private:
  static double DELTA;

  void assertEvaluatesTo(const std::wstring &expression,
                         double expected) ;

public:
  virtual void testAbsMethod() ;

  virtual void testAcosMethod() ;

  virtual void testAcoshMethod() ;

  virtual void testAsinMethod() ;

  virtual void testAsinhMethod() ;

  virtual void testAtanMethod() ;

  virtual void testAtan2Method() ;

  virtual void testAtanhMethod() ;

  virtual void testCeilMethod() ;

  virtual void testCosMethod() ;

  virtual void testCoshMethod() ;

  virtual void testExpMethod() ;

  virtual void testFloorMethod() ;

  virtual void testHaversinMethod() ;

  virtual void testLnMethod() ;

  virtual void testLog10Method() ;

  virtual void testLognMethod() ;

  virtual void testMaxMethod() ;

  virtual void testMinMethod() ;

  virtual void testPowMethod() ;

  virtual void testSinMethod() ;

  virtual void testSinhMethod() ;

  virtual void testSqrtMethod() ;

  virtual void testTanMethod() ;

  virtual void testTanhMethod() ;

protected:
  std::shared_ptr<TestJavascriptFunction> shared_from_this()
  {
    return std::static_pointer_cast<TestJavascriptFunction>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
