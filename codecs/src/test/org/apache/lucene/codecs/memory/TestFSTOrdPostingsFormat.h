#pragma once
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/BasePostingsFormatTestCase.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/codecs/memory/FSTOrdPostingsFormat.h"
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::codecs::memory
{

using Codec = org::apache::lucene::codecs::Codec;
using BasePostingsFormatTestCase =
    org::apache::lucene::index::BasePostingsFormatTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

/**
 * Tests FSTOrdPostingsFormat
 */
class TestFSTOrdPostingsFormat : public BasePostingsFormatTestCase
{
  GET_CLASS_NAME(TestFSTOrdPostingsFormat)
private:
  const std::shared_ptr<Codec> codec =
      TestUtil::alwaysPostingsFormat(std::make_shared<FSTOrdPostingsFormat>());

protected:
  std::shared_ptr<Codec> getCodec() override;

protected:
  std::shared_ptr<TestFSTOrdPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestFSTOrdPostingsFormat>(
        org.apache.lucene.index.BasePostingsFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
