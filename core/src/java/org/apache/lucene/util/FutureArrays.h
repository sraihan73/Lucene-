#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

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
 * Additional methods from Java 9's <a
 * href="https://docs.oracle.com/javase/9/docs/api/java/util/Arrays.html">
 * {@code java.util.Arrays}</a>.
 * <p>
 * This class will be removed when Java 9 is minimum requirement.
 * Currently any bytecode is patched to use the Java 9 native
 * classes through MR-JAR (Multi-Release JAR) mechanism.
 * In Java 8 it will use THIS implementation.
 * Because of patching, inside the Java source files we always
 * refer to the Lucene implementations, but the final Lucene
 * JAR files will use the native Java 9 class names when executed
 * with Java 9.
 * @lucene.internal
 */
class FutureArrays final : public std::enable_shared_from_this<FutureArrays>
{
  GET_CLASS_NAME(FutureArrays)

private:
  FutureArrays();

  // methods in Arrays are defined stupid: they cannot use
  // Objects.checkFromToIndex they throw IAE (vs IOOBE) in the case of fromIndex
  // > toIndex. so this method works just like checkFromToIndex, but with that
  // stupidity added.
  static void checkFromToIndex(int fromIndex, int toIndex, int length);

  // byte[]

  /**
   * Behaves like Java 9's Arrays.mismatch
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#mismatch-byte:A-int-int-byte:A-int-int-">Arrays.mismatch</a>
   */
public:
  static int mismatch(std::deque<char> &a, int aFromIndex, int aToIndex,
                      std::deque<char> &b, int bFromIndex, int bToIndex);

  /**
   * Behaves like Java 9's Arrays.compareUnsigned
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#compareUnsigned-byte:A-int-int-byte:A-int-int-">Arrays.compareUnsigned</a>
   */
  static int compareUnsigned(std::deque<char> &a, int aFromIndex, int aToIndex,
                             std::deque<char> &b, int bFromIndex,
                             int bToIndex);

  /**
   * Behaves like Java 9's Arrays.equals
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#equals-byte:A-int-int-byte:A-int-int-">Arrays.equals</a>
   */
  static bool equals(std::deque<char> &a, int aFromIndex, int aToIndex,
                     std::deque<char> &b, int bFromIndex, int bToIndex);

  // char[]

  /**
   * Behaves like Java 9's Arrays.mismatch
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#mismatch-char:A-int-int-char:A-int-int-">Arrays.mismatch</a>
   */
  static int mismatch(std::deque<wchar_t> &a, int aFromIndex, int aToIndex,
                      std::deque<wchar_t> &b, int bFromIndex, int bToIndex);

  /**
   * Behaves like Java 9's Arrays.compare
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#compare-char:A-int-int-char:A-int-int-">Arrays.compare</a>
   */
  static int compare(std::deque<wchar_t> &a, int aFromIndex, int aToIndex,
                     std::deque<wchar_t> &b, int bFromIndex, int bToIndex);

  /**
   * Behaves like Java 9's Arrays.equals
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#equals-char:A-int-int-char:A-int-int-">Arrays.equals</a>
   */
  static bool equals(std::deque<wchar_t> &a, int aFromIndex, int aToIndex,
                     std::deque<wchar_t> &b, int bFromIndex, int bToIndex);

  // int[]

  /**
   * Behaves like Java 9's Arrays.compare
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#compare-int:A-int-int-int:A-int-int-">Arrays.compare</a>
   */
  static int compare(std::deque<int> &a, int aFromIndex, int aToIndex,
                     std::deque<int> &b, int bFromIndex, int bToIndex);

  /**
   * Behaves like Java 9's Arrays.equals
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#equals-int:A-int-int-int:A-int-int-">Arrays.equals</a>
   */
  static bool equals(std::deque<int> &a, int aFromIndex, int aToIndex,
                     std::deque<int> &b, int bFromIndex, int bToIndex);

  // long[]

  /**
   * Behaves like Java 9's Arrays.compare
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#compare-long:A-int-int-long:A-int-int-">Arrays.compare</a>
   */
  static int compare(std::deque<int64_t> &a, int aFromIndex, int aToIndex,
                     std::deque<int64_t> &b, int bFromIndex, int bToIndex);

  /**
   * Behaves like Java 9's Arrays.equals
   * @see <a
   * href="http://download.java.net/java/jdk9/docs/api/java/util/Arrays.html#equals-long:A-int-int-long:A-int-int-">Arrays.equals</a>
   */
  static bool equals(std::deque<int64_t> &a, int aFromIndex, int aToIndex,
                     std::deque<int64_t> &b, int bFromIndex, int bToIndex);
};

} // namespace org::apache::lucene::util
