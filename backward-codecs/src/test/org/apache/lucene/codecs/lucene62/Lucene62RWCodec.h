#pragma once
#include "../../../../../../java/org/apache/lucene/codecs/lucene62/Lucene62Codec.h"
#include "../lucene53/Lucene53RWNormsFormat.h"
#include "Lucene62RWSegmentInfoFormat.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/NormsFormat.h"

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
namespace org::apache::lucene::codecs::lucene62
{

using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using Lucene53RWNormsFormat =
    org::apache::lucene::codecs::lucene53::Lucene53RWNormsFormat;
using Lucene62Codec = org::apache::lucene::codecs::lucene62::Lucene62Codec;

/**
 * Read-write version of 6.2 codec for testing
 * @deprecated for test purposes only
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class Lucene62RWCodec extends
// org.apache.lucene.codecs.lucene62.Lucene62Codec
class Lucene62RWCodec : public Lucene62Codec
{

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<SegmentInfoFormat> segmentInfoFormat_ =
      std::make_shared<Lucene62RWSegmentInfoFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<NormsFormat> normsFormat_ =
      std::make_shared<Lucene53RWNormsFormat>();

public:
  std::shared_ptr<SegmentInfoFormat> segmentInfoFormat() override;

  std::shared_ptr<NormsFormat> normsFormat() override;

protected:
  std::shared_ptr<Lucene62RWCodec> shared_from_this()
  {
    return std::static_pointer_cast<Lucene62RWCodec>(
        org.apache.lucene.codecs.lucene62.Lucene62Codec::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene62/
