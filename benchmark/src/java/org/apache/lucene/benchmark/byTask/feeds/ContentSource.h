#pragma once
#include "ContentItemsSource.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

/**
 * Represents content from a specified source, such as TREC, Reuters etc. A
 * {@link ContentSource} is responsible for creating {@link DocData} objects for
 * its documents to be consumed by {@link DocMaker}. It also keeps track
 * of various statistics, such as how many documents were generated, size in
 * bytes etc.
 * <p>
 * For supported configuration parameters see {@link ContentItemsSource}.
 */
class ContentSource : public ContentItemsSource
{
  GET_CLASS_NAME(ContentSource)

  /** Returns the next {@link DocData} from the content source.
   * Implementations must account for multi-threading, as multiple threads
   * can call this method simultaneously. */
public:
  virtual std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) = 0;

protected:
  std::shared_ptr<ContentSource> shared_from_this()
  {
    return std::static_pointer_cast<ContentSource>(
        ContentItemsSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
