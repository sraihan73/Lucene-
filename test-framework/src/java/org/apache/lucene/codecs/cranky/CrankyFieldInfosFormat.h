#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldInfosFormat;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
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
namespace org::apache::lucene::codecs::cranky
{

using FieldInfosFormat = org::apache::lucene::codecs::FieldInfosFormat;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

class CrankyFieldInfosFormat : public FieldInfosFormat
{
  GET_CLASS_NAME(CrankyFieldInfosFormat)
public:
  const std::shared_ptr<FieldInfosFormat> delegate_;
  const std::shared_ptr<Random> random;

  CrankyFieldInfosFormat(std::shared_ptr<FieldInfosFormat> delegate_,
                         std::shared_ptr<Random> random);

  std::shared_ptr<FieldInfos>
  read(std::shared_ptr<Directory> directory,
       std::shared_ptr<SegmentInfo> segmentInfo,
       const std::wstring &segmentSuffix,
       std::shared_ptr<IOContext> iocontext)  override;

  void write(std::shared_ptr<Directory> directory,
             std::shared_ptr<SegmentInfo> segmentInfo,
             const std::wstring &segmentSuffix,
             std::shared_ptr<FieldInfos> infos,
             std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<CrankyFieldInfosFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyFieldInfosFormat>(
        org.apache.lucene.codecs.FieldInfosFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
