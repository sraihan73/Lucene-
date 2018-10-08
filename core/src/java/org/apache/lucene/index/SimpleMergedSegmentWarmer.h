#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class InfoStream;
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

using IndexReaderWarmer =
    org::apache::lucene::index::IndexWriter::IndexReaderWarmer;
using InfoStream = org::apache::lucene::util::InfoStream;

/**
 * A very simple merged segment warmer that just ensures
 * data structures are initialized.
 */
class SimpleMergedSegmentWarmer
    : public std::enable_shared_from_this<SimpleMergedSegmentWarmer>,
      public IndexReaderWarmer
{
  GET_CLASS_NAME(SimpleMergedSegmentWarmer)
private:
  const std::shared_ptr<InfoStream> infoStream;

  /**
   * Creates a new SimpleMergedSegmentWarmer
   * @param infoStream InfoStream to log statistics about warming.
   */
public:
  SimpleMergedSegmentWarmer(std::shared_ptr<InfoStream> infoStream);

  void warm(std::shared_ptr<LeafReader> reader)  override;
};

} // namespace org::apache::lucene::index
