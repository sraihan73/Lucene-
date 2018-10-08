#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
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
namespace org::apache::lucene::facet::taxonomy::writercache
{

using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;

/**
 * Abstract class for storing Label-&gt;Ordinal mappings in a taxonomy.
 *
 * @lucene.experimental
 */
class LabelToOrdinal : public std::enable_shared_from_this<LabelToOrdinal>
{
  GET_CLASS_NAME(LabelToOrdinal)

  /** How many ordinals we've seen. */
protected:
  int counter = 0;

  /** Returned by {@link #getOrdinal} when the label isn't
   *  recognized. */
public:
  static constexpr int INVALID_ORDINAL = -2;

  /** Default constructor. */
  LabelToOrdinal();

  /**
   * return the maximal Ordinal assigned so far
   */
  virtual int getMaxOrdinal();

  /**
   * Returns the next unassigned ordinal. The default behavior of this method
   * is to simply increment a counter.
   */
  virtual int getNextOrdinal();

  /**
   * Adds a new label if its not yet in the table.
   * Throws an {@link IllegalArgumentException} if the same label with
   * a different ordinal was previoulsy added to this table.
   */
  virtual void addLabel(std::shared_ptr<FacetLabel> label, int ordinal) = 0;

  /**
   * Returns the ordinal assigned to the given label,
   * or {@link #INVALID_ORDINAL} if the label cannot be found in this table.
   */
  virtual int getOrdinal(std::shared_ptr<FacetLabel> label) = 0;
};

} // namespace org::apache::lucene::facet::taxonomy::writercache
