#pragma once
#include "HashFunction.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
 * This is a very fast, non-cryptographic hash suitable for general hash-based
 * lookup. See http://murmurhash.googlepages.com/ for more details.
 * <p>
 * The C version of MurmurHash 2.0 found at that site was ported to Java by
 * Andrzej Bialecki (ab at getopt org).
 * </p>
 * <p>
 *  The code from getopt.org was adapted by Mark Harwood in the form here as one
 * of a pluggable choice of hashing functions as the core function had to be
 * adapted to work with BytesRefs with offsets and lengths rather than raw byte
 * arrays.
 * </p>
 * @lucene.experimental
 */
class MurmurHash2 final : public HashFunction
{
  GET_CLASS_NAME(MurmurHash2)

public:
  static const std::shared_ptr<MurmurHash2> INSTANCE;

private:
  MurmurHash2();

public:
  static int hash(std::deque<char> &data, int seed, int offset, int len);

  /**
   * Generates 32 bit hash from byte array with default seed value.
   *
   * @param data
   *          byte array to hash
   * @param offset
   *          the start position in the array to hash
   * @param len
   *          length of the array elements to hash
   * @return 32 bit hash of the given array
   */
  static int hash32(std::deque<char> &data, int offset, int len);

  int hash(std::shared_ptr<BytesRef> br) override final;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MurmurHash2> shared_from_this()
  {
    return std::static_pointer_cast<MurmurHash2>(
        HashFunction::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/bloom/
