#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"

#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using CodecReader = org::apache::lucene::index::CodecReader;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using FilterCodecReader = org::apache::lucene::index::FilterCodecReader;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiTermsEnum : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiTermsEnum)

  // LUCENE-6826
public:
  virtual void testNoTermsInField() ;

private:
  class MigratingCodecReader : public FilterCodecReader
  {
    GET_CLASS_NAME(MigratingCodecReader)

  public:
    MigratingCodecReader(std::shared_ptr<CodecReader> in_);

    std::shared_ptr<FieldsProducer> getPostingsReader() override;

  private:
    class MigratingFieldsProducer : public BaseMigratingFieldsProducer
    {
      GET_CLASS_NAME(MigratingFieldsProducer)
    public:
      MigratingFieldsProducer(std::shared_ptr<FieldsProducer> delegate_,
                              std::shared_ptr<FieldInfos> newFieldInfo);

      std::shared_ptr<Terms>
      terms(const std::wstring &field)  override;

    protected:
      std::shared_ptr<FieldsProducer>
      create(std::shared_ptr<FieldsProducer> delegate_,
             std::shared_ptr<FieldInfos> newFieldInfo) override;

    private:
      class ValueFilteredTerms : public Terms
      {
        GET_CLASS_NAME(ValueFilteredTerms)

      private:
        const std::shared_ptr<Terms> delegate_;
        const std::shared_ptr<BytesRef> value;

      public:
        ValueFilteredTerms(std::shared_ptr<Terms> delegate_,
                           std::shared_ptr<BytesRef> value);

        std::shared_ptr<TermsEnum> iterator()  override;

      private:
        class FilteredTermsEnumAnonymousInnerClass : public FilteredTermsEnum
        {
          GET_CLASS_NAME(FilteredTermsEnumAnonymousInnerClass)
        private:
          std::shared_ptr<ValueFilteredTerms> outerInstance;

        public:
          FilteredTermsEnumAnonymousInnerClass(
              std::shared_ptr<ValueFilteredTerms> outerInstance,
              std::shared_ptr<TermsEnum> iterator);

        protected:
          AcceptStatus accept(std::shared_ptr<BytesRef> term) override;

          std::shared_ptr<BytesRef>
          nextSeekTerm(std::shared_ptr<BytesRef> currentTerm) override;

        protected:
          std::shared_ptr<FilteredTermsEnumAnonymousInnerClass>
          shared_from_this()
          {
            return std::static_pointer_cast<
                FilteredTermsEnumAnonymousInnerClass>(
                org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
          }
        };

      public:
        int64_t size()  override;

        int64_t getSumTotalTermFreq()  override;

        int64_t getSumDocFreq()  override;

        int getDocCount()  override;

        bool hasFreqs() override;

        bool hasOffsets() override;

        bool hasPositions() override;

        bool hasPayloads() override;

      protected:
        std::shared_ptr<ValueFilteredTerms> shared_from_this()
        {
          return std::static_pointer_cast<ValueFilteredTerms>(
              org.apache.lucene.index.Terms::shared_from_this());
        }
      };

    protected:
      std::shared_ptr<MigratingFieldsProducer> shared_from_this()
      {
        return std::static_pointer_cast<MigratingFieldsProducer>(
            BaseMigratingFieldsProducer::shared_from_this());
      }
    };

  private:
    class BaseMigratingFieldsProducer : public FieldsProducer
    {
      GET_CLASS_NAME(BaseMigratingFieldsProducer)

    private:
      const std::shared_ptr<FieldsProducer> delegate_;
      const std::shared_ptr<FieldInfos> newFieldInfo;

    public:
      BaseMigratingFieldsProducer(std::shared_ptr<FieldsProducer> delegate_,
                                  std::shared_ptr<FieldInfos> newFieldInfo);

      std::shared_ptr<Iterator<std::wstring>> iterator() override;

    private:
      class IteratorAnonymousInnerClass
          : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
            public Iterator<std::wstring>
      {
        GET_CLASS_NAME(IteratorAnonymousInnerClass)
      private:
        std::shared_ptr<BaseMigratingFieldsProducer> outerInstance;

        std::shared_ptr<FieldInfos::const_iterator> fieldInfoIterator;

      public:
        IteratorAnonymousInnerClass(
            std::shared_ptr<BaseMigratingFieldsProducer> outerInstance,
            std::shared_ptr<FieldInfos::const_iterator> fieldInfoIterator);

        bool hasNext();

        void remove();

        std::wstring next();
      };

    public:
      int size() override;

      std::shared_ptr<Terms>
      terms(const std::wstring &field)  override;

      std::shared_ptr<FieldsProducer>
      getMergeInstance()  override;

    protected:
      virtual std::shared_ptr<FieldsProducer>
      create(std::shared_ptr<FieldsProducer> delegate_,
             std::shared_ptr<FieldInfos> newFieldInfo);

    public:
      void checkIntegrity()  override;

      int64_t ramBytesUsed() override;

      std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
      getChildResources() override;

      virtual ~BaseMigratingFieldsProducer();

    protected:
      std::shared_ptr<BaseMigratingFieldsProducer> shared_from_this()
      {
        return std::static_pointer_cast<BaseMigratingFieldsProducer>(
            org.apache.lucene.codecs.FieldsProducer::shared_from_this());
      }
    };

  public:
    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<MigratingCodecReader> shared_from_this()
    {
      return std::static_pointer_cast<MigratingCodecReader>(
          org.apache.lucene.index.FilterCodecReader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMultiTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiTermsEnum>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
