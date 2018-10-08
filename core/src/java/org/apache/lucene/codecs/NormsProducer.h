#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"

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

using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using Accountable = org::apache::lucene::util::Accountable;

/** Abstract API that produces field normalization values
 *
 * @lucene.experimental
 */
class NormsProducer : public std::enable_shared_from_this<NormsProducer>,
                      public Accountable
{
  GET_CLASS_NAME(NormsProducer)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  NormsProducer();

  /** Returns {@link NumericDocValues} for this field.
   *  The returned instance need not be thread-safe: it will only be
   *  used by a single thread. */
public:
  virtual std::shared_ptr<NumericDocValues>
  getNorms(std::shared_ptr<FieldInfo> field) = 0;

  /**
   * Checks consistency of this producer
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
  virtual void checkIntegrity() = 0;

  /**
   * Returns an instance optimized for merging.
   * <p>
   * The default implementation returns {@code this} */
  virtual std::shared_ptr<NormsProducer> getMergeInstance() ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
