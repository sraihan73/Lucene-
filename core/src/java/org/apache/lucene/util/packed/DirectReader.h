#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RandomAccessInput;
}

namespace org::apache::lucene::util
{
class LongValues;
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

using RandomAccessInput = org::apache::lucene::store::RandomAccessInput;
using LongValues = org::apache::lucene::util::LongValues;

/**
 * Retrieves an instance previously written by {@link DirectWriter}
 * <p>
 * Example usage:
 * <pre class="prettyprint">
 *   int bitsPerValue = 100;
 *   IndexInput in = dir.openInput("packed", IOContext.DEFAULT);
 *   LongValues values = DirectReader.getInstance(in.randomAccessSlice(start,
 * end), bitsPerValue); for (int i = 0; i &lt; numValues; i++) { long value =
 * values.get(i);
 *   }
 * </pre>
 * @see DirectWriter
 */
class DirectReader : public std::enable_shared_from_this<DirectReader>
{
  GET_CLASS_NAME(DirectReader)

  /**
   * Retrieves an instance from the specified slice written decoding
   * {@code bitsPerValue} for each value
   */
public:
  static std::shared_ptr<LongValues>
  getInstance(std::shared_ptr<RandomAccessInput> slice, int bitsPerValue);

  /**
   * Retrieves an instance from the specified {@code offset} of the given slice
   * decoding {@code bitsPerValue} for each value
   */
  static std::shared_ptr<LongValues>
  getInstance(std::shared_ptr<RandomAccessInput> slice, int bitsPerValue,
              int64_t offset);

public:
  class DirectPackedReader1 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader1)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader1(std::shared_ptr<RandomAccessInput> in_,
                        int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader1> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader1>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader2 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader2)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader2(std::shared_ptr<RandomAccessInput> in_,
                        int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader2> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader2>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader4 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader4)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader4(std::shared_ptr<RandomAccessInput> in_,
                        int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader4> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader4>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader8 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader8)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader8(std::shared_ptr<RandomAccessInput> in_,
                        int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader8> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader8>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader12 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader12)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader12(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader12> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader12>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader16 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader16)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader16(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader16> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader16>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader20 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader20)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader20(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader20> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader20>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader24 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader24)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader24(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader24> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader24>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader28 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader28)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader28(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader28> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader28>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader32 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader32)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader32(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader32> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader32>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader40 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader40)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader40(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader40> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader40>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader48 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader48)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader48(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader48> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader48>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader56 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader56)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader56(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader56> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader56>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };

public:
  class DirectPackedReader64 final : public LongValues
  {
    GET_CLASS_NAME(DirectPackedReader64)
  public:
    const std::shared_ptr<RandomAccessInput> in_;
    const int64_t offset;

    DirectPackedReader64(std::shared_ptr<RandomAccessInput> in_,
                         int64_t offset);

    int64_t get(int64_t index) override;

  protected:
    std::shared_ptr<DirectPackedReader64> shared_from_this()
    {
      return std::static_pointer_cast<DirectPackedReader64>(
          org.apache.lucene.util.LongValues::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::util::packed
