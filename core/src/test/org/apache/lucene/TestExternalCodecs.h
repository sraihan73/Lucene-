#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
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
namespace org::apache::lucene
{

using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

/* Intentionally outside of oal.index to verify fully
   external codecs work fine */

class TestExternalCodecs : public LuceneTestCase
{
  GET_CLASS_NAME(TestExternalCodecs)

private:
  class CustomPerFieldCodec final : public AssertingCodec
  {
    GET_CLASS_NAME(CustomPerFieldCodec)

  private:
    const std::shared_ptr<PostingsFormat> ramFormat =
        PostingsFormat::forName(L"RAMOnly");
    const std::shared_ptr<PostingsFormat> defaultFormat =
        TestUtil::getDefaultPostingsFormat();
    const std::shared_ptr<PostingsFormat> memoryFormat =
        PostingsFormat::forName(L"Memory");

  public:
    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<CustomPerFieldCodec> shared_from_this()
    {
      return std::static_pointer_cast<CustomPerFieldCodec>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  // tests storing "id" and "field2" fields as pulsing codec,
  // whose term sort is backwards unicode code point, and
  // storing "field1" as a custom entirely-in-RAM codec
public:
  virtual void testPerFieldCodec() ;

protected:
  std::shared_ptr<TestExternalCodecs> shared_from_this()
  {
    return std::static_pointer_cast<TestExternalCodecs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene
