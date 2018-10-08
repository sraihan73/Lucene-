#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfos;
}

namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class FieldInfo;
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
namespace org::apache::lucene::index
{

/**
 * Shuffles field numbers around to try to trip bugs where field numbers
 * are assumed to always be consistent across segments.
 */
class MismatchedLeafReader : public FilterLeafReader
{
  GET_CLASS_NAME(MismatchedLeafReader)
public:
  const std::shared_ptr<FieldInfos> shuffled;

  /** Creates a new reader which will renumber fields in {@code in} */
  MismatchedLeafReader(std::shared_ptr<LeafReader> in_,
                       std::shared_ptr<Random> random);

  std::shared_ptr<FieldInfos> getFieldInfos() override;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  static std::shared_ptr<FieldInfos>
  shuffleInfos(std::shared_ptr<FieldInfos> infos,
               std::shared_ptr<Random> random);

  /**
   * StoredFieldsVisitor that remaps actual field numbers
   * to our new shuffled ones.
   */
  // TODO: its strange this part of our IR api exposes FieldInfo,
  // no other "user-accessible" codec apis do this?
public:
  class MismatchedVisitor : public StoredFieldVisitor
  {
    GET_CLASS_NAME(MismatchedVisitor)
  private:
    std::shared_ptr<MismatchedLeafReader> outerInstance;

  public:
    const std::shared_ptr<StoredFieldVisitor> in_;

    MismatchedVisitor(std::shared_ptr<MismatchedLeafReader> outerInstance,
                      std::shared_ptr<StoredFieldVisitor> in_);

    void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void intField(std::shared_ptr<FieldInfo> fieldInfo,
                  int value)  override;

    void longField(std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t value)  override;

    void floatField(std::shared_ptr<FieldInfo> fieldInfo,
                    float value)  override;

    void doubleField(std::shared_ptr<FieldInfo> fieldInfo,
                     double value)  override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) throw(
        IOException) override;

    virtual std::shared_ptr<FieldInfo>
    renumber(std::shared_ptr<FieldInfo> original);

  protected:
    std::shared_ptr<MismatchedVisitor> shared_from_this()
    {
      return std::static_pointer_cast<MismatchedVisitor>(
          StoredFieldVisitor::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MismatchedLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<MismatchedLeafReader>(
        FilterLeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
