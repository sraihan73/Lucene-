#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class CompoundFormat;
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

using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

class CrankyCompoundFormat : public CompoundFormat
{
  GET_CLASS_NAME(CrankyCompoundFormat)
public:
  std::shared_ptr<CompoundFormat> delegate_;
  std::shared_ptr<Random> random;

  CrankyCompoundFormat(std::shared_ptr<CompoundFormat> delegate_,
                       std::shared_ptr<Random> random);

  std::shared_ptr<Directory> getCompoundReader(
      std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
      std::shared_ptr<IOContext> context)  override;

  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<CrankyCompoundFormat> shared_from_this()
  {
    return std::static_pointer_cast<CrankyCompoundFormat>(
        org.apache.lucene.codecs.CompoundFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::cranky
