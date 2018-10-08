#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/codecs/blockterms/LuceneVarGapDocFreqInterval.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/BasePostingsFormatTestCase.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
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
namespace org::apache::lucene::codecs::blockterms
{

using Codec = org::apache::lucene::codecs::Codec;
using LuceneVarGapDocFreqInterval =
    org::apache::lucene::codecs::blockterms::LuceneVarGapDocFreqInterval;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Basic tests of a PF using VariableGap terms dictionary (fixed interval,
 * docFreq threshold)
 */
class TestVarGapFixedIntervalPostingsFormat : public BasePostingsFormatTestCase
{
  GET_CLASS_NAME(TestVarGapFixedIntervalPostingsFormat)
private:
  const std::shared_ptr<Codec> codec = TestUtil::alwaysPostingsFormat(
      std::make_shared<LuceneVarGapDocFreqInterval>(
          TestUtil::nextInt(random(), 1, 100),
          TestUtil::nextInt(random(), 1, 1000)));

protected:
  std::shared_ptr<Codec> getCodec() override;

protected:
  std::shared_ptr<TestVarGapFixedIntervalPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestVarGapFixedIntervalPostingsFormat>(
        org.apache.lucene.index.BasePostingsFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/blockterms/
