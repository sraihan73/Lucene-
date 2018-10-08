#pragma once
#include "../../../../../../java/org/apache/lucene/codecs/lucene53/Lucene53NormsFormat.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class NormsConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
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
namespace org::apache::lucene::codecs::lucene53
{

using NormsConsumer = org::apache::lucene::codecs::NormsConsumer;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

class Lucene53RWNormsFormat : public Lucene53NormsFormat
{
  GET_CLASS_NAME(Lucene53RWNormsFormat)

public:
  std::shared_ptr<NormsConsumer> normsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

protected:
  std::shared_ptr<Lucene53RWNormsFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene53RWNormsFormat>(
        Lucene53NormsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene53
