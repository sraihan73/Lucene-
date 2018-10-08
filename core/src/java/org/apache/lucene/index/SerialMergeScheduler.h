#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
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

/** A {@link MergeScheduler} that simply does each merge
 *  sequentially, using the current thread. */
class SerialMergeScheduler : public MergeScheduler
{
  GET_CLASS_NAME(SerialMergeScheduler)

  /** Sole constructor. */
public:
  SerialMergeScheduler();

  /** Just do the merges in sequence. We do this
   * "synchronized" so that even if the application is using
   * multiple threads, only one merge may run at a time. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
             bool newMergesFound)  override;

  virtual ~SerialMergeScheduler();

protected:
  std::shared_ptr<SerialMergeScheduler> shared_from_this()
  {
    return std::static_pointer_cast<SerialMergeScheduler>(
        MergeScheduler::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
