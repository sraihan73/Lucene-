#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::queryparser::flexible::standard::config
{

using FuzzyQuery = org::apache::lucene::search::FuzzyQuery;

/**
 * Configuration parameters for {@link FuzzyQuery}s
 */
class FuzzyConfig : public std::enable_shared_from_this<FuzzyConfig>
{
  GET_CLASS_NAME(FuzzyConfig)

private:
  int prefixLength = FuzzyQuery::defaultPrefixLength;

  float minSimilarity = FuzzyQuery::defaultMinSimilarity;

public:
  FuzzyConfig();

  virtual int getPrefixLength();

  virtual void setPrefixLength(int prefixLength);

  virtual float getMinSimilarity();

  virtual void setMinSimilarity(float minSimilarity);
};

} // namespace org::apache::lucene::queryparser::flexible::standard::config
