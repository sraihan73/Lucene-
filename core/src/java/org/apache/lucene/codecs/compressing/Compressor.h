#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataOutput;
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
namespace org::apache::lucene::codecs::compressing
{

using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * A data compressor.
 */
class Compressor : public std::enable_shared_from_this<Compressor>
{
  GET_CLASS_NAME(Compressor)

  /** Sole constructor, typically called from sub-classes. */
protected:
  Compressor();

  /**
   * Compress bytes into <code>out</code>. It it the responsibility of the
   * compressor to add all necessary information so that a {@link Decompressor}
   * will know when to stop decompressing bytes from the stream.
   */
public:
  virtual void compress(std::deque<char> &bytes, int off, int len,
                        std::shared_ptr<DataOutput> out) = 0;
};

} // namespace org::apache::lucene::codecs::compressing
