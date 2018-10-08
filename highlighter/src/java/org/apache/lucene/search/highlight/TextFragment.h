#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::search::highlight
{
/**
 * Low-level class used to record information about a section of a document
 * with a score.
 *
 *
 */
class TextFragment : public std::enable_shared_from_this<TextFragment>
{
  GET_CLASS_NAME(TextFragment)
public:
  std::shared_ptr<std::wstring> markedUpText;
  int fragNum = 0;
  int textStartPos = 0;
  int textEndPos = 0;
  float score = 0;

  TextFragment(std::shared_ptr<std::wstring> markedUpText, int textStartPos,
               int fragNum);

  virtual void setScore(float score);
  virtual float getScore();
  /**
   * @param frag2 Fragment to be merged into this one
   */
  virtual void merge(std::shared_ptr<TextFragment> frag2);
  /**
   * @return true if this fragment follows the one passed
   */
  virtual bool follows(std::shared_ptr<TextFragment> fragment);

  /**
   * @return the fragment sequence number
   */
  virtual int getFragNum();

  /* Returns the marked-up text for this text fragment
   */
  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
