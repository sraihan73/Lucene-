#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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
//    import static org.apache.lucene.util.packed.PackedInts.checkBlockSize;

using DataOutput = org::apache::lucene::store::DataOutput;

class AbstractBlockPackedWriter
    : public std::enable_shared_from_this<AbstractBlockPackedWriter>
{
  GET_CLASS_NAME(AbstractBlockPackedWriter)

public:
  static constexpr int MIN_BLOCK_SIZE = 64;
  static const int MAX_BLOCK_SIZE = 1 << (30 - 3);
  static const int MIN_VALUE_EQUALS_0 = 1 << 0;
  static constexpr int BPV_SHIFT = 1;

  // same as DataOutput.writeVLong but accepts negative values
  static void writeVLong(std::shared_ptr<DataOutput> out,
                         int64_t i) ;

protected:
  std::shared_ptr<DataOutput> out;
  std::deque<int64_t> const values;
  std::deque<char> blocks;
  int off = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ord_ = 0;
  bool finished = false;

  /**
   * Sole constructor.
   * @param blockSize the number of values of a single block, must be a multiple
   * of <tt>64</tt>
   */
public:
  AbstractBlockPackedWriter(std::shared_ptr<DataOutput> out, int blockSize);

  /** Reset this writer to wrap <code>out</code>. The block size remains
   * unchanged. */
  virtual void reset(std::shared_ptr<DataOutput> out);

private:
  void checkNotFinished();

  /** Append a new long. */
public:
  virtual void add(int64_t l) ;

  // For testing only
  virtual void addBlockOfZeros() ;

  /** Flush all buffered data to disk. This instance is not usable anymore
   *  after this method has been called until {@link #reset(DataOutput)} has
   *  been called. */
  virtual void finish() ;

  /** Return the number of values which have been added. */
  virtual int64_t ord();

protected:
  virtual void flush() = 0;

  void writeValues(int bitsRequired) ;
};

} // #include  "core/src/java/org/apache/lucene/util/packed/
