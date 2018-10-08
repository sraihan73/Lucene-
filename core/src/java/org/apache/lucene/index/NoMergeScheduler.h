#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergeScheduler;
}

namespace org::apache::lucene::index
{
class IndexWriter;
}
namespace org::apache::lucene::index
{
class MergePolicy;
}
namespace org::apache::lucene::index
{
class OneMerge;
}
namespace org::apache::lucene::store
{
class Directory;
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

using OneMerge = org::apache::lucene::index::MergePolicy::OneMerge;
using Directory = org::apache::lucene::store::Directory;

/**
 * A {@link MergeScheduler} which never executes any merges. It is also a
 * singleton and can be accessed through {@link NoMergeScheduler#INSTANCE}. Use
 * it if you want to prevent an {@link IndexWriter} from ever executing merges,
 * regardless of the {@link MergePolicy} used. Note that you can achieve the
 * same thing by using {@link NoMergePolicy}, however with
 * {@link NoMergeScheduler} you also ensure that no unnecessary code of any
 * {@link MergeScheduler} implementation is ever executed. Hence it is
 * recommended to use both if you want to disable merges from ever happening.
 */
class NoMergeScheduler final : public MergeScheduler
{
  GET_CLASS_NAME(NoMergeScheduler)

  /** The single instance of {@link NoMergeScheduler} */
public:
  static const std::shared_ptr<MergeScheduler> INSTANCE;

private:
  NoMergeScheduler();

public:
  virtual ~NoMergeScheduler();
  void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
             bool newMergesFound) override;
  std::shared_ptr<Directory>
  wrapForMerge(std::shared_ptr<OneMerge> merge,
               std::shared_ptr<Directory> in_) override;

  std::shared_ptr<MergeScheduler> clone() override;

protected:
  std::shared_ptr<NoMergeScheduler> shared_from_this()
  {
    return std::static_pointer_cast<NoMergeScheduler>(
        MergeScheduler::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
