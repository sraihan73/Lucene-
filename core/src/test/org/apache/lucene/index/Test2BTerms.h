#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util
{
class AttributeImpl;
}
namespace org::apache::lucene::util
{
class AttributeReflector;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
}
namespace org::apache::lucene::index
{
class IndexReader;
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

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// NOTE: SimpleText codec will consume very large amounts of
// disk (but, should run successfully).  Best to run w/
// -Dtests.codec=<current codec>, and w/ plenty of RAM, eg:
//
//   ant test -Dtests.monster=true -Dtests.heapsize=8g -Dtests.codec=Lucene62
//   -Dtestcase=Test2BTerms
//
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" })
// @Monster("very slow, use 5g minimum heap") @TimeoutSuite(millis = 80 *
// TimeUnits.HOUR) @SuppressSysoutChecks(bugUrl = "Stuff gets printed") public
// class Test2BTerms extends org.apache.lucene.util.LuceneTestCase
class Test2BTerms : public LuceneTestCase
{

private:
  static constexpr int TOKEN_LEN = 5;

  static const std::shared_ptr<BytesRef> bytes;

private:
  class MyTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(MyTokenStream)

  private:
    const int tokensPerDoc;
    int tokenCount = 0;

  public:
    const std::deque<std::shared_ptr<BytesRef>> savedTerms =
        std::deque<std::shared_ptr<BytesRef>>();

  private:
    int nextSave = 0;
    int64_t termCounter = 0;
    const std::shared_ptr<Random> random;

  public:
    MyTokenStream(std::shared_ptr<Random> random, int tokensPerDoc);

    bool incrementToken() override;

    void reset() override;

  private:
    class MyTermAttributeImpl final : public AttributeImpl,
                                      public TermToBytesRefAttribute
    {
      GET_CLASS_NAME(MyTermAttributeImpl)
    public:
      std::shared_ptr<BytesRef> getBytesRef() override;

      void clear() override;

      void copyTo(std::shared_ptr<AttributeImpl> target) override;

      std::shared_ptr<MyTermAttributeImpl> clone() override;

      void reflectWith(AttributeReflector reflector) override;

    protected:
      std::shared_ptr<MyTermAttributeImpl> shared_from_this()
      {
        return std::static_pointer_cast<MyTermAttributeImpl>(
            org.apache.lucene.util.AttributeImpl::shared_from_this());
      }
    };

  private:
    class MyAttributeFactory final : public AttributeFactory
    {
      GET_CLASS_NAME(MyAttributeFactory)
    private:
      const std::shared_ptr<AttributeFactory> delegate_;

    public:
      MyAttributeFactory(std::shared_ptr<AttributeFactory> delegate_);

      std::shared_ptr<AttributeImpl>
      createAttributeInstance(std::type_info attClass) override;

    protected:
      std::shared_ptr<MyAttributeFactory> shared_from_this()
      {
        return std::static_pointer_cast<MyAttributeFactory>(
            org.apache.lucene.util.AttributeFactory::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MyTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<MyTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void test2BTerms() ;

private:
  std::deque<std::shared_ptr<BytesRef>>
  findTerms(std::shared_ptr<IndexReader> r) ;

  void testSavedTerms(
      std::shared_ptr<IndexReader> r,
      std::deque<std::shared_ptr<BytesRef>> &terms) ;

protected:
  std::shared_ptr<Test2BTerms> shared_from_this()
  {
    return std::static_pointer_cast<Test2BTerms>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
