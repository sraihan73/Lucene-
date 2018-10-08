#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

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

class ParallelPostingsArray
    : public std::enable_shared_from_this<ParallelPostingsArray>
{
  GET_CLASS_NAME(ParallelPostingsArray)
public:
  static const int BYTES_PER_POSTING = 3 * Integer::BYTES;

  const int size;
  std::deque<int> const textStarts;
  std::deque<int> const intStarts;
  std::deque<int> const byteStarts;

  ParallelPostingsArray(int const size);

  virtual int bytesPerPosting();

  virtual std::shared_ptr<ParallelPostingsArray> newInstance(int size);

  std::shared_ptr<ParallelPostingsArray> grow();

  virtual void copyTo(std::shared_ptr<ParallelPostingsArray> toArray,
                      int numToCopy);
};

} // namespace org::apache::lucene::index
