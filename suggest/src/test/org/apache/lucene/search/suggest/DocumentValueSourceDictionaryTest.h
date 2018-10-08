#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/LongValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
namespace org::apache::lucene::search::suggest
{

using Document = org::apache::lucene::document::Document;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class DocumentValueSourceDictionaryTest : public LuceneTestCase
{
  GET_CLASS_NAME(DocumentValueSourceDictionaryTest)

public:
  static const std::wstring FIELD_NAME;
  static const std::wstring WEIGHT_FIELD_NAME_1;
  static const std::wstring WEIGHT_FIELD_NAME_2;
  static const std::wstring WEIGHT_FIELD_NAME_3;
  static const std::wstring PAYLOAD_FIELD_NAME;
  static const std::wstring CONTEXTS_FIELD_NAME;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testValueSourceEmptyReader() throws
  // java.io.IOException
  virtual void testValueSourceEmptyReader() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongValuesSourceEmptyReader() throws
  // java.io.IOException
  virtual void testLongValuesSourceEmptyReader() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testValueSourceBasic() throws
  // java.io.IOException
  virtual void testValueSourceBasic() ;

private:
  static std::shared_ptr<LongValuesSource>
  sum(std::deque<std::wstring> &fields);

private:
  class LongValuesSourceAnonymousInnerClass : public LongValuesSource
  {
    GET_CLASS_NAME(LongValuesSourceAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<LongValuesSource>> sources;

  public:
    LongValuesSourceAnonymousInnerClass(
        std::deque<std::shared_ptr<LongValuesSource>> &sources);

    std::shared_ptr<LongValues>
    getValues(std::shared_ptr<LeafReaderContext> ctx,
              std::shared_ptr<DoubleValues> scores)  override;

  private:
    class LongValuesAnonymousInnerClass : public LongValues
    {
      GET_CLASS_NAME(LongValuesAnonymousInnerClass)
    private:
      std::shared_ptr<LongValuesSourceAnonymousInnerClass> outerInstance;

      std::deque<std::shared_ptr<LongValues>> values;

    public:
      LongValuesAnonymousInnerClass(
          std::shared_ptr<LongValuesSourceAnonymousInnerClass> outerInstance,
          std::deque<std::shared_ptr<LongValues>> &values);

      int64_t longValue()  override;

      bool advanceExact(int doc)  override;

    protected:
      std::shared_ptr<LongValuesAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<LongValuesAnonymousInnerClass>(
            org.apache.lucene.search.LongValues::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    bool needsScores() override;

    virtual int hashCode();

    bool equals(std::any obj) override;

    virtual std::wstring toString();

    std::shared_ptr<LongValuesSource> rewrite(
        std::shared_ptr<IndexSearcher> searcher)  override;

  protected:
    std::shared_ptr<LongValuesSourceAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<LongValuesSourceAnonymousInnerClass>(
          org.apache.lucene.search.LongValuesSource::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongValuesSourceBasic() throws
  // java.io.IOException
  virtual void testLongValuesSourceBasic() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testValueSourceWithContext() throws
  // java.io.IOException
  virtual void testValueSourceWithContext() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongValuesSourceWithContext() throws
  // java.io.IOException
  virtual void testLongValuesSourceWithContext() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testValueSourceWithoutPayload() throws
  // java.io.IOException
  virtual void testValueSourceWithoutPayload() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongValuesSourceWithoutPayload()
  // throws java.io.IOException
  virtual void testLongValuesSourceWithoutPayload() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testValueSourceWithDeletions() throws
  // java.io.IOException
  virtual void testValueSourceWithDeletions() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testLongValuesSourceWithDeletions() throws
  // java.io.IOException
  virtual void testLongValuesSourceWithDeletions() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithValueSource() throws
  // java.io.IOException
  virtual void testWithValueSource() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithLongValuesSource() throws
  // java.io.IOException
  virtual void testWithLongValuesSource() ;

private:
  std::unordered_map<std::wstring, std::shared_ptr<Document>>
  generateIndexDocuments(int ndocs);

protected:
  std::shared_ptr<DocumentValueSourceDictionaryTest> shared_from_this()
  {
    return std::static_pointer_cast<DocumentValueSourceDictionaryTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
