#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfos;
}

namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
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

/**
 * A {@link FilterLeafReader} that exposes only a subset
 * of fields from the underlying wrapped reader.
 */
class FieldFilterLeafReader final : public FilterLeafReader
{
  GET_CLASS_NAME(FieldFilterLeafReader)

private:
  const std::shared_ptr<Set<std::wstring>> fields;
  const bool negate;
  const std::shared_ptr<FieldInfos> fieldInfos;

public:
  FieldFilterLeafReader(std::shared_ptr<LeafReader> in_,
                        std::shared_ptr<Set<std::wstring>> fields, bool negate);

  bool hasField(const std::wstring &field);

  std::shared_ptr<FieldInfos> getFieldInfos() override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

  void document(
      int const docID,
      std::shared_ptr<StoredFieldVisitor> visitor)  override;

private:
  class StoredFieldVisitorAnonymousInnerClass : public StoredFieldVisitor
  {
    GET_CLASS_NAME(StoredFieldVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<FieldFilterLeafReader> outerInstance;

    std::shared_ptr<org::apache::lucene::index::StoredFieldVisitor> visitor;

  public:
    StoredFieldVisitorAnonymousInnerClass(
        std::shared_ptr<FieldFilterLeafReader> outerInstance,
        std::shared_ptr<org::apache::lucene::index::StoredFieldVisitor>
            visitor);

    void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void intField(std::shared_ptr<FieldInfo> fieldInfo,
                  int value)  override;

    void longField(std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t value)  override;

    void floatField(std::shared_ptr<FieldInfo> fieldInfo,
                    float value)  override;

    void doubleField(std::shared_ptr<FieldInfo> fieldInfo,
                     double value)  override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) throw(
        IOException) override;

  protected:
    std::shared_ptr<StoredFieldVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StoredFieldVisitorAnonymousInnerClass>(
          StoredFieldVisitor::shared_from_this());
    }
  };

public:
  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  virtual std::wstring toString();

private:
  class FieldFilterFields : public FilterFields
  {
    GET_CLASS_NAME(FieldFilterFields)
  private:
    std::shared_ptr<FieldFilterLeafReader> outerInstance;

  public:
    FieldFilterFields(std::shared_ptr<FieldFilterLeafReader> outerInstance,
                      std::shared_ptr<Fields> in_);

    int size() override;

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

  private:
    class FilterIteratorAnonymousInnerClass
        : public FilterIterator<std::wstring, std::wstring>
    {
      GET_CLASS_NAME(FilterIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<FieldFilterFields> outerInstance;

    public:
      FilterIteratorAnonymousInnerClass(
          std::shared_ptr<FieldFilterFields> outerInstance,
          std::shared_ptr<Iterator<std::wstring>> iterator);

    protected:
      bool predicateFunction(const std::wstring &field) override;

    protected:
      std::shared_ptr<FilterIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterIteratorAnonymousInnerClass>(
            org.apache.lucene.util
                .FilterIterator<std::wstring, std::wstring>::shared_from_this());
      }
    };

  public:
    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  protected:
    std::shared_ptr<FieldFilterFields> shared_from_this()
    {
      return std::static_pointer_cast<FieldFilterFields>(
          FilterFields::shared_from_this());
    }
  };

public:
  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<FieldFilterLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<FieldFilterLeafReader>(
        FilterLeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
