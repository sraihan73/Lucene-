#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DoubleValuesSource;
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
namespace org::apache::lucene::expressions
{

using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;

/**
 * Binds variable names in expressions to actual data.
 * <p>
 * These are typically DocValues fields/FieldCache, the document's
 * relevance score, or other ValueSources.
 *
 * @lucene.experimental
 */
class Bindings : public std::enable_shared_from_this<Bindings>
{
  GET_CLASS_NAME(Bindings)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  Bindings();

  /**
   * Returns a DoubleValuesSource bound to the variable name
   */
public:
  virtual std::shared_ptr<DoubleValuesSource>
  getDoubleValuesSource(const std::wstring &name) = 0;
};

} // namespace org::apache::lucene::expressions
