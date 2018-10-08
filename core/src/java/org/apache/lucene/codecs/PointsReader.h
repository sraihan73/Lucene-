#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PointValues.h"

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
using Accountable = org::apache::lucene::util::Accountable;

/** Abstract API to visit point values.
 *
 * @lucene.experimental
 */
class PointsReader : public std::enable_shared_from_this<PointsReader>,
                     public Accountable
{
  GET_CLASS_NAME(PointsReader)

  /** Sole constructor. (For invocation by subclass constructors, typically
   * implicit.) */
protected:
  PointsReader();

  /**
   * Checks consistency of this reader.
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
public:
  virtual void checkIntegrity() = 0;

  /** Return {@link PointValues} for the given {@code field}. */
  virtual std::shared_ptr<PointValues> getValues(const std::wstring &field) = 0;

  /**
   * Returns an instance optimized for merging.
   * <p>
   * The default implementation returns {@code this} */
  virtual std::shared_ptr<PointsReader> getMergeInstance() ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
