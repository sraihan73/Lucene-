#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Version;
}

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
namespace org::apache::lucene::codecs::lucene70
{

using Codec = org::apache::lucene::codecs::Codec;
using BaseSegmentInfoFormatTestCase =
    org::apache::lucene::index::BaseSegmentInfoFormatTestCase;
using Version = org::apache::lucene::util::Version;

class TestLucene70SegmentInfoFormat : public BaseSegmentInfoFormatTestCase
{
  GET_CLASS_NAME(TestLucene70SegmentInfoFormat)

protected:
  std::deque<std::shared_ptr<Version>> getVersions() override;

  std::shared_ptr<Codec> getCodec() override;

protected:
  std::shared_ptr<TestLucene70SegmentInfoFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestLucene70SegmentInfoFormat>(
        org.apache.lucene.index
            .BaseSegmentInfoFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene70
