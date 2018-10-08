#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
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

/**
 * Tests {@link Document} class.
 */
class TestDocument : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocument)

public:
  std::wstring binaryVal =
      L"this text will be stored as a byte array in the index";
  std::wstring binaryVal2 =
      L"this text will be also stored as a byte array in the index";

  virtual void testBinaryField() ;

  /**
   * Tests {@link Document#removeField(std::wstring)} method for a brand new Document
   * that has not been indexed yet.
   *
   * @throws Exception on error
   */
  virtual void testRemoveForNewDocument() ;

  virtual void testConstructorExceptions() ;

  virtual void testClearDocument();

  /** test that Document.getFields() actually returns an immutable deque */
  virtual void testGetFieldsImmutable();

  /**
   * Tests {@link Document#getValues(std::wstring)} method for a brand new Document
   * that has not been indexed yet.
   *
   * @throws Exception on error
   */
  virtual void testGetValuesForNewDocument() ;

  /**
   * Tests {@link Document#getValues(std::wstring)} method for a Document retrieved
   * from an index.
   *
   * @throws Exception on error
   */
  virtual void testGetValuesForIndexedDocument() ;

  virtual void testGetValues();

  virtual void testPositionIncrementMultiFields() ;

private:
  std::shared_ptr<Document> makeDocumentWithFields();

  void doAssert(std::shared_ptr<Document> doc, bool fromIndex);

public:
  virtual void testFieldSetValue() ;

  // LUCENE-3616
  virtual void testInvalidFields();

  virtual void testNumericFieldAsString() ;

protected:
  std::shared_ptr<TestDocument> shared_from_this()
  {
    return std::static_pointer_cast<TestDocument>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
