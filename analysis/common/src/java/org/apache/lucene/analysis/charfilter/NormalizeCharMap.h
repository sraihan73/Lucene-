#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
template <typename T>
class Arc;
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

using CharsRef = org::apache::lucene::util::CharsRef;
using FST = org::apache::lucene::util::fst::FST;

// TODO: save/load?

/**
 * Holds a map_obj of std::wstring input to std::wstring output, to be used
 * with {@link MappingCharFilter}.  Use the {@link Builder}
 * to create this.
 */
class NormalizeCharMap : public std::enable_shared_from_this<NormalizeCharMap>
{
  GET_CLASS_NAME(NormalizeCharMap)

public:
  const std::shared_ptr<FST<std::shared_ptr<CharsRef>>> map_obj;
  const std::unordered_map<wchar_t, FST::Arc<std::shared_ptr<CharsRef>>>
      cachedRootArcs =
          std::unordered_map<wchar_t, FST::Arc<std::shared_ptr<CharsRef>>>();

  // Use the builder to create:
private:
  NormalizeCharMap(std::shared_ptr<FST<std::shared_ptr<CharsRef>>> map_obj);

  /**
   * Builds an NormalizeCharMap.
   * <p>
   * Call add() until you have added all the mappings, then call build() to get
   * a NormalizeCharMap
   * @lucene.experimental
   */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)

  private:
    const std::unordered_map<std::wstring, std::wstring> pendingPairs =
        std::map_obj<std::wstring, std::wstring>();

    /** Records a replacement to be applied to the input
     *  stream.  Whenever <code>singleMatch</code> occurs in
     *  the input, it will be replaced with
     *  <code>replacement</code>.
     *
     * @param match input std::wstring to be replaced
     * @param replacement output std::wstring
     * @throws IllegalArgumentException if
     * <code>match</code> is the empty string, or was
     * already previously added
     */
  public:
    virtual void add(const std::wstring &match,
                     const std::wstring &replacement);

    /** Builds the NormalizeCharMap; call this once you
     *  are done calling {@link #add}. */
    virtual std::shared_ptr<NormalizeCharMap> build();
  };
};

} // namespace org::apache::lucene::analysis::charfilter
