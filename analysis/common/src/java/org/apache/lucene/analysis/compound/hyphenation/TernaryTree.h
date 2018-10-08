#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stack>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/CharVector.h"

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
 * <h2>Ternary Search Tree.</h2>
 *
 * <p>
 * A ternary search tree is a hybrid between a binary tree and a digital search
 * tree (trie). Keys are limited to strings. A data value of type char is stored
 * in each leaf node. It can be used as an index (or pointer) to the data.
 * Branches that only contain one key are compressed to one node by storing a
 * pointer to the trailer substring of the key. This class is intended to serve
 * as base class or helper class to implement Dictionary collections or the
 * like. Ternary trees have some nice properties as the following: the tree can
 * be traversed in sorted order, partial matches (wildcard) can be implemented,
 * retrieval of all keys within a given distance from the target, etc. The
 * storage requirements are higher than a binary tree but a lot less than a
 * trie. Performance is comparable with a hash table, sometimes it outperforms a
 * hash function (most of the time can determine a miss faster than a hash).
 * </p>
 *
 * <p>
 * The main purpose of this java port is to serve as a base for implementing
 * TeX's hyphenation algorithm (see The TeXBook, appendix H). Each language
 * requires from 5000 to 15000 hyphenation patterns which will be keys in this
 * tree. The strings patterns are usually small (from 2 to 5 characters), but
 * each char in the tree is stored in a node. Thus memory usage is the main
 * concern. We will sacrifice 'elegance' to keep memory requirements to the
 * minimum. Using java's char type as pointer (yes, I know pointer it is a
 * forbidden word in java) we can keep the size of the node to be just 8 bytes
 * (3 pointers and the data char). This gives room for about 65000 nodes. In my
 * tests the english patterns took 7694 nodes and the german patterns 10055
 * nodes, so I think we are safe.
 * </p>
 *
 * <p>
 * All said, this is a map_obj with strings as keys and char as value. Pretty
 * limited!. It can be extended to a general map_obj by using the string
 * representation of an object and using the char value as an index to an array
 * that contains the object values.
 * </p>
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */

class TernaryTree : public std::enable_shared_from_this<TernaryTree>,
                    public Cloneable
{
  GET_CLASS_NAME(TernaryTree)

  /**
   * We use 4 arrays to represent a node. I guess I should have created a proper
   * node class, but somehow Knuth's pascal code made me forget we now have a
   * portable language with virtual memory management and automatic garbage
   * collection! And now is kind of late, furthermore, if it ain't broken, don't
   * fix it.
   */

  /**
   * Pointer to low branch and to rest of the key when it is stored directly in
   * this node, we don't have unions in java!
   */
protected:
  std::deque<wchar_t> lo;

  /**
   * Pointer to high branch.
   */
  std::deque<wchar_t> hi;

  /**
   * Pointer to equal branch and to data when this node is a string terminator.
   */
  std::deque<wchar_t> eq;

  /**
   * <P>
   * The character stored in this node: splitchar. Two special values are
   * reserved:
   * </P>
   * <ul>
   * <li>0x0000 as string terminator</li>
   * <li>0xFFFF to indicate that the branch starting at this node is
   * compressed</li>
   * </ul>
   * <p>
   * This shouldn't be a problem if we give the usual semantics to strings since
   * 0xFFFF is guaranteed not to be an Unicode character.
   * </p>
   */
  std::deque<wchar_t> sc;

  /**
   * This deque holds the trailing of the keys when the branch is compressed.
   */
  std::shared_ptr<CharVector> kv;

  wchar_t root = L'\0';

  wchar_t freenode = L'\0';

  int length = 0; // number of items in tree

  static constexpr int BLOCK_SIZE = 2048; // allocation size for arrays

public:
  TernaryTree();

protected:
  virtual void init();

  /**
   * Branches are initially compressed, needing one node per key plus the size
   * of the string key. They are decompressed as needed when another key with
   * same prefix is inserted. This saves a lot of space, specially for long
   * keys.
   */
public:
  virtual void insert(const std::wstring &key, wchar_t val);

  virtual void insert(std::deque<wchar_t> &key, int start, wchar_t val);

  /**
   * The actual insertion function, recursive version.
   */
private:
  wchar_t insert(wchar_t p, std::deque<wchar_t> &key, int start, wchar_t val);

  /**
   * Compares 2 null terminated char arrays
   */
public:
  static int strcmp(std::deque<wchar_t> &a, int startA,
                    std::deque<wchar_t> &b, int startB);

  /**
   * Compares a string with null terminated char array
   */
  static int strcmp(const std::wstring &str, std::deque<wchar_t> &a,
                    int start);

  static void strcpy(std::deque<wchar_t> &dst, int di,
                     std::deque<wchar_t> &src, int si);

  static int strlen(std::deque<wchar_t> &a, int start);

  static int strlen(std::deque<wchar_t> &a);

  virtual int find(const std::wstring &key);

  virtual int find(std::deque<wchar_t> &key, int start);

  virtual bool knows(const std::wstring &key);

  // redimension the arrays
private:
  void redimNodeArrays(int newsize);

public:
  virtual int size();

  std::shared_ptr<TernaryTree> clone() override;

  /**
   * Recursively insert the median first and then the median of the lower and
   * upper halves, and so on in order to get a balanced tree. The array of keys
   * is assumed to be sorted in ascending order.
   */
protected:
  virtual void insertBalanced(std::deque<std::wstring> &k,
                              std::deque<wchar_t> &v, int offset, int n);

  /**
   * Balance the tree for best search performance
   */
public:
  virtual void balance();

  /**
   * Each node stores a character (splitchar) which is part of some key(s). In a
   * compressed branch (one that only contain a single string key) the trailer
   * of the key which is not already in nodes is stored externally in the kv
   * array. As items are inserted, key substrings decrease. Some substrings may
   * completely disappear when the whole branch is totally decompressed. The
   * tree is traversed to find the key substrings actually used. In addition,
   * duplicate substrings are removed using a map_obj (implemented with a
   * TernaryTree!).
   *
   */
  virtual void trimToSize();

private:
  void compact(std::shared_ptr<CharVector> kx, std::shared_ptr<TernaryTree> map_obj,
               wchar_t p);

public:
  virtual std::shared_ptr<java::util::Iterator<std::wstring>> keys();

public:
  class Iterator : public std::enable_shared_from_this<Iterator>,
                   public java::util::Iterator<std::wstring>
  {
    GET_CLASS_NAME(Iterator)
  private:
    std::shared_ptr<TernaryTree> outerInstance;

    /**
     * current node index
     */
  public:
    int cur = 0;

    /**
     * current key
     */
    std::wstring curkey;

  private:
    class Item : public std::enable_shared_from_this<Item>, public Cloneable
    {
      GET_CLASS_NAME(Item)
    private:
      std::shared_ptr<TernaryTree::Iterator> outerInstance;

    public:
      wchar_t parent = L'\0';

      wchar_t child = L'\0';

      Item(std::shared_ptr<TernaryTree::Iterator> outerInstance);

      Item(std::shared_ptr<TernaryTree::Iterator> outerInstance, wchar_t p,
           wchar_t c);

      std::shared_ptr<Item> clone() override;
    };

    /**
     * Node stack
     */
  public:
    std::stack<std::shared_ptr<Item>> ns;

    /**
     * key stack implemented with a StringBuilder
     */
    std::shared_ptr<StringBuilder> ks;

    Iterator(std::shared_ptr<TernaryTree> outerInstance);

    virtual void rewind();

    std::wstring nextElement() override;

    virtual wchar_t getValue();

    bool hasMoreElements() override;

    /**
     * traverse upwards
     */
  private:
    int up();

    /**
     * traverse the tree to find next key
     */
    int run();
  };

public:
  virtual void printStats(std::shared_ptr<PrintStream> out);
  /*
    public static void main(std::wstring[] args) {
      TernaryTree tt = new TernaryTree();
      tt.insert("Carlos", 'C');
      tt.insert("Car", 'r');
      tt.insert("palos", 'l');
      tt.insert("pa", 'p');
      tt.trimToSize();
      System.out.println((char) tt.find("Car"));
      System.out.println((char) tt.find("Carlos"));
      System.out.println((char) tt.find("alto"));
      tt.printStats(System.out);
    }
    */
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
