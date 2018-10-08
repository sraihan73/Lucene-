#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename T>
class UnCompiledNode;
}

namespace org::apache::lucene::util::fst
{
class CompiledNode;
}
namespace org::apache::lucene::util::fst
{
class Node;
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
namespace org::apache::lucene::util::fst
{

using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;

// TODO: could we somehow stream an FST to disk while we
// build it?

/**
 * Builds a minimal FST (maps an IntsRef term to an arbitrary
 * output) from pre-sorted terms with outputs.  The FST
 * becomes an FSA if you use NoOutputs.  The FST is written
 * on-the-fly into a compact serialized format byte array, which can
 * be saved to / loaded from a Directory or used directly
 * for traversal.  The FST is always finite (no cycles).
 *
 * <p>NOTE: The algorithm is described at
 * http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.24.3698</p>
 *
 * <p>The parameterized type T is the output type.  See the
 * subclasses of {@link Outputs}.
 *
 * <p>FSTs larger than 2.1GB are now possible (as of Lucene
 * 4.2).  FSTs containing more than 2.1B nodes are also now
 * possible, however they cannot be packed.
 *
 * @lucene.experimental
 */

template <typename T>
class Builder : public std::enable_shared_from_this<Builder>
{
  GET_CLASS_NAME(Builder)
private:
  const std::shared_ptr<NodeHash<T>> dedupHash;

public:
  const std::shared_ptr<FST<T>> fst;

private:
  const T NO_OUTPUT;

  // private static final bool DEBUG = true;

  // simplistic pruning: we prune node (and all following
  // nodes) if less than this number of terms go through it:
  const int minSuffixCount1;

  // better pruning: we prune node (and all following
  // nodes) if the prior node has less than this number of
  // terms go through it:
  const int minSuffixCount2;

  const bool doShareNonSingletonNodes;
  const int shareMaxTailLength;

  const std::shared_ptr<IntsRefBuilder> lastInput =
      std::make_shared<IntsRefBuilder>();

  // NOTE: cutting this over to ArrayList instead loses ~6%
  // in build performance on 9.8M Wikipedia terms; so we
  // left this as an array:
  // current "frontier"
  std::deque<UnCompiledNode<T>> frontier;

  // Used for the BIT_TARGET_NEXT optimization (whereby
  // instead of storing the address of the target node for
  // a given arc, we mark a single bit noting that the next
  // node in the byte[] is the target node):
public:
  int64_t lastFrozenNode = 0;

  // Reused temporarily while building the FST:
  std::deque<int> reusedBytesPerArc = std::deque<int>(4);

  int64_t arcCount = 0;
  int64_t nodeCount = 0;

  bool allowArrayArcs = false;

  std::shared_ptr<BytesStore> bytes;

  /**
   * Instantiates an FST/FSA builder without any pruning. A shortcut
   * to {@link #Builder(FST.INPUT_TYPE, int, int, bool,
   * bool, int, Outputs, bool, int)} with pruning options turned off.
   */
  Builder(FST::INPUT_TYPE inputType, std::shared_ptr<Outputs<T>> outputs)
      : Builder(inputType, 0, 0, true, true, std::numeric_limits<int>::max(),
                outputs, true, 15)
  {
  }

  /**
   * Instantiates an FST/FSA builder with all the possible tuning and
   * construction tweaks. Read parameter documentation carefully.
   *
   * @param inputType
   *    The input type (transition labels). Can be anything from {@link
   * INPUT_TYPE} enumeration. Shorter types will consume less memory. Strings
   * (character sequences) are represented as {@link INPUT_TYPE#BYTE4} (full
   * unicode codepoints).
   *
   * @param minSuffixCount1
   *    If pruning the input graph during construction, this threshold is used
   * for telling if a node is kept or pruned. If transition_count(node) &gt;=
   * minSuffixCount1, the node is kept.
   *
   * @param minSuffixCount2
   *    (Note: only Mike McCandless knows what this one is really doing...)
   *
   * @param doShareSuffix
   *    If <code>true</code>, the shared suffixes will be compacted into unique
   * paths. This requires an additional RAM-intensive hash map_obj for lookups in
   * memory. Setting this parameter to <code>false</code> creates a single
   * suffix path for all input sequences. This will result in a larger FST, but
   * requires substantially less memory and CPU during building.
   *
   * @param doShareNonSingletonNodes
   *    Only used if doShareSuffix is true.  Set this to
   *    true to ensure FST is fully minimal, at cost of more
   *    CPU and more RAM during building.
   *
   * @param shareMaxTailLength
   *    Only used if doShareSuffix is true.  Set this to
   *    Integer.MAX_VALUE to ensure FST is fully minimal, at cost of more
   *    CPU and more RAM during building.
   *
   * @param outputs The output type for each input sequence. Applies only if
   * building an FST. For FSA, use {@link NoOutputs#getSingleton()} and {@link
   * NoOutputs#getNoOutput()} as the singleton output object.
   *
   * @param allowArrayArcs Pass false to disable the array arc optimization
   *    while building the FST; this will make the resulting
   *    FST smaller but slower to traverse.
   *
   * @param bytesPageBits How many bits wide to make each
   *    byte[] block in the BytesStore; if you know the FST
   *    will be large then make this larger.  For example 15
   *    bits = 32768 byte pages.
   */
  Builder(FST::INPUT_TYPE inputType, int minSuffixCount1, int minSuffixCount2,
          bool doShareSuffix, bool doShareNonSingletonNodes,
          int shareMaxTailLength, std::shared_ptr<Outputs<T>> outputs,
          bool allowArrayArcs, int bytesPageBits)
      : fst(std::make_shared<FST<T>>(inputType, outputs, bytesPageBits)),
        NO_OUTPUT(outputs->getNoOutput()), minSuffixCount1(minSuffixCount1),
        minSuffixCount2(minSuffixCount2),
        doShareNonSingletonNodes(doShareNonSingletonNodes),
        shareMaxTailLength(shareMaxTailLength)
  {
    this->allowArrayArcs = allowArrayArcs;
    bytes = fst->bytes;
    assert(bytes != nullptr);
    if (doShareSuffix) {
      dedupHash =
          std::make_shared<NodeHash<T>>(fst, bytes->getReverseReader(false));
    } else {
      dedupHash.reset();
    }

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
    // UnCompiledNode<T>[] f = (UnCompiledNode<T>[]) new UnCompiledNode[10];
    std::deque<UnCompiledNode<T>> f =
        static_cast<std::deque<UnCompiledNode<T>>>(
            std::deque<std::shared_ptr<UnCompiledNode>>(10));
    frontier = f;
    for (int idx = 0; idx < frontier.size(); idx++) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      frontier[idx] =
          std::make_shared<UnCompiledNode<T>>(shared_from_this(), idx);
    }
  }

  virtual int64_t getTermCount() { return frontier[0]->inputCount; }

  virtual int64_t getNodeCount()
  {
    // 1+ in order to count the -1 implicit final node
    return 1 + nodeCount;
  }

  virtual int64_t getArcCount() { return arcCount; }

  virtual int64_t getMappedStateCount()
  {
    return dedupHash == nullptr ? 0 : nodeCount;
  }

private:
  std::shared_ptr<CompiledNode>
  compileNode(std::shared_ptr<UnCompiledNode<T>> nodeIn,
              int tailLength) 
  {
    constexpr int64_t node;
    int64_t bytesPosStart = bytes->getPosition();
    if (dedupHash != nullptr &&
        (doShareNonSingletonNodes || nodeIn->numArcs <= 1) &&
        tailLength <= shareMaxTailLength) {
      if (nodeIn->numArcs == 0) {
        node = fst->addNode(shared_from_this(), nodeIn);
        lastFrozenNode = node;
      } else {
        node = dedupHash->add(shared_from_this(), nodeIn);
      }
    } else {
      node = fst->addNode(shared_from_this(), nodeIn);
    }
    assert(node != -2);

    int64_t bytesPosEnd = bytes->getPosition();
    if (bytesPosEnd != bytesPosStart) {
      // The FST added a new node:
      assert(bytesPosEnd > bytesPosStart);
      lastFrozenNode = node;
    }

    nodeIn->clear();

    std::shared_ptr<CompiledNode> *const fn = std::make_shared<CompiledNode>();
    fn->node = node;
    return fn;
  }

  void freezeTail(int prefixLenPlus1) 
  {
    // System.out.println("  compileTail " + prefixLenPlus1);
    constexpr int downTo = std::max(1, prefixLenPlus1);
    for (int idx = lastInput->length(); idx >= downTo; idx--) {

      bool doPrune = false;
      bool doCompile = false;

      std::shared_ptr<UnCompiledNode<T>> *const node = frontier[idx];
      std::shared_ptr<UnCompiledNode<T>> *const parent = frontier[idx - 1];

      if (node->inputCount < minSuffixCount1) {
        doPrune = true;
        doCompile = true;
      } else if (idx > prefixLenPlus1) {
        // prune if parent's inputCount is less than suffixMinCount2
        if (parent->inputCount < minSuffixCount2 ||
            (minSuffixCount2 == 1 && parent->inputCount == 1 && idx > 1)) {
          // my parent, about to be compiled, doesn't make the cut, so
          // I'm definitely pruned

          // if minSuffixCount2 is 1, we keep only up
          // until the 'distinguished edge', ie we keep only the
          // 'divergent' part of the FST. if my parent, about to be
          // compiled, has inputCount 1 then we are already past the
          // distinguished edge.  NOTE: this only works if
          // the FST outputs are not "compressible" (simple
          // ords ARE compressible).
          doPrune = true;
        } else {
          // my parent, about to be compiled, does make the cut, so
          // I'm definitely not pruned
          doPrune = false;
        }
        doCompile = true;
      } else {
        // if pruning is disabled (count is 0) we can always
        // compile current node
        doCompile = minSuffixCount2 == 0;
      }

      // System.out.println("    label=" + ((char)
      // lastInput.ints[lastInput.offset+idx-1]) + " idx=" + idx + " inputCount="
      // + frontier[idx].inputCount + " doCompile=" + doCompile + " doPrune=" +
      // doPrune);

      if (node->inputCount < minSuffixCount2 ||
          (minSuffixCount2 == 1 && node->inputCount == 1 && idx > 1)) {
        // drop all arcs
        for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++) {
          // C++ TODO: Most Java annotations will not have direct C++
          // equivalents: ORIGINAL LINE:
          // @SuppressWarnings({"rawtypes","unchecked"}) final UnCompiledNode<T>
          // target = (UnCompiledNode<T>) node.arcs[arcIdx].target;
          std::shared_ptr<UnCompiledNode<T>> *const target =
              std::static_pointer_cast<UnCompiledNode<T>>(
                  node->arcs[arcIdx]->target);
          target->clear();
        }
        node->numArcs = 0;
      }

      if (doPrune) {
        // this node doesn't make it -- deref it
        node->clear();
        parent->deleteLast(lastInput->intAt(idx - 1), node);
      } else {

        if (minSuffixCount2 != 0) {
          compileAllTargets(node, lastInput->length() - idx);
        }
        constexpr T nextFinalOutput = node->output;

        // We "fake" the node as being final if it has no
        // outgoing arcs; in theory we could leave it
        // as non-final (the FST can represent this), but
        // FSTEnum, Util, etc., have trouble w/ non-final
        // dead-end states:
        constexpr bool isFinal = node->isFinal || node->numArcs == 0;

        if (doCompile) {
          // this node makes it and we now compile it.  first,
          // compile any targets that were previously
          // undecided:
          parent->replaceLast(lastInput->intAt(idx - 1),
                              compileNode(node, 1 + lastInput->length() - idx),
                              nextFinalOutput, isFinal);
        } else {
          // replaceLast just to install
          // nextFinalOutput/isFinal onto the arc
          parent->replaceLast(lastInput->intAt(idx - 1), node, nextFinalOutput,
                              isFinal);
          // this node will stay in play for now, since we are
          // undecided on whether to prune it.  later, it
          // will be either compiled or pruned, so we must
          // allocate a new node:
          frontier[idx] =
              std::make_shared<UnCompiledNode<T>>(shared_from_this(), idx);
        }
      }
    }
  }

  // for debugging
  /*
  private std::wstring toString(BytesRef b) {
    try {
      return b.utf8ToString() + " " + b;
    } catch (Throwable t) {
      return b.toString();
    }
  }
  */

  /** Add the next input/output pair.  The provided input
   *  must be sorted after the previous one according to
   *  {@link IntsRef#compareTo}.  It's also OK to add the same
   *  input twice in a row with different outputs, as long
   *  as {@link Outputs} implements the {@link Outputs#merge}
   *  method. Note that input is fully consumed after this
   *  method is returned (so caller is free to reuse), but
   *  output is not.  So if your outputs are changeable (eg
   *  {@link ByteSequenceOutputs} or {@link
   *  IntSequenceOutputs}) then you cannot reuse across
   *  calls. */
public:
  virtual void add(std::shared_ptr<IntsRef> input, T output) 
  {
    /*
    if (DEBUG) {
      BytesRef b = new BytesRef(input.length);
      for(int x=0;x<input.length;x++) {
        b.bytes[x] = (byte) input.ints[x];
      }
      b.length = input.length;
      if (output == NO_OUTPUT) {
        System.out.println("\nFST ADD: input=" + toString(b) + " " + b);
      } else {
        System.out.println("\nFST ADD: input=" + toString(b) + " " + b + "
    output=" + fst.outputs.outputToString(output));
      }
    }
    */

    // De-dup NO_OUTPUT since it must be a singleton:
    if (output.equals(NO_OUTPUT)) {
      output = NO_OUTPUT;
    }

    assert((lastInput->length() == 0 || input->compareTo(lastInput->get()) >= 0,
            L"inputs are added out of order lastInput=" + lastInput->get() +
                L" vs input=" + input));
    assert(validOutput(output));

    // System.out.println("\nadd: " + input);
    if (input->length == 0) {
      // empty input: only allowed as first input.  we have
      // to special case this because the packed FST
      // format cannot represent the empty input since
      // 'finalness' is stored on the incoming arc, not on
      // the node
      frontier[0]->inputCount++;
      frontier[0]->isFinal = true;
      fst->setEmptyOutput(output);
      return;
    }

    // compare shared prefix length
    int pos1 = 0;
    int pos2 = input->offset;
    constexpr int pos1Stop = std::min(lastInput->length(), input->length);
    while (true) {
      frontier[pos1]->inputCount++;
      // System.out.println("  incr " + pos1 + " ct=" +
      // frontier[pos1].inputCount + " n=" + frontier[pos1]);
      if (pos1 >= pos1Stop || lastInput->intAt(pos1) != input->ints[pos2]) {
        break;
      }
      pos1++;
      pos2++;
    }
    constexpr int prefixLenPlus1 = pos1 + 1;

    if (frontier.size() < input->length + 1) {
      std::deque<UnCompiledNode<T>> next =
          ArrayUtil::grow(frontier, input->length + 1);
      for (int idx = frontier.size(); idx < next.size(); idx++) {
        next[idx] =
            std::make_shared<UnCompiledNode<T>>(shared_from_this(), idx);
      }
      frontier = next;
    }

    // minimize/compile states from previous input's
    // orphan'd suffix
    freezeTail(prefixLenPlus1);

    // init tail states for current input
    for (int idx = prefixLenPlus1; idx <= input->length; idx++) {
      frontier[idx - 1]->addArc(input->ints[input->offset + idx - 1],
                                frontier[idx]);
      frontier[idx]->inputCount++;
    }

    std::shared_ptr<UnCompiledNode<T>> *const lastNode =
        frontier[input->length];
    if (lastInput->length() != input->length ||
        prefixLenPlus1 != input->length + 1) {
      lastNode->isFinal = true;
      lastNode->output = NO_OUTPUT;
    }

    // push conflicting outputs forward, only as far as
    // needed
    for (int idx = 1; idx < prefixLenPlus1; idx++) {
      std::shared_ptr<UnCompiledNode<T>> *const node = frontier[idx];
      std::shared_ptr<UnCompiledNode<T>> *const parentNode = frontier[idx - 1];

      constexpr T lastOutput =
          parentNode->getLastOutput(input->ints[input->offset + idx - 1]);
      assert(validOutput(lastOutput));

      constexpr T commonOutputPrefix;
      constexpr T wordSuffix;

      if (lastOutput != NO_OUTPUT) {
        commonOutputPrefix = fst->outputs.common(output, lastOutput);
        assert(validOutput(commonOutputPrefix));
        wordSuffix = fst->outputs.subtract(lastOutput, commonOutputPrefix);
        assert(validOutput(wordSuffix));
        parentNode->setLastOutput(input->ints[input->offset + idx - 1],
                                  commonOutputPrefix);
        node->prependOutput(wordSuffix);
      } else {
        commonOutputPrefix = wordSuffix = NO_OUTPUT;
      }

      output = fst->outputs.subtract(output, commonOutputPrefix);
      assert(validOutput(output));
    }

    if (lastInput->length() == input->length &&
        prefixLenPlus1 == 1 + input->length) {
      // same input more than 1 time in a row, mapping to
      // multiple outputs
      lastNode->output = fst->outputs.merge(lastNode->output, output);
    } else {
      // this new arc is private to this new input; set its
      // arc output to the leftover output:
      frontier[prefixLenPlus1 - 1]->setLastOutput(
          input->ints[input->offset + prefixLenPlus1 - 1], output);
    }

    // save last input
    lastInput->copyInts(input);

    // System.out.println("  count[0]=" + frontier[0].inputCount);
  }

private:
  bool validOutput(T output)
  {
    return output == NO_OUTPUT || !output.equals(NO_OUTPUT);
  }

  /** Returns final FST.  NOTE: this will return null if
   *  nothing is accepted by the FST. */
public:
  virtual std::shared_ptr<FST<T>> finish() 
  {

    std::shared_ptr<UnCompiledNode<T>> *const root = frontier[0];

    // minimize nodes in the last word's suffix
    freezeTail(0);
    if (root->inputCount < minSuffixCount1 ||
        root->inputCount < minSuffixCount2 || root->numArcs == 0) {
      if (fst->emptyOutput == nullptr) {
        return nullptr;
      } else if (minSuffixCount1 > 0 || minSuffixCount2 > 0) {
        // empty string got pruned
        return nullptr;
      }
    } else {
      if (minSuffixCount2 != 0) {
        compileAllTargets(root, lastInput->length());
      }
    }
    // if (DEBUG) System.out.println("  builder.finish root.isFinal=" +
    // root.isFinal + " root.output=" + root.output);
    fst->finish(compileNode(root, lastInput->length())->node);

    return fst;
  }

private:
  void compileAllTargets(std::shared_ptr<UnCompiledNode<T>> node,
                         int tailLength) 
  {
    for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++) {
      std::shared_ptr<Arc<T>> *const arc = node->arcs[arcIdx];
      if (!arc->target->isCompiled()) {
        // not yet compiled
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) final
        // UnCompiledNode<T> n = (UnCompiledNode<T>) arc.target;
        std::shared_ptr<UnCompiledNode<T>> *const n =
            std::static_pointer_cast<UnCompiledNode<T>>(arc->target);
        if (n->numArcs == 0) {
          // System.out.println("seg=" + segment + "        FORCE final arc=" +
          // (char) arc.label);
          arc->isFinal = n->isFinal = true;
        }
        arc->target = compileNode(n, tailLength - 1);
      }
    }
  }

  /** Expert: holds a pending (seen but not yet serialized) arc. */
public:
  template <typename T>
  class Arc : public std::enable_shared_from_this<Arc>
  {
    GET_CLASS_NAME(Arc)
  public:
    int label = 0; // really an "unsigned" byte
    std::shared_ptr<Node> target;
    bool isFinal = false;
    T output;
    T nextFinalOutput;
  };

  // NOTE: not many instances of Node or CompiledNode are in
  // memory while the FST is being built; it's only the
  // current "frontier":

public:
  class Node
  {
    GET_CLASS_NAME(Node)
  public:
    virtual bool isCompiled() = 0;
  };

public:
  virtual int64_t fstRamBytesUsed() { return fst->ramBytesUsed(); }

public:
  class CompiledNode final : public std::enable_shared_from_this<CompiledNode>,
                             public Node
  {
    GET_CLASS_NAME(CompiledNode)
  public:
    int64_t node = 0;
    bool isCompiled() override;
  };

  /** Expert: holds a pending (seen but not yet serialized) Node. */
public:
  template <typename T>
  class UnCompiledNode final
      : public std::enable_shared_from_this<UnCompiledNode>,
        public Node
  {
    GET_CLASS_NAME(UnCompiledNode)
  public:
    const std::shared_ptr<Builder<T>> owner;
    int numArcs = 0;
    std::deque<Arc<T>> arcs;
    // TODO: instead of recording isFinal/output on the
    // node, maybe we should use -1 arc to mean "end" (like
    // we do when reading the FST).  Would simplify much
    // code here...
    T output;
    bool isFinal = false;
    int64_t inputCount = 0;

    /** This node's depth, starting from the automaton root. */
    const int depth;

    /**
     * @param depth
     *          The node's depth starting from the automaton root. Needed for
     *          LUCENE-2934 (node expansion based on conditions other than the
     *          fanout size).
     */
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public
    // UnCompiledNode(Builder<T> owner, int depth)
    UnCompiledNode(std::shared_ptr<Builder<T>> owner, int depth)
        : owner(owner), depth(depth)
    {
      arcs = static_cast<std::deque<Arc<T>>>(
          std::deque<std::shared_ptr<Arc>>(1));
      arcs[0] = std::make_shared<Arc<T>>();
      output = owner->NO_OUTPUT;
    }

    bool isCompiled() override { return false; }

    void clear()
    {
      numArcs = 0;
      isFinal = false;
      output = owner->NO_OUTPUT;
      inputCount = 0;

      // We don't clear the depth here because it never changes
      // for nodes on the frontier (even when reused).
    }

    T getLastOutput(int labelToMatch)
    {
      assert(numArcs > 0);
      assert(arcs[numArcs - 1]->label == labelToMatch);
      return arcs[numArcs - 1]->output;
    }

    void addArc(int label, std::shared_ptr<Node> target)
    {
      assert(label >= 0);
      assert((numArcs == 0 || label > arcs[numArcs - 1]->label,
              L"arc[-1].label=" + std::to_wstring(arcs[numArcs - 1]->label) +
                  L" new label=" + std::to_wstring(label) + L" numArcs=" +
                  std::to_wstring(numArcs)));
      if (numArcs == arcs.size()) {
        std::deque<Arc<T>> newArcs = ArrayUtil::grow(arcs, numArcs + 1);
        for (int arcIdx = numArcs; arcIdx < newArcs.size(); arcIdx++) {
          newArcs[arcIdx] = std::make_shared<Arc<T>>();
        }
        arcs = newArcs;
      }
      std::shared_ptr<Arc<T>> *const arc = arcs[numArcs++];
      arc->label = label;
      arc->target = target;
      arc->output = arc->nextFinalOutput = owner->NO_OUTPUT;
      arc->isFinal = false;
    }

    void replaceLast(int labelToMatch, std::shared_ptr<Node> target,
                     T nextFinalOutput, bool isFinal)
    {
      assert(numArcs > 0);
      std::shared_ptr<Arc<T>> *const arc = arcs[numArcs - 1];
      assert((arc->label == labelToMatch,
              L"arc.label=" + std::to_wstring(arc->label) + L" vs " +
                  std::to_wstring(labelToMatch)));
      arc->target = target;
      // assert target.node != -2;
      arc->nextFinalOutput = nextFinalOutput;
      arc->isFinal = isFinal;
    }

    void deleteLast(int label, std::shared_ptr<Node> target)
    {
      assert(numArcs > 0);
      assert(label == arcs[numArcs - 1]->label);
      assert(target == arcs[numArcs - 1]->target);
      numArcs--;
    }

    void setLastOutput(int labelToMatch, T newOutput)
    {
      assert(owner->validOutput(newOutput));
      assert(numArcs > 0);
      std::shared_ptr<Arc<T>> *const arc = arcs[numArcs - 1];
      assert(arc->label == labelToMatch);
      arc->output = newOutput;
    }

    // pushes an output prefix forward onto all arcs
    void prependOutput(T outputPrefix)
    {
      assert(owner->validOutput(outputPrefix));

      for (int arcIdx = 0; arcIdx < numArcs; arcIdx++) {
        arcs[arcIdx]->output =
            owner->fst->outputs->add(outputPrefix, arcs[arcIdx]->output);
        assert(owner->validOutput(arcs[arcIdx]->output));
      }

      if (isFinal) {
        output = owner->fst->outputs->add(outputPrefix, output);
        assert(owner->validOutput(output));
      }
    }
  };
};

} // namespace org::apache::lucene::util::fst
