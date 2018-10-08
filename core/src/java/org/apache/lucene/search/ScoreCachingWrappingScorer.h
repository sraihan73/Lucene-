#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Scorer;
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
namespace org::apache::lucene::search
{

/**
 * A {@link Scorer} which wraps another scorer and caches the score of the
 * current document. Successive calls to {@link #score()} will return the same
 * result and will not invoke the wrapped Scorer's score() method, unless the
 * current document has changed.<br>
 * This class might be useful due to the changes done to the {@link Collector}
 * interface, in which the score is not computed for a document by default, only
 * if the collector requests it. Some collectors may need to use the score in
 * several places, however all they have in hand is a {@link Scorer} object, and
 * might end up computing the score of a document more than once.
 */
class ScoreCachingWrappingScorer : public FilterScorer
{
  GET_CLASS_NAME(ScoreCachingWrappingScorer)

private:
  int curDoc = -1;
  float curScore = 0;

  /** Creates a new instance by wrapping the given scorer. */
public:
  ScoreCachingWrappingScorer(std::shared_ptr<Scorer> scorer);

  float score()  override;

  std::shared_ptr<std::deque<std::shared_ptr<ChildScorer>>>
  getChildren() override;

protected:
  std::shared_ptr<ScoreCachingWrappingScorer> shared_from_this()
  {
    return std::static_pointer_cast<ScoreCachingWrappingScorer>(
        FilterScorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
