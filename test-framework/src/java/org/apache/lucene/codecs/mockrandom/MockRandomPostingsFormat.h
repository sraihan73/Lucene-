#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class TermStats;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::mockrandom
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Randomly combines terms index impl w/ postings impls.
 */

class MockRandomPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(MockRandomPostingsFormat)
private:
  const std::shared_ptr<Random> seedRandom;
  static const std::wstring SEED_EXT;

public:
  MockRandomPostingsFormat();

  MockRandomPostingsFormat(std::shared_ptr<Random> random);

private:
  class RandomAnonymousInnerClass : public Random
  {
    GET_CLASS_NAME(RandomAnonymousInnerClass)
  private:
    std::shared_ptr<MockRandomPostingsFormat> outerInstance;

  public:
    RandomAnonymousInnerClass(
        std::shared_ptr<MockRandomPostingsFormat> outerInstance);

  protected:
    int next(int arg0) override;

  protected:
    std::shared_ptr<RandomAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RandomAnonymousInnerClass>(
          java.util.Random::shared_from_this());
    }
  };

public:
  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

private:
  class IndexTermSelectorAnonymousInnerClass
      : public VariableGapTermsIndexWriter::IndexTermSelector
  {
    GET_CLASS_NAME(IndexTermSelectorAnonymousInnerClass)
  private:
    std::shared_ptr<MockRandomPostingsFormat> outerInstance;

    int64_t seed2 = 0;
    int gap = 0;

  public:
    IndexTermSelectorAnonymousInnerClass(
        std::shared_ptr<MockRandomPostingsFormat> outerInstance,
        int64_t seed2, int gap);

    const std::shared_ptr<Random> rand;

    bool isIndexTerm(std::shared_ptr<BytesRef> term,
                     std::shared_ptr<TermStats> stats) override;

    void newField(std::shared_ptr<FieldInfo> fieldInfo) override;

  protected:
    std::shared_ptr<IndexTermSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IndexTermSelectorAnonymousInnerClass>(
          org.apache.lucene.codecs.blockterms.VariableGapTermsIndexWriter
              .IndexTermSelector::shared_from_this());
    }
  };

public:
  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

protected:
  std::shared_ptr<MockRandomPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<MockRandomPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::mockrandom
