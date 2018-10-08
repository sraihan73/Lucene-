#pragma once
#include "ContentSource.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using com::ibm::icu::text::RuleBasedNumberFormat;

/**
 * Creates documents whose content is a <code>long</code> number starting from
 * <code>{@link Long#MIN_VALUE} + 10</code>.
 */
class LongToEnglishContentSource : public ContentSource
{
  GET_CLASS_NAME(LongToEnglishContentSource)
private:
  int64_t counter = 0;

public:
  virtual ~LongToEnglishContentSource();

  // TODO: we could take param to specify locale...
private:
  const std::shared_ptr<RuleBasedNumberFormat> rnbf =
      std::make_shared<RuleBasedNumberFormat>(Locale::ROOT,
                                              RuleBasedNumberFormat::SPELLOUT);

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

  void resetInputs()  override;

protected:
  std::shared_ptr<LongToEnglishContentSource> shared_from_this()
  {
    return std::static_pointer_cast<LongToEnglishContentSource>(
        ContentSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
