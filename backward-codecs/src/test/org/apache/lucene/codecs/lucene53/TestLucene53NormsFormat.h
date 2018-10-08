#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/BaseNormsFormatTestCase.h"
#include "../lucene62/Lucene62RWCodec.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

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
namespace org::apache::lucene::codecs::lucene53
{

using Codec = org::apache::lucene::codecs::Codec;
using Lucene62RWCodec = org::apache::lucene::codecs::lucene62::Lucene62RWCodec;
using BaseNormsFormatTestCase =
    org::apache::lucene::index::BaseNormsFormatTestCase;

/**
 * Tests Lucene53NormsFormat
 */
class TestLucene53NormsFormat : public BaseNormsFormatTestCase
{
  GET_CLASS_NAME(TestLucene53NormsFormat)
private:
  const std::shared_ptr<Codec> codec = std::make_shared<Lucene62RWCodec>();

protected:
  int getCreatedVersionMajor() override;

  std::shared_ptr<Codec> getCodec() override;

  bool codecSupportsSparsity() override;

protected:
  std::shared_ptr<TestLucene53NormsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestLucene53NormsFormat>(
        org.apache.lucene.index.BaseNormsFormatTestCase::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/codecs/lucene53/
