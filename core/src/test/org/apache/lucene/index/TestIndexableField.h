#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexableFieldType;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::index
{
class IndexableField;
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexableField : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexableField)

private:
  class MyField : public std::enable_shared_from_this<MyField>,
                  public IndexableField
  {
    GET_CLASS_NAME(MyField)

  private:
    const int counter;
    const std::shared_ptr<IndexableFieldType> fieldType =
        std::make_shared<IndexableFieldTypeAnonymousInnerClass>();

  private:
    class IndexableFieldTypeAnonymousInnerClass
        : public std::enable_shared_from_this<
              IndexableFieldTypeAnonymousInnerClass>,
          public IndexableFieldType
    {
      GET_CLASS_NAME(IndexableFieldTypeAnonymousInnerClass)
    public:
      IndexableFieldTypeAnonymousInnerClass();

      bool stored() override;

      bool tokenized() override;

      bool storeTermVectors() override;

      bool storeTermVectorOffsets() override;

      bool storeTermVectorPositions() override;

      bool storeTermVectorPayloads() override;

      bool omitNorms() override;

      IndexOptions indexOptions() override;

      DocValuesType docValuesType() override;

      int pointDimensionCount() override;

      int pointNumBytes() override;
    };

  public:
    MyField(int counter);

    std::wstring name() override;

    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<Number> numericValue() override;

    std::shared_ptr<IndexableFieldType> fieldType() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> analyzer,
                std::shared_ptr<TokenStream> previous) override;
  };

  // Silly test showing how to index documents w/o using Lucene's core
  // Document nor Field class
public:
  virtual void testArbitraryFields() ;

private:
  class IterableAnonymousInnerClass
      : public std::enable_shared_from_this<IterableAnonymousInnerClass>,
        public std::deque<std::shared_ptr<IndexableField>>
  {
    GET_CLASS_NAME(IterableAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexableField> outerInstance;

    int fieldCount = 0;
    int finalDocCount = 0;
    int finalBaseCount = 0;

  public:
    IterableAnonymousInnerClass(
        std::shared_ptr<TestIndexableField> outerInstance, int fieldCount,
        int finalDocCount, int finalBaseCount);

    std::shared_ptr<Iterator<std::shared_ptr<IndexableField>>> iterator();

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::shared_ptr<IndexableField>>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<IterableAnonymousInnerClass> outerInstance;

    public:
      IteratorAnonymousInnerClass(
          std::shared_ptr<IterableAnonymousInnerClass> outerInstance);

      int fieldUpto = 0;

      bool hasNext();

      std::shared_ptr<IndexableField> next();

      void remove();
    };
  };

private:
  class CustomField : public std::enable_shared_from_this<CustomField>,
                      public IndexableField
  {
    GET_CLASS_NAME(CustomField)
  public:
    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<Number> numericValue() override;

    std::wstring name() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> a,
                std::shared_ptr<TokenStream> reuse) override;

    std::shared_ptr<IndexableFieldType> fieldType() override;
  };

  // LUCENE-5611
public:
  virtual void testNotIndexedTermVectors() ;

protected:
  std::shared_ptr<TestIndexableField> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexableField>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
