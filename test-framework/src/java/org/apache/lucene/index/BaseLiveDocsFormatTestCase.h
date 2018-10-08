#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Abstract class that performs basic testing of a codec's
 * {@link LiveDocsFormat}.
GET_CLASS_NAME(that)
 */
class BaseLiveDocsFormatTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseLiveDocsFormatTestCase)

  /** Returns the codec to run tests against */
protected:
  virtual std::shared_ptr<Codec> getCodec() = 0;

private:
  std::shared_ptr<Codec> savedCodec;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testDenseLiveDocs() ;

  virtual void testEmptyLiveDocs() ;

  virtual void testSparseLiveDocs() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Monster("Uses lots of memory") public void testOverflow()
  // throws java.io.IOException
  virtual void testOverflow() ;

private:
  void testSerialization(int maxDoc, int numLiveDocs,
                         bool fixedBitSet) ;

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<BaseLiveDocsFormatTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> liveDocs;

  public:
    BitsAnonymousInnerClass(
        std::shared_ptr<BaseLiveDocsFormatTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> liveDocs);

    bool get(int index) override;

    int length() override;
  };

protected:
  std::shared_ptr<BaseLiveDocsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseLiveDocsFormatTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
