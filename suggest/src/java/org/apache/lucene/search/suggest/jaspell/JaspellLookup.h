#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/jaspell/JaspellTernarySearchTrie.h"

#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
#include  "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/search/suggest/jaspell/TSTNode.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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
namespace org::apache::lucene::search::suggest::jaspell
{

using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using TSTNode = org::apache::lucene::search::suggest::jaspell::
    JaspellTernarySearchTrie::TSTNode;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Suggest implementation based on
 * <a href="http://jaspell.sourceforge.net/">JaSpell</a>.
 *
 * @see JaspellTernarySearchTrie
 * @deprecated Migrate to one of the newer suggesters which are much more RAM
 * efficient.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class JaspellLookup extends
// org.apache.lucene.search.suggest.Lookup implements
// org.apache.lucene.util.Accountable
class JaspellLookup : public Lookup, public Accountable
{
public:
  std::shared_ptr<JaspellTernarySearchTrie> trie =
      std::make_shared<JaspellTernarySearchTrie>();

private:
  bool usePrefix = true;
  int editDistance = 2;

  /** Number of entries the lookup was built with */
  int64_t count = 0;

  /**
   * Creates a new empty trie
   * @see #build(InputIterator)
   * */
public:
  JaspellLookup();

  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  /**
   * Adds a new node if <code>key</code> already exists,
   * otherwise replaces its value.
   * <p>
   * This method always returns false.
   */
  virtual bool add(std::shared_ptr<std::wstring> key, std::any value);

  /**
   * Returns the value for the specified key, or null
   * if the key does not exist.
   */
  virtual std::any get(std::shared_ptr<std::wstring> key);

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) override;

private:
  static constexpr char LO_KID = 0x01;
  static constexpr char EQ_KID = 0x02;
  static constexpr char HI_KID = 0x04;
  static constexpr char HAS_VALUE = 0x08;

  void readRecursively(std::shared_ptr<DataInput> in_,
                       std::shared_ptr<TSTNode> node) ;

  void writeRecursively(std::shared_ptr<DataOutput> out,
                        std::shared_ptr<TSTNode> node) ;

public:
  bool store(std::shared_ptr<DataOutput> output)  override;

  bool load(std::shared_ptr<DataInput> input)  override;

  int64_t ramBytesUsed() override;

  int64_t getCount() override;

protected:
  std::shared_ptr<JaspellLookup> shared_from_this()
  {
    return std::static_pointer_cast<JaspellLookup>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/jaspell/
