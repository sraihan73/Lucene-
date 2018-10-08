#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <type_traits>

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
namespace org::apache::lucene::util
{

/**
 * Sneaky: rethrowing checked exceptions as unchecked
 * ones. Eh, it is sometimes useful...
 *
 * <p>Pulled from <a href="http://www.javapuzzlers.com">Java Puzzlers</a>.</p>
 * @see <a
 * href="http://www.amazon.com/Java-Puzzlers-Traps-Pitfalls-Corner/dp/032133678X">http://www.amazon.com/Java-Puzzlers-Traps-Pitfalls-Corner/dp/032133678X</a>
 */
class Rethrow final : public std::enable_shared_from_this<Rethrow>
{
  GET_CLASS_NAME(Rethrow)
private:
  Rethrow();

  /**
   * Rethrows <code>t</code> (identical object).
   */
public:
  static void rethrow(std::runtime_error t);

private:
  template <typename T>
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") private static <T extends
  // Throwable> void rethrow0(Throwable t) throws T
  static void rethrow0(std::runtime_error t) ;
};

} // #include  "core/src/java/org/apache/lucene/util/
