#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Bits.h"

#include  "core/src/java/org/apache/lucene/index/ReaderSlice.h"

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
namespace org::apache::lucene::index
{

using Bits = org::apache::lucene::util::Bits;

/**
 * Exposes a slice of an existing Bits as a new Bits.
 *
 * @lucene.internal
 */
class BitsSlice final : public std::enable_shared_from_this<BitsSlice>,
                        public Bits
{
  GET_CLASS_NAME(BitsSlice)
private:
  const std::shared_ptr<Bits> parent;
  const int start;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int length_;

  // start is inclusive; end is exclusive (length = end-start)
public:
  BitsSlice(std::shared_ptr<Bits> parent, std::shared_ptr<ReaderSlice> slice);

  bool get(int doc) override;

  int length() override;
};

} // #include  "core/src/java/org/apache/lucene/index/
