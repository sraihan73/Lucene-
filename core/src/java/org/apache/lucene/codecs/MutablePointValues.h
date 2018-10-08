#pragma once
#include "../index/PointValues.h"
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::codecs
{

using PointValues = org::apache::lucene::index::PointValues;
using BytesRef = org::apache::lucene::util::BytesRef;

/** {@link PointValues} whose order of points can be changed.
 *  This class is useful for codecs to optimize flush.
 *  @lucene.internal */
class MutablePointValues : public PointValues
{
  GET_CLASS_NAME(MutablePointValues)

  /** Sole constructor. */
protected:
  MutablePointValues();

  /** Set {@code packedValue} with a reference to the packed bytes of the i-th
   * value. */
public:
  virtual void getValue(int i, std::shared_ptr<BytesRef> packedValue) = 0;

  /** Get the k-th byte of the i-th value. */
  virtual char getByteAt(int i, int k) = 0;

  /** Return the doc ID of the i-th value. */
  virtual int getDocID(int i) = 0;

  /** Swap the i-th and j-th values. */
  virtual void swap(int i, int j) = 0;

protected:
  std::shared_ptr<MutablePointValues> shared_from_this()
  {
    return std::static_pointer_cast<MutablePointValues>(
        org.apache.lucene.index.PointValues::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/
