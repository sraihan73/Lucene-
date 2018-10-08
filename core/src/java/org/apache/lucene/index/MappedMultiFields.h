#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergeState;
}

namespace org::apache::lucene::index
{
class MultiFields;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class MultiTerms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class MultiTermsEnum;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.FilterLeafReader.FilterFields;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.FilterLeafReader.FilterTerms;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.FilterLeafReader.FilterTermsEnum;

/** A {@link Fields} implementation that merges multiple
 *  Fields into one, and maps around deleted documents.
 *  This is used for merging.
 *  @lucene.internal
 */
class MappedMultiFields : public FilterFields
{
  GET_CLASS_NAME(MappedMultiFields)
public:
  const std::shared_ptr<MergeState> mergeState;

  /** Create a new MappedMultiFields for merging, based on the supplied
   * mergestate and merged view of terms. */
  MappedMultiFields(std::shared_ptr<MergeState> mergeState,
                    std::shared_ptr<MultiFields> multiFields);

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

private:
  class MappedMultiTerms : public FilterTerms
  {
    GET_CLASS_NAME(MappedMultiTerms)
  public:
    const std::shared_ptr<MergeState> mergeState;
    const std::wstring field;

    MappedMultiTerms(const std::wstring &field,
                     std::shared_ptr<MergeState> mergeState,
                     std::shared_ptr<MultiTerms> multiTerms);

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size()  override;

    int64_t getSumTotalTermFreq()  override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

  protected:
    std::shared_ptr<MappedMultiTerms> shared_from_this()
    {
      return std::static_pointer_cast<MappedMultiTerms>(
          FilterTerms::shared_from_this());
    }
  };

private:
  class MappedMultiTermsEnum : public FilterTermsEnum
  {
    GET_CLASS_NAME(MappedMultiTermsEnum)
  public:
    const std::shared_ptr<MergeState> mergeState;
    const std::wstring field;

    MappedMultiTermsEnum(const std::wstring &field,
                         std::shared_ptr<MergeState> mergeState,
                         std::shared_ptr<MultiTermsEnum> multiTermsEnum);

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

  protected:
    std::shared_ptr<MappedMultiTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<MappedMultiTermsEnum>(
          FilterTermsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MappedMultiFields> shared_from_this()
  {
    return std::static_pointer_cast<MappedMultiFields>(
        FilterFields::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
