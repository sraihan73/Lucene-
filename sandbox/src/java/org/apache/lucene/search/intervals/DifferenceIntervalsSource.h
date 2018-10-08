#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::intervals
{
class IntervalsSource;
}

namespace org::apache::lucene::search::intervals
{
class DifferenceIntervalFunction;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
}
namespace org::apache::lucene::index
{
class Term;
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

namespace org::apache::lucene::search::intervals
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

class DifferenceIntervalsSource : public IntervalsSource
{
  GET_CLASS_NAME(DifferenceIntervalsSource)

public:
  const std::shared_ptr<IntervalsSource> minuend;
  const std::shared_ptr<IntervalsSource> subtrahend;
  const std::shared_ptr<DifferenceIntervalFunction> function;

  DifferenceIntervalsSource(
      std::shared_ptr<IntervalsSource> minuend,
      std::shared_ptr<IntervalsSource> subtrahend,
      std::shared_ptr<DifferenceIntervalFunction> function);

  std::shared_ptr<IntervalIterator>
  intervals(const std::wstring &field,
            std::shared_ptr<LeafReaderContext> ctx)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

  virtual std::wstring toString();

  void extractTerms(const std::wstring &field,
                    std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

protected:
  std::shared_ptr<DifferenceIntervalsSource> shared_from_this()
  {
    return std::static_pointer_cast<DifferenceIntervalsSource>(
        IntervalsSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
