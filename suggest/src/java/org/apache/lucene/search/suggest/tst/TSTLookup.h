#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/tst/TernaryTreeNode.h"

#include  "core/src/java/org/apache/lucene/search/suggest/tst/TSTAutocomplete.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
#include  "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
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
namespace org::apache::lucene::search::suggest::tst
{

using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Suggest implementation based on a
 * <a href="http://en.wikipedia.org/wiki/Ternary_search_tree">Ternary Search
 * Tree</a>
 *
 * @see TSTAutocomplete
 */
class TSTLookup : public Lookup
{
  GET_CLASS_NAME(TSTLookup)
public:
  std::shared_ptr<TernaryTreeNode> root = std::make_shared<TernaryTreeNode>();
  std::shared_ptr<TSTAutocomplete> autocomplete =
      std::make_shared<TSTAutocomplete>();

  /** Number of entries the lookup was built with */
private:
  int64_t count = 0;

  const std::shared_ptr<Directory> tempDir;
  const std::wstring tempFileNamePrefix;

  /**
   * Creates a new TSTLookup with an empty Ternary Search Tree.
   * @see #build(InputIterator)
   */
public:
  TSTLookup();

  /**
   * Creates a new TSTLookup, for building.
   * @see #build(InputIterator)
   */
  TSTLookup(std::shared_ptr<Directory> tempDir,
            const std::wstring &tempFileNamePrefix);

  // TODO: Review if this comparator is really needed for TST to work
  // correctly!!!

  /** TST uses UTF-16 sorting, so we need a suitable BytesRef comparator to do
   * this. */
private:
  static const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
      utf8SortedAsUTF16SortOrder;

public:
  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  /**
   * Adds a new node if <code>key</code> already exists,
   * otherwise replaces its value.
   * <p>
   * This method always returns true.
   */
  virtual bool add(std::shared_ptr<std::wstring> key, std::any value);

  /**
   * Returns the value for the specified key, or null
   * if the key does not exist.
   */
  virtual std::any get(std::shared_ptr<std::wstring> key);

private:
  static bool charSeqEquals(std::shared_ptr<std::wstring> left,
                            std::shared_ptr<std::wstring> right);

public:
  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) override;

private:
  static constexpr char LO_KID = 0x01;
  static constexpr char EQ_KID = 0x02;
  static constexpr char HI_KID = 0x04;
  static constexpr char HAS_TOKEN = 0x08;
  static constexpr char HAS_VALUE = 0x10;

  // pre-order traversal
  void
  readRecursively(std::shared_ptr<DataInput> in_,
                  std::shared_ptr<TernaryTreeNode> node) ;

  // pre-order traversal
  void
  writeRecursively(std::shared_ptr<DataOutput> out,
                   std::shared_ptr<TernaryTreeNode> node) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool store(std::shared_ptr<DataOutput> output)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool load(std::shared_ptr<DataInput> input)  override;

  /** Returns byte size of the underlying TST */
  int64_t ramBytesUsed() override;

  int64_t getCount() override;

protected:
  std::shared_ptr<TSTLookup> shared_from_this()
  {
    return std::static_pointer_cast<TSTLookup>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/tst/
