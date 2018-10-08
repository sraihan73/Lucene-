#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Bits;
}

namespace org::apache::lucene::index
{
class LeafReader;
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
namespace org::apache::lucene::index
{

using Bits = org::apache::lucene::util::Bits;

/**
 * Filters the incoming reader and makes all documents appear deleted.
 */
class AllDeletedFilterReader : public FilterLeafReader
{
  GET_CLASS_NAME(AllDeletedFilterReader)
public:
  const std::shared_ptr<Bits> liveDocs;

  AllDeletedFilterReader(std::shared_ptr<LeafReader> in_);

  std::shared_ptr<Bits> getLiveDocs() override;

  int numDocs() override;

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<AllDeletedFilterReader> shared_from_this()
  {
    return std::static_pointer_cast<AllDeletedFilterReader>(
        FilterLeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
