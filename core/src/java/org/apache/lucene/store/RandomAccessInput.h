#pragma once
#include "stringhelper.h"
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
namespace org::apache::lucene::store
{

/**
 * Random Access Index API.
 * Unlike {@link IndexInput}, this has no concept of file position, all reads
 * are absolute. However, like IndexInput, it is only intended for use by a
 * single thread.
 */
class RandomAccessInput
{
  GET_CLASS_NAME(RandomAccessInput)

  /**
   * Reads a byte at the given position in the file
   * @see DataInput#readByte
   */
public:
  virtual char readByte(int64_t pos) = 0;
  /**
   * Reads a short at the given position in the file
   * @see DataInput#readShort
   */
  virtual short readShort(int64_t pos) = 0;
  /**
   * Reads an integer at the given position in the file
   * @see DataInput#readInt
   */
  virtual int readInt(int64_t pos) = 0;
  /**
   * Reads a long at the given position in the file
   * @see DataInput#readLong
   */
  virtual int64_t readLong(int64_t pos) = 0;
};

} // namespace org::apache::lucene::store
