#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
namespace org::apache::lucene::queries::function::valuesource
{

using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/** A function with a single argument
 */
class SingleFunction : public ValueSource
{
  GET_CLASS_NAME(SingleFunction)
protected:
  const std::shared_ptr<ValueSource> source;

public:
  SingleFunction(std::shared_ptr<ValueSource> source);

protected:
  virtual std::wstring name() = 0;

public:
  std::wstring description() override;

  virtual int hashCode();

  virtual bool equals(std::any o);

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<SingleFunction> shared_from_this()
  {
    return std::static_pointer_cast<SingleFunction>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
