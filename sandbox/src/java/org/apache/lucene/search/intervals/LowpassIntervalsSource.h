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

namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::intervals
{
class IntervalIterator;
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

class LowpassIntervalsSource : public IntervalsSource
{
  GET_CLASS_NAME(LowpassIntervalsSource)

public:
  const std::shared_ptr<IntervalsSource> in_;

private:
  const int maxWidth;

public:
  LowpassIntervalsSource(std::shared_ptr<IntervalsSource> in_, int maxWidth);

  virtual bool equals(std::any o);

  virtual std::wstring toString();

  void extractTerms(const std::wstring &field,
                    std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  std::shared_ptr<IntervalIterator>
  intervals(const std::wstring &field,
            std::shared_ptr<LeafReaderContext> ctx)  override;

private:
  class IntervalFilterAnonymousInnerClass : public IntervalFilter
  {
    GET_CLASS_NAME(IntervalFilterAnonymousInnerClass)
  private:
    std::shared_ptr<LowpassIntervalsSource> outerInstance;

    std::shared_ptr<org::apache::lucene::search::intervals::IntervalIterator> i;

  public:
    IntervalFilterAnonymousInnerClass(
        std::shared_ptr<LowpassIntervalsSource> outerInstance,
        std::shared_ptr<
            org::apache::lucene::search::intervals::IntervalIterator>
            i);

  protected:
    bool accept() override;

  protected:
    std::shared_ptr<IntervalFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntervalFilterAnonymousInnerClass>(
          IntervalFilter::shared_from_this());
    }
  };

public:
  virtual int hashCode();

protected:
  std::shared_ptr<LowpassIntervalsSource> shared_from_this()
  {
    return std::static_pointer_cast<LowpassIntervalsSource>(
        IntervalsSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::intervals
