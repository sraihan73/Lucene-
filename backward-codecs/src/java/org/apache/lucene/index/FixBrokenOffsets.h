#pragma once
#include "../../../../../../../core/src/java/org/apache/lucene/index/FilterLeafReader.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentInfos;
}

namespace org::apache::lucene::store
{
class FSDirectory;
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
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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

using FSDirectory = org::apache::lucene::store::FSDirectory;

/**
 * Command-line tool that reads from a source index and
 * writes to a dest index, correcting any broken offsets
 * in the process.
 *
 * @lucene.experimental
 */
class FixBrokenOffsets : public std::enable_shared_from_this<FixBrokenOffsets>
{
  GET_CLASS_NAME(FixBrokenOffsets)
public:
  std::shared_ptr<SegmentInfos> infos;

  std::shared_ptr<FSDirectory> fsDir;

  std::shared_ptr<Path> dir;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") public static void main(std::wstring[] args) throws
  // java.io.IOException
  static void main(std::deque<std::wstring> &args) ;

private:
  class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
  {
    GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
  public:
    FilterLeafReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::LeafReader> reader);

    std::shared_ptr<Fields>
    getTermVectors(int docID)  override;

  private:
    class FilterFieldsAnonymousInnerClass : public FilterFields
    {
      GET_CLASS_NAME(FilterFieldsAnonymousInnerClass)
    private:
      std::shared_ptr<FilterLeafReaderAnonymousInnerClass> outerInstance;

    public:
      FilterFieldsAnonymousInnerClass(
          std::shared_ptr<FilterLeafReaderAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::index::Fields> termVectors);

      std::shared_ptr<Terms>
      terms(const std::wstring &field)  override;

    private:
      class FilterTermsAnonymousInnerClass : public FilterTerms
      {
        GET_CLASS_NAME(FilterTermsAnonymousInnerClass)
      private:
        std::shared_ptr<FilterFieldsAnonymousInnerClass> outerInstance;

      public:
        FilterTermsAnonymousInnerClass(
            std::shared_ptr<FilterFieldsAnonymousInnerClass> outerInstance,
            std::shared_ptr<UnknownType> terms);

        std::shared_ptr<TermsEnum> iterator()  override;

      private:
        class FilterTermsEnumAnonymousInnerClass : public FilterTermsEnum
        {
          GET_CLASS_NAME(FilterTermsEnumAnonymousInnerClass)
        private:
          std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance;

        public:
          FilterTermsEnumAnonymousInnerClass(
              std::shared_ptr<FilterTermsAnonymousInnerClass> outerInstance,
              std::shared_ptr<UnknownType> iterator);

          std::shared_ptr<PostingsEnum>
          postings(std::shared_ptr<PostingsEnum> reuse,
                   int flags)  override;

        private:
          class FilterPostingsEnumAnonymousInnerClass
              : public FilterPostingsEnum
          {
            GET_CLASS_NAME(FilterPostingsEnumAnonymousInnerClass)
          private:
            std::shared_ptr<FilterTermsEnumAnonymousInnerClass> outerInstance;

          public:
            FilterPostingsEnumAnonymousInnerClass(
                std::shared_ptr<FilterTermsEnumAnonymousInnerClass>
                    outerInstance,
                std::shared_ptr<UnknownType> postings);

            int nextLastStartOffset = 0;
            int lastStartOffset = 0;

            int nextPosition()  override;

            int startOffset()  override;

            int endOffset()  override;

          protected:
            std::shared_ptr<FilterPostingsEnumAnonymousInnerClass>
            shared_from_this()
            {
              return std::static_pointer_cast<
                  FilterPostingsEnumAnonymousInnerClass>(
                  FilterPostingsEnum::shared_from_this());
            }
          };

        protected:
          std::shared_ptr<FilterTermsEnumAnonymousInnerClass> shared_from_this()
          {
            return std::static_pointer_cast<FilterTermsEnumAnonymousInnerClass>(
                FilterTermsEnum::shared_from_this());
          }
        };

      protected:
        std::shared_ptr<FilterTermsAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FilterTermsAnonymousInnerClass>(
              FilterTerms::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<FilterFieldsAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterFieldsAnonymousInnerClass>(
            FilterFields::shared_from_this());
      }
    };

  public:
    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
          FilterLeafReader::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
