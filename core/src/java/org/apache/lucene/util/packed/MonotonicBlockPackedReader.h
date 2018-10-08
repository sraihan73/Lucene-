#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedInts;
}

namespace org::apache::lucene::util::packed
{
class Reader;
}
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::util::packed
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MAX_BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.packed.AbstractBlockPackedWriter.MIN_BLOCK_SIZE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.packed.PackedInts.numBlocks;

using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using LongValues = org::apache::lucene::util::LongValues;

/**
 * Provides random access to a stream written with
 * {@link MonotonicBlockPackedWriter}.
 * @lucene.internal
 */
class MonotonicBlockPackedReader : public LongValues, public Accountable
{
  GET_CLASS_NAME(MonotonicBlockPackedReader)

public:
  static int64_t expected(int64_t origin, float average, int index);

  const int blockShift, blockMask;
  const int64_t valueCount;
  std::deque<int64_t> const minValues;
  std::deque<float> const averages;
  std::deque<std::shared_ptr<PackedInts::Reader>> const subReaders;
  const int64_t sumBPV;

  /** Sole constructor. */
  static std::shared_ptr<MonotonicBlockPackedReader>
  of(std::shared_ptr<IndexInput> in_, int packedIntsVersion, int blockSize,
     int64_t valueCount, bool direct) ;

private:
  MonotonicBlockPackedReader(std::shared_ptr<IndexInput> in_,
                             int packedIntsVersion, int blockSize,
                             int64_t valueCount,
                             bool direct) ;

public:
  int64_t get(int64_t index) override;

  /** Returns the number of values */
  virtual int64_t size();

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<MonotonicBlockPackedReader> shared_from_this()
  {
    return std::static_pointer_cast<MonotonicBlockPackedReader>(
        org.apache.lucene.util.LongValues::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::packed
