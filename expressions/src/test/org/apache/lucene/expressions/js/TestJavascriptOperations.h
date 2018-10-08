#pragma once
#include "stringhelper.h"
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

class TestJavascriptOperations : public LuceneTestCase
{
  GET_CLASS_NAME(TestJavascriptOperations)
private:
  void assertEvaluatesTo(const std::wstring &expression,
                         int64_t expected) ;

public:
  virtual void testNegationOperation() ;

  virtual void testAddOperation() ;

  virtual void testSubtractOperation() ;

  virtual void testMultiplyOperation() ;

  virtual void testDivisionOperation() ;

  virtual void testModuloOperation() ;

  virtual void testLessThanOperation() ;

  virtual void testLessThanEqualsOperation() ;

  virtual void testGreaterThanOperation() ;

  virtual void testGreaterThanEqualsOperation() ;

  virtual void testEqualsOperation() ;

  virtual void testNotEqualsOperation() ;

  virtual void testBoolNotOperation() ;

  virtual void testBoolAndOperation() ;

  virtual void testBoolOrOperation() ;

  virtual void testConditionalOperation() ;

  virtual void testBitShiftLeft() ;

  virtual void testBitShiftRight() ;

  virtual void testBitShiftRightUnsigned() ;

  virtual void testBitwiseAnd() ;

  virtual void testBitwiseOr() ;

  virtual void testBitwiseXor() ;

  virtual void testBitwiseNot() ;

  virtual void testDecimalConst() ;

  virtual void testHexConst() ;

  virtual void testHexConst2() ;

  virtual void testOctalConst() ;

protected:
  std::shared_ptr<TestJavascriptOperations> shared_from_this()
  {
    return std::static_pointer_cast<TestJavascriptOperations>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions::js
