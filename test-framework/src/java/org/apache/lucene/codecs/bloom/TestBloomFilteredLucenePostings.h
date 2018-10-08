#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/bloom/BloomFilteringPostingsFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/bloom/FuzzySet.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

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
namespace org::apache::lucene::codecs::bloom
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * A class used for testing {@link BloomFilteringPostingsFormat} with a concrete
 * delegate (Lucene41). Creates a Bloom filter on ALL fields and with tiny
 * amounts of memory reserved for the filter. DO NOT USE IN A PRODUCTION
 * APPLICATION This is not a realistic application of Bloom Filters as they
 * ordinarily are larger and operate on only primary key type fields.
 */
class TestBloomFilteredLucenePostings final : public PostingsFormat
{
  GET_CLASS_NAME(TestBloomFilteredLucenePostings)

private:
  std::shared_ptr<BloomFilteringPostingsFormat> delegate_;

  // Special class used to avoid OOM exceptions where Junit tests create many
  // fields.
public:
  class LowMemoryBloomFactory : public BloomFilterFactory
  {
    GET_CLASS_NAME(LowMemoryBloomFactory)
  public:
    std::shared_ptr<FuzzySet>
    getSetForField(std::shared_ptr<SegmentWriteState> state,
                   std::shared_ptr<FieldInfo> info) override;

    bool isSaturated(std::shared_ptr<FuzzySet> bloomFilter,
                     std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<LowMemoryBloomFactory> shared_from_this()
    {
      return std::static_pointer_cast<LowMemoryBloomFactory>(
          BloomFilterFactory::shared_from_this());
    }
  };

public:
  TestBloomFilteredLucenePostings();

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<TestBloomFilteredLucenePostings> shared_from_this()
  {
    return std::static_pointer_cast<TestBloomFilteredLucenePostings>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/bloom/
