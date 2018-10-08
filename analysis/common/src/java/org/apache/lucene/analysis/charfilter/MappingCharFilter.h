#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/CharSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "BaseCharFilter.h"
#include "stringhelper.h"
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class CharsRef;
}

namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::analysis::util
{
class RollingCharBuffer;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
namespace org::apache::lucene::analysis::charfilter
{
class NormalizeCharMap;
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
namespace org::apache::lucene::analysis::charfilter
{

using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharSequenceOutputs = org::apache::lucene::util::fst::CharSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;

/**
 * Simplistic {@link CharFilter} that applies the mappings
 * contained in a {@link NormalizeCharMap} to the character
 * stream, and correcting the resulting changes to the
 * offsets.  Matching is greedy (longest pattern matching at
 * a given point wins).  Replacement is allowed to be the
 * empty string.
 */

class MappingCharFilter : public BaseCharFilter
{
  GET_CLASS_NAME(MappingCharFilter)

private:
  const std::shared_ptr<Outputs<std::shared_ptr<CharsRef>>> outputs =
      CharSequenceOutputs::getSingleton();
  const std::shared_ptr<FST<std::shared_ptr<CharsRef>>> map_obj;
  const std::shared_ptr<FST::BytesReader> fstReader;
  const std::shared_ptr<RollingCharBuffer> buffer =
      std::make_shared<RollingCharBuffer>();
  const std::shared_ptr<FST::Arc<std::shared_ptr<CharsRef>>> scratchArc =
      std::make_shared<FST::Arc<std::shared_ptr<CharsRef>>>();
  const std::unordered_map<wchar_t, FST::Arc<std::shared_ptr<CharsRef>>>
      cachedRootArcs;

  std::shared_ptr<CharsRef> replacement;
  int replacementPointer = 0;
  int inputOff = 0;

  /** Default constructor that takes a {@link Reader}. */
public:
  MappingCharFilter(std::shared_ptr<NormalizeCharMap> normMap,
                    std::shared_ptr<Reader> in_);

  void reset()  override;

  int read()  override;

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

protected:
  std::shared_ptr<MappingCharFilter> shared_from_this()
  {
    return std::static_pointer_cast<MappingCharFilter>(
        BaseCharFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::charfilter
