#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::codecs::bloom
{
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Base class for hashing functions that can be referred to by name.
 * Subclasses are expected to provide threadsafe implementations of the hash
 * function on the range of bytes referenced in the provided {@link BytesRef}
 * @lucene.experimental
 */
class HashFunction : public std::enable_shared_from_this<HashFunction>
{
  GET_CLASS_NAME(HashFunction)

  /**
   * Hashes the contents of the referenced bytes
   * @param bytes the data to be hashed
   * @return the hash of the bytes referenced by bytes.offset and length
   * bytes.length
   */
public:
  virtual int hash(std::shared_ptr<BytesRef> bytes) = 0;
};

} // namespace org::apache::lucene::codecs::bloom
