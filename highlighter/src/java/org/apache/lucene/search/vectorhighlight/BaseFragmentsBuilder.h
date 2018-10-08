#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::vectorhighlight
{
class BoundaryScanner;
}

namespace org::apache::lucene::search::vectorhighlight
{
class FieldFragList;
}
namespace org::apache::lucene::search::vectorhighlight
{
class WeightedFragInfo;
}
namespace org::apache::lucene::search::highlight
{
class Encoder;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::document
{
class Field;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::search::vectorhighlight
{

using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using DefaultEncoder = org::apache::lucene::search::highlight::DefaultEncoder;
using Encoder = org::apache::lucene::search::highlight::Encoder;
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;

/**
 * Base FragmentsBuilder implementation that supports colored pre/post
 * tags and multivalued fields.
 * <p>
 * Uses {@link BoundaryScanner} to determine fragments.
 */
class BaseFragmentsBuilder
    : public std::enable_shared_from_this<BaseFragmentsBuilder>,
      public FragmentsBuilder
{
  GET_CLASS_NAME(BaseFragmentsBuilder)

protected:
  std::deque<std::wstring> preTags, postTags;

public:
  static std::deque<std::wstring> const COLORED_PRE_TAGS;
  static std::deque<std::wstring> const COLORED_POST_TAGS;

private:
  wchar_t multiValuedSeparator = L' ';
  const std::shared_ptr<BoundaryScanner> boundaryScanner;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool discreteMultiValueHighlighting_ = false;

protected:
  BaseFragmentsBuilder();

  BaseFragmentsBuilder(std::deque<std::wstring> &preTags,
                       std::deque<std::wstring> &postTags);

  BaseFragmentsBuilder(std::shared_ptr<BoundaryScanner> boundaryScanner);

  BaseFragmentsBuilder(std::deque<std::wstring> &preTags,
                       std::deque<std::wstring> &postTags,
                       std::shared_ptr<BoundaryScanner> boundaryScanner);

public:
  static std::any checkTagsArgument(std::any tags);

  virtual std::deque<std::shared_ptr<WeightedFragInfo>>
  getWeightedFragInfoList(
      std::deque<std::shared_ptr<WeightedFragInfo>> &src) = 0;

private:
  static const std::shared_ptr<Encoder> NULL_ENCODER;

public:
  std::wstring createFragment(
      std::shared_ptr<IndexReader> reader, int docId,
      const std::wstring &fieldName,
      std::shared_ptr<FieldFragList> fieldFragList)  override;

  std::deque<std::wstring>
  createFragments(std::shared_ptr<IndexReader> reader, int docId,
                  const std::wstring &fieldName,
                  std::shared_ptr<FieldFragList> fieldFragList,
                  int maxNumFragments)  override;

  std::wstring
  createFragment(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &fieldName,
                 std::shared_ptr<FieldFragList> fieldFragList,
                 std::deque<std::wstring> &preTags,
                 std::deque<std::wstring> &postTags,
                 std::shared_ptr<Encoder> encoder)  override;

  std::deque<std::wstring>
  createFragments(std::shared_ptr<IndexReader> reader, int docId,
                  const std::wstring &fieldName,
                  std::shared_ptr<FieldFragList> fieldFragList,
                  int maxNumFragments, std::deque<std::wstring> &preTags,
                  std::deque<std::wstring> &postTags,
                  std::shared_ptr<Encoder> encoder)  override;

protected:
  virtual std::deque<std::shared_ptr<Field>>
  getFields(std::shared_ptr<IndexReader> reader, int docId,
            const std::wstring &fieldName) ;

private:
  class StoredFieldVisitorAnonymousInnerClass : public StoredFieldVisitor
  {
    GET_CLASS_NAME(StoredFieldVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<BaseFragmentsBuilder> outerInstance;

    std::wstring fieldName;
    std::deque<std::shared_ptr<Field>> fields;

  public:
    StoredFieldVisitorAnonymousInnerClass(
        std::shared_ptr<BaseFragmentsBuilder> outerInstance,
        const std::wstring &fieldName,
        std::deque<std::shared_ptr<Field>> &fields);

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &bytes) override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<StoredFieldVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<StoredFieldVisitorAnonymousInnerClass>(
          org.apache.lucene.index.StoredFieldVisitor::shared_from_this());
    }
  };

protected:
  virtual std::wstring makeFragment(std::shared_ptr<StringBuilder> buffer,
                                    std::deque<int> &index,
                                    std::deque<std::shared_ptr<Field>> &values,
                                    std::shared_ptr<WeightedFragInfo> fragInfo,
                                    std::deque<std::wstring> &preTags,
                                    std::deque<std::wstring> &postTags,
                                    std::shared_ptr<Encoder> encoder);

  virtual std::wstring getFragmentSourceMSO(
      std::shared_ptr<StringBuilder> buffer, std::deque<int> &index,
      std::deque<std::shared_ptr<Field>> &values, int startOffset,
      int endOffset, std::deque<int> &modifiedStartOffset);

  virtual std::wstring
  getFragmentSource(std::shared_ptr<StringBuilder> buffer,
                    std::deque<int> &index,
                    std::deque<std::shared_ptr<Field>> &values,
                    int startOffset, int endOffset);

  virtual std::deque<std::shared_ptr<WeightedFragInfo>>
  discreteMultiValueHighlighting(
      std::deque<std::shared_ptr<WeightedFragInfo>> &fragInfos,
      std::deque<std::shared_ptr<Field>> &fields);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<WeightedFragInfo>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<BaseFragmentsBuilder> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<BaseFragmentsBuilder> outerInstance);

    int compare(std::shared_ptr<FieldFragList::WeightedFragInfo> info1,
                std::shared_ptr<FieldFragList::WeightedFragInfo> info2);
  };

public:
  virtual void setMultiValuedSeparator(wchar_t separator);

  virtual wchar_t getMultiValuedSeparator();

  virtual bool isDiscreteMultiValueHighlighting();

  virtual void
  setDiscreteMultiValueHighlighting(bool discreteMultiValueHighlighting);

protected:
  virtual std::wstring getPreTag(int num);

  virtual std::wstring getPostTag(int num);

  virtual std::wstring getPreTag(std::deque<std::wstring> &preTags, int num);

  virtual std::wstring getPostTag(std::deque<std::wstring> &postTags, int num);
};

} // namespace org::apache::lucene::search::vectorhighlight
