#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Field;
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
namespace org::apache::lucene::document
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// sanity check some basics of fields
class TestField : public LuceneTestCase
{
  GET_CLASS_NAME(TestField)

public:
  virtual void testDoublePoint() ;

  virtual void testDoublePoint2D() ;

  virtual void testDoubleDocValuesField() ;

  virtual void testFloatDocValuesField() ;

  virtual void testFloatPoint() ;

  virtual void testFloatPoint2D() ;

  virtual void testIntPoint() ;

  virtual void testIntPoint2D() ;

  virtual void testNumericDocValuesField() ;

  virtual void testLongPoint() ;

  virtual void testLongPoint2D() ;

  virtual void testSortedBytesDocValuesField() ;

  virtual void testBinaryDocValuesField() ;

  virtual void testStringField() ;

  virtual void testTextFieldString() ;

  virtual void testTextFieldReader() ;

  /* TODO: this is pretty expert and crazy
   * see if we can fix it up later
  public void testTextFieldTokenStream() throws Exception {
  }
  */

  virtual void testStoredFieldBytes() ;

  virtual void testStoredFieldString() ;

  virtual void testStoredFieldInt() ;

  virtual void testStoredFieldDouble() ;

  virtual void testStoredFieldFloat() ;

  virtual void testStoredFieldLong() ;

  virtual void testIndexedBinaryField() ;

private:
  void trySetByteValue(std::shared_ptr<Field> f);

  void trySetBytesValue(std::shared_ptr<Field> f);

  void trySetBytesRefValue(std::shared_ptr<Field> f);

  void trySetDoubleValue(std::shared_ptr<Field> f);

  void trySetIntValue(std::shared_ptr<Field> f);

  void trySetLongValue(std::shared_ptr<Field> f);

  void trySetFloatValue(std::shared_ptr<Field> f);

  void trySetReaderValue(std::shared_ptr<Field> f);

  void trySetShortValue(std::shared_ptr<Field> f);

  void trySetStringValue(std::shared_ptr<Field> f);

  void trySetTokenStreamValue(std::shared_ptr<Field> f);

protected:
  std::shared_ptr<TestField> shared_from_this()
  {
    return std::static_pointer_cast<TestField>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
