#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cctype>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

/*
 * Copyright (c) 2005 Bruno Martins
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this deque of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this deque of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the organization nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
namespace org::apache::lucene::search::suggest::jaspell
{

using Accountable = org::apache::lucene::util::Accountable;

/**
 * Implementation of a Ternary Search Trie, a data structure for storing
 * <code>std::wstring</code> objects that combines the compact size of a binary search
 * tree with the speed of a digital search trie, and is therefore ideal for
 * practical use in sorting and searching data.
 *
 * <p>
 * This data structure is faster than hashing for many typical search problems,
 * and supports a broader range of useful problems and operations. Ternary
 * searches are faster than hashing and more powerful, too.
 * </p>
 *
 * <p>
 * The theory of ternary search trees was described at a symposium in 1997 (see
 * "Fast Algorithms for Sorting and Searching Strings," by J.L. Bentley and R.
 * Sedgewick, Proceedings of the 8th Annual ACM-SIAM Symposium on Discrete
 * Algorithms, January 1997). Algorithms in C, Third Edition, by Robert
 * Sedgewick (Addison-Wesley, 1998) provides yet another view of ternary search
 * trees.
 * </p>
 *
 * @deprecated Migrate to one of the newer suggesters which are much more RAM
 * efficient.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class JaspellTernarySearchTrie implements
// org.apache.lucene.util.Accountable
class JaspellTernarySearchTrie
    : public std::enable_shared_from_this<JaspellTernarySearchTrie>,
      public Accountable
{

  /**
   * An inner class of Ternary Search Trie that represents a node in the trie.
   */
protected:
  class TSTNode final : public std::enable_shared_from_this<TSTNode>,
                        public Accountable
  {
    GET_CLASS_NAME(TSTNode)

    /** Index values for accessing relatives array. */
  protected:
    static constexpr int PARENT = 0, LOKID = 1, EQKID = 2, HIKID = 3;

    /** The key to the node. */
    std::any data;

    /** The relative nodes. */
    std::deque<std::shared_ptr<TSTNode>> const relatives =
        std::deque<std::shared_ptr<TSTNode>>(4);

    /** The char used in the split. */
    wchar_t splitchar = L'\0';

    /**
     * Constructor method.
     *
     *@param splitchar
     *          The char used in the split.
     *@param parent
     *          The parent node.
     */
    TSTNode(wchar_t splitchar, std::shared_ptr<TSTNode> parent);

  public:
    int64_t ramBytesUsed() override;
  };

  /**
   * Compares characters by alfabetical order.
   *
   *@param cCompare2
   *          The first char in the comparison.
   *@param cRef
   *          The second char in the comparison.
   *@return A negative number, 0 or a positive number if the second char is
   *         less, equal or greater.
   */
private:
  static int compareCharsAlphabetically(wchar_t cCompare2, wchar_t cRef);

  /* what follows is the original Jaspell code.
  private static int compareCharsAlphabetically(int cCompare2, int cRef) {
    int cCompare = 0;
    if (cCompare2 >= 65) {
      if (cCompare2 < 89) {
        cCompare = (2 * cCompare2) - 65;
      } else if (cCompare2 < 97) {
        cCompare = cCompare2 + 24;
      } else if (cCompare2 < 121) {
        cCompare = (2 * cCompare2) - 128;
      } else cCompare = cCompare2;
    } else cCompare = cCompare2;
    if (cRef < 65) {
      return cCompare - cRef;
    }
    if (cRef < 89) {
      return cCompare - ((2 * cRef) - 65);
    }
    if (cRef < 97) {
      return cCompare - (cRef + 24);
    }
    if (cRef < 121) {
      return cCompare - ((2 * cRef) - 128);
    }
    return cCompare - cRef;
  }
  */

  /**
   * The default number of values returned by the <code>matchAlmost</code>
   * method.
   */
  int defaultNumReturnValues = -1;

  /**
   * the number of differences allowed in a call to the
   * <code>matchAlmostKey</code> method.
   */
  int matchAlmostDiff = 0;

  /** The base node in the trie. */
  std::shared_ptr<TSTNode> rootNode;

  const std::shared_ptr<Locale> locale;

  /**
   * Constructs an empty Ternary Search Trie.
   */
public:
  JaspellTernarySearchTrie();

  /**
   * Constructs an empty Ternary Search Trie,
   * specifying the Locale used for lowercasing.
   */
  JaspellTernarySearchTrie(std::shared_ptr<Locale> locale);

  // for loading
  virtual void setRoot(std::shared_ptr<TSTNode> newRoot);

  // for saving
  virtual std::shared_ptr<TSTNode> getRoot();

  /**
   * Constructs a Ternary Search Trie and loads data from a <code>Path</code>
   * into the Trie. The file is a normal text document, where each line is of
   * the form word TAB float.
   *
   *@param file
   *          The <code>Path</code> with the data to load into the Trie.
   *@exception IOException
   *              A problem occured while reading the data.
   */
  JaspellTernarySearchTrie(std::shared_ptr<Path> file) ;

  /**
   * Constructs a Ternary Search Trie and loads data from a <code>File</code>
   * into the Trie. The file is a normal text document, where each line is of
   * the form "word TAB float".
   *
   *@param file
   *          The <code>File</code> with the data to load into the Trie.
   *@param compression
   *          If true, the file is compressed with the GZIP algorithm, and if
   *          false, the file is a normal text document.
   *@exception IOException
   *              A problem occured while reading the data.
   */
  JaspellTernarySearchTrie(std::shared_ptr<Path> file,
                           bool compression) ;

  /**
   * Deletes the node passed in as an argument. If this node has non-null data,
   * then both the node and the data will be deleted. It also deletes any other
   * nodes in the trie that are no longer needed after the deletion of the node.
   *
   *@param nodeToDelete
   *          The node to delete.
   */
private:
  void deleteNode(std::shared_ptr<TSTNode> nodeToDelete);

  /**
   * Recursively visits each node to be deleted.
   *
   * To delete a node, first set its data to null, then pass it into this
   * method, then pass the node returned by this method into this method (make
   * sure you don't delete the data of any of the nodes returned from this
   * method!) and continue in this fashion until the node returned by this
   * method is <code>null</code>.
   *
   * The TSTNode instance returned by this method will be next node to be
   * operated on by <code>deleteNodeRecursion</code> (This emulates recursive
   * method call while avoiding the JVM overhead normally associated with a
   * recursive method.)
   *
   *@param currentNode
   *          The node to delete.
   *@return The next node to be called in deleteNodeRecursion.
   */
  std::shared_ptr<TSTNode>
  deleteNodeRecursion(std::shared_ptr<TSTNode> currentNode);

  /**
   * Retrieve the object indexed by a key.
   *
   *@param key
   *          A <code>std::wstring</code> index.
   *@return The object retrieved from the Ternary Search Trie.
   */
public:
  virtual std::any get(std::shared_ptr<std::wstring> key);

  /**
   * Retrieve the <code>Float</code> indexed by key, increment it by one unit
   * and store the new <code>Float</code>.
   *
   *@param key
   *          A <code>std::wstring</code> index.
   *@return The <code>Float</code> retrieved from the Ternary Search Trie.
   */
  virtual std::optional<float> getAndIncrement(const std::wstring &key);

  /**
   * Returns the key that indexes the node argument.
   *
   *@param node
   *          The node whose index is to be calculated.
   *@return The <code>std::wstring</code> that indexes the node argument.
   */
protected:
  virtual std::wstring getKey(std::shared_ptr<TSTNode> node);

  /**
   * Returns the node indexed by key, or <code>null</code> if that node doesn't
   * exist. Search begins at root node.
   *
   *@param key
   *          A <code>std::wstring</code> that indexes the node that is returned.
   *@return The node object indexed by key. This object is an instance of an
   *         inner class named <code>TernarySearchTrie.TSTNode</code>.
   */
public:
  virtual std::shared_ptr<TSTNode> getNode(std::shared_ptr<std::wstring> key);

  /**
   * Returns the node indexed by key, or <code>null</code> if that node doesn't
   * exist. The search begins at root node.
   *
   *@param key
   *          A <code>std::wstring</code> that indexes the node that is returned.
   *@param startNode
   *          The top node defining the subtrie to be searched.
   *@return The node object indexed by key. This object is an instance of an
   *         inner class named <code>TernarySearchTrie.TSTNode</code>.
   */
protected:
  virtual std::shared_ptr<TSTNode> getNode(std::shared_ptr<std::wstring> key,
                                           std::shared_ptr<TSTNode> startNode);

  /**
   * Returns the node indexed by key, creating that node if it doesn't exist,
   * and creating any required intermediate nodes if they don't exist.
   *
   *@param key
   *          A <code>std::wstring</code> that indexes the node that is returned.
   *@return The node object indexed by key. This object is an instance of an
   *         inner class named <code>TernarySearchTrie.TSTNode</code>.
   *@exception NullPointerException
   *              If the key is <code>null</code>.
   *@exception IllegalArgumentException
   *              If the key is an empty <code>std::wstring</code>.
   */
  virtual std::shared_ptr<TSTNode> getOrCreateNode(
      std::shared_ptr<std::wstring> key) throw(NullPointerException,
                                               std::invalid_argument);

  /**
   * Returns a <code>List</code> of keys that almost match the argument key.
   * Keys returned will have exactly diff characters that do not match the
   * target key, where diff is equal to the last value passed in as an argument
   * to the <code>setMatchAlmostDiff</code> method.
   * <p>
   * If the <code>matchAlmost</code> method is called before the
   * <code>setMatchAlmostDiff</code> method has been called for the first time,
   * then diff = 0.
   *
   *@param key
   *          The target key.
   *@return A <code>List</code> with the results.
   */
public:
  virtual std::deque<std::wstring> matchAlmost(const std::wstring &key);

  /**
   * Returns a <code>List</code> of keys that almost match the argument key.
   * Keys returned will have exactly diff characters that do not match the
   * target key, where diff is equal to the last value passed in as an argument
   * to the <code>setMatchAlmostDiff</code> method.
   * <p>
   * If the <code>matchAlmost</code> method is called before the
   * <code>setMatchAlmostDiff</code> method has been called for the first time,
   * then diff = 0.
   *
   *@param key
   *          The target key.
   *@param numReturnValues
   *          The maximum number of values returned by this method.
   *@return A <code>List</code> with the results
   */
  virtual std::deque<std::wstring>
  matchAlmost(std::shared_ptr<std::wstring> key, int numReturnValues);

  /**
   * Recursivelly vists the nodes in order to find the ones that almost match a
   * given key.
   *
   *@param currentNode
   *          The current node.
   *@param charIndex
   *          The current char.
   *@param d
   *          The number of differences so far.
   *@param matchAlmostNumReturnValues
   *          The maximum number of values in the result <code>List</code>.
   *@param matchAlmostResult2
   *          The results so far.
   *@param upTo
   *          If true all keys having up to and including matchAlmostDiff
   *          mismatched letters will be included in the result (including a key
   *          that is exactly the same as the target string) otherwise keys will
   *          be included in the result only if they have exactly
   *          matchAlmostDiff number of mismatched letters.
   *@param matchAlmostKey
   *          The key being searched.
   *@return A <code>List</code> with the results.
   */
private:
  std::deque<std::wstring>
  matchAlmostRecursion(std::shared_ptr<TSTNode> currentNode, int charIndex,
                       int d, std::shared_ptr<std::wstring> matchAlmostKey,
                       int matchAlmostNumReturnValues,
                       std::deque<std::wstring> &matchAlmostResult2,
                       bool upTo);

  /**
   * Returns an alphabetical <code>List</code> of all keys in the trie that
   * begin with a given prefix. Only keys for nodes having non-null data are
   * included in the <code>List</code>.
   *
   *@param prefix
   *          Each key returned from this method will begin with the characters
   *          in prefix.
   *@return A <code>List</code> with the results.
   */
public:
  virtual std::deque<std::wstring> matchPrefix(const std::wstring &prefix);

  /**
   * Returns an alphabetical <code>List</code> of all keys in the trie that
   * begin with a given prefix. Only keys for nodes having non-null data are
   * included in the <code>List</code>.
   *
   *@param prefix
   *          Each key returned from this method will begin with the characters
   *          in prefix.
   *@param numReturnValues
   *          The maximum number of values returned from this method.
   *@return A <code>List</code> with the results
   */
  virtual std::deque<std::wstring>
  matchPrefix(std::shared_ptr<std::wstring> prefix, int numReturnValues);

  /**
   * Returns the number of nodes in the trie that have non-null data.
   *
   *@return The number of nodes in the trie that have non-null data.
   */
  virtual int numDataNodes();

  /**
   * Returns the number of nodes in the subtrie below and including the starting
   * node. The method counts only nodes that have non-null data.
   *
   *@param startingNode
   *          The top node of the subtrie. the node that defines the subtrie.
   *@return The total number of nodes in the subtrie.
   */
protected:
  virtual int numDataNodes(std::shared_ptr<TSTNode> startingNode);

  /**
   * Returns the total number of nodes in the trie. The method counts nodes
   * whether or not they have data.
   *
   *@return The total number of nodes in the trie.
   */
public:
  virtual int numNodes();

  /**
   * Returns the total number of nodes in the subtrie below and including the
   * starting Node. The method counts nodes whether or not they have data.
   *
   *@param startingNode
   *          The top node of the subtrie. The node that defines the subtrie.
   *@return The total number of nodes in the subtrie.
   */
protected:
  virtual int numNodes(std::shared_ptr<TSTNode> startingNode);

  /**
   * Stores a value in the trie. The value may be retrieved using the key.
   *
   *@param key
   *          A <code>std::wstring</code> that indexes the object to be stored.
   *@param value
   *          The object to be stored in the Trie.
   */
public:
  virtual void put(std::shared_ptr<std::wstring> key, std::any value);

  /**
   * Recursivelly visists each node to calculate the number of nodes.
   *
   *@param currentNode
   *          The current node.
   *@param checkData
   *          If true we check the data to be different of <code>null</code>.
   *@param numNodes2
   *          The number of nodes so far.
   *@return The number of nodes accounted.
   */
private:
  int recursiveNodeCalculator(std::shared_ptr<TSTNode> currentNode,
                              bool checkData, int numNodes2);

  /**
   * Removes the value indexed by key. Also removes all nodes that are rendered
   * unnecessary by the removal of this data.
   *
   *@param key
   *          A <code>string</code> that indexes the object to be removed from
   *          the Trie.
   */
public:
  virtual void remove(const std::wstring &key);

  /**
   * Sets the number of characters by which words can differ from target word
   * when calling the <code>matchAlmost</code> method.
   * <p>
   * Arguments less than 0 will set the char difference to 0, and arguments
   * greater than 3 will set the char difference to 3.
   *
   *@param diff
   *          The number of characters by which words can differ from target
   *          word.
   */
  virtual void setMatchAlmostDiff(int diff);

  /**
   * Sets the default maximum number of values returned from the
   * <code>matchPrefix</code> and <code>matchAlmost</code> methods.
   * <p>
   * The value should be set this to -1 to get an unlimited number of return
   * values. note that the methods mentioned above provide overloaded versions
   * that allow you to specify the maximum number of return values, in which
   * case this value is temporarily overridden.
   *
   **@param num
   *          The number of values that will be returned when calling the
   *          methods above.
   */
  virtual void setNumReturnValues(int num);

  /**
   * Returns keys sorted in alphabetical order. This includes the start Node and
   * all nodes connected to the start Node.
   * <p>
   * The number of keys returned is limited to numReturnValues. To get a deque
   * that isn't limited in size, set numReturnValues to -1.
   *
   *@param startNode
   *          The top node defining the subtrie to be searched.
   *@param numReturnValues
   *          The maximum number of values returned from this method.
   *@return A <code>List</code> with the results.
   */
protected:
  virtual std::deque<std::wstring> sortKeys(std::shared_ptr<TSTNode> startNode,
                                             int numReturnValues);

  /**
   * Returns keys sorted in alphabetical order. This includes the current Node
   * and all nodes connected to the current Node.
   * <p>
   * Sorted keys will be appended to the end of the resulting <code>List</code>.
   * The result may be empty when this method is invoked, but may not be
   * <code>null</code>.
   *
   *@param currentNode
   *          The current node.
   *@param sortKeysNumReturnValues
   *          The maximum number of values in the result.
   *@param sortKeysResult2
   *          The results so far.
   *@return A <code>List</code> with the results.
   */
private:
  std::deque<std::wstring>
  sortKeysRecursion(std::shared_ptr<TSTNode> currentNode,
                    int sortKeysNumReturnValues,
                    std::deque<std::wstring> &sortKeysResult2);

public:
  int64_t ramBytesUsed() override;
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/jaspell/
