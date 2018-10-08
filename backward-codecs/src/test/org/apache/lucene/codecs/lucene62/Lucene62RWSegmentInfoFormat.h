#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/lucene62/Lucene62SegmentInfoFormat.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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

using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/**
 * Read-write version of 6.2 SegmentInfoFormat for testing
 * @deprecated for test purposes only
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class Lucene62RWSegmentInfoFormat extends
// Lucene62SegmentInfoFormat
class Lucene62RWSegmentInfoFormat : public Lucene62SegmentInfoFormat
{
public:
  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> ioContext)  override;

protected:
  std::shared_ptr<Lucene62RWSegmentInfoFormat> shared_from_this()
  {
    return std::static_pointer_cast<Lucene62RWSegmentInfoFormat>(
        Lucene62SegmentInfoFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/lucene62/
