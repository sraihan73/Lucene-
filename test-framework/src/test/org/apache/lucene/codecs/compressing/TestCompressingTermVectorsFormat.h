#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
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
namespace org::apache::lucene::codecs::compressing
{

using Codec = org::apache::lucene::codecs::Codec;
using BaseTermVectorsFormatTestCase =
    org::apache::lucene::index::BaseTermVectorsFormatTestCase;

class TestCompressingTermVectorsFormat : public BaseTermVectorsFormatTestCase
{
  GET_CLASS_NAME(TestCompressingTermVectorsFormat)

protected:
  std::shared_ptr<Codec> getCodec() override;

  // https://issues.apache.org/jira/browse/LUCENE-5156
public:
  virtual void testNoOrds() ;

  /**
   * writes some tiny segments with incomplete compressed blocks,
   * and ensures merge recompresses them.
   */
  virtual void testChunkCleanup() ;

protected:
  std::shared_ptr<TestCompressingTermVectorsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestCompressingTermVectorsFormat>(
        org.apache.lucene.index
            .BaseTermVectorsFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
