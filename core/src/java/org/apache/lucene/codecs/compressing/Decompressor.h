#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

namespace org::apache::lucene::util
{
class BytesRef;
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

using DataInput = org::apache::lucene::store::DataInput;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A decompressor.
 */
class Decompressor : public std::enable_shared_from_this<Decompressor>,
                     public Cloneable
{
  GET_CLASS_NAME(Decompressor)

  /** Sole constructor, typically called from sub-classes. */
protected:
  Decompressor();

  /**
   * Decompress bytes that were stored between offsets <code>offset</code> and
   * <code>offset+length</code> in the original stream from the compressed
   * stream <code>in</code> to <code>bytes</code>. After returning, the length
   * of <code>bytes</code> (<code>bytes.length</code>) must be equal to
   * <code>length</code>. Implementations of this method are free to resize
   * <code>bytes</code> depending on their needs.
   *
   * @param in the input that stores the compressed stream
   * @param originalLength the length of the original data (before compression)
   * @param offset bytes before this offset do not need to be decompressed
   * @param length bytes after <code>offset+length</code> do not need to be
   * decompressed
   * @param bytes a {@link BytesRef} where to store the decompressed data
   */
public:
  virtual void decompress(std::shared_ptr<DataInput> in_, int originalLength,
                          int offset, int length,
                          std::shared_ptr<BytesRef> bytes) = 0;

  std::shared_ptr<Decompressor> clone() = 0;
  override
};

} // namespace org::apache::lucene::codecs::compressing
