#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::compound::hyphenation
{

/**
 * This class represents a hyphenated word.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class Hyphenation : public std::enable_shared_from_this<Hyphenation>
{
  GET_CLASS_NAME(Hyphenation)

private:
  std::deque<int> hyphenPoints;

  /**
   * rawWord as made of alternating strings and {@link Hyphen Hyphen} instances
   */
public:
  Hyphenation(std::deque<int> &points);

  /**
   * @return the number of hyphenation points in the word
   */
  virtual int length();

  /**
   * @return the hyphenation points
   */
  virtual std::deque<int> getHyphenationPoints();
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
