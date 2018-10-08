#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/Sorter.h"

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

class StringMSBRadixSorter : public MSBRadixSorter
{
  GET_CLASS_NAME(StringMSBRadixSorter)

public:
  StringMSBRadixSorter();

  /** Get a {@link BytesRef} for the given index. */
protected:
  virtual std::shared_ptr<BytesRef> get(int i) = 0;

  int byteAt(int i, int k) override;

  std::shared_ptr<Sorter> getFallbackSorter(int k) override;

private:
  class IntroSorterAnonymousInnerClass : public IntroSorter
  {
    GET_CLASS_NAME(IntroSorterAnonymousInnerClass)
  private:
    std::shared_ptr<StringMSBRadixSorter> outerInstance;

    int k = 0;

  public:
    IntroSorterAnonymousInnerClass(
        std::shared_ptr<StringMSBRadixSorter> outerInstance, int k);

  private:
    void get(int i, int k, std::shared_ptr<BytesRef> scratch);

  protected:
    void swap(int i, int j) override;

    int compare(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  private:
    const std::shared_ptr<BytesRef> pivot;

  protected:
    std::shared_ptr<IntroSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSorterAnonymousInnerClass>(
          IntroSorter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<StringMSBRadixSorter> shared_from_this()
  {
    return std::static_pointer_cast<StringMSBRadixSorter>(
        MSBRadixSorter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
