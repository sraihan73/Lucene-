#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class Spans;
}

namespace org::apache::lucene::search::spans
{
class SpanCollector;
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
namespace org::apache::lucene::search::spans
{

class ContainSpans : public ConjunctionSpans
{
  GET_CLASS_NAME(ContainSpans)
public:
  std::shared_ptr<Spans> sourceSpans;
  std::shared_ptr<Spans> bigSpans;
  std::shared_ptr<Spans> littleSpans;

  ContainSpans(std::shared_ptr<Spans> bigSpans,
               std::shared_ptr<Spans> littleSpans,
               std::shared_ptr<Spans> sourceSpans);

  int startPosition() override;

  int endPosition() override;

  int width() override;

  void
  collect(std::shared_ptr<SpanCollector> collector)  override;

protected:
  std::shared_ptr<ContainSpans> shared_from_this()
  {
    return std::static_pointer_cast<ContainSpans>(
        ConjunctionSpans::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
