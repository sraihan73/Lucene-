#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentWriteState;
}

namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

class DocConsumer : public std::enable_shared_from_this<DocConsumer>
{
  GET_CLASS_NAME(DocConsumer)
public:
  virtual void processDocument() = 0;
  virtual std::shared_ptr<Sorter::DocMap>
  flush(std::shared_ptr<SegmentWriteState> state) = 0;
  virtual void abort() = 0;

  /**
   * Returns a {@link DocIdSetIterator} for the given field or null if the field
   * doesn't have doc values.
   */
  virtual std::shared_ptr<DocIdSetIterator>
  getHasDocValues(const std::wstring &field) = 0;
};

} // namespace org::apache::lucene::index
