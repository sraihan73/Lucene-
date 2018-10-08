#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

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

using PagedGrowableWriter =
    org::apache::lucene::util::packed::PagedGrowableWriter;

// Used to dedup states (lookup already-frozen states)
template <typename T>
class NodeHash final : public std::enable_shared_from_this<NodeHash>
{
  GET_CLASS_NAME(NodeHash)

private:
  std::shared_ptr<PagedGrowableWriter> table;
  int64_t count = 0;
  int64_t mask = 0;
  const std::shared_ptr<FST<T>> fst;
  const std::shared_ptr<FST::Arc<T>> scratchArc =
      std::make_shared<FST::Arc<T>>();
  const std::shared_ptr<FST::BytesReader> in_;

public:
  NodeHash(std::shared_ptr<FST<T>> fst, std::shared_ptr<FST::BytesReader> in_)
      : fst(fst), in_(in_)
  {
    table = std::make_shared<PagedGrowableWriter>(16, 1 << 27, 8,
                                                  PackedInts::COMPACT);
    mask = 15;
  }

private:
  bool nodesEqual(std::shared_ptr<Builder::UnCompiledNode<T>> node,
                  int64_t address) 
  {
    fst->readFirstRealTargetArc(address, scratchArc, in_);
    if (scratchArc->bytesPerArc != 0 && node->numArcs != scratchArc->numArcs) {
      return false;
    }
    for (int arcUpto = 0; arcUpto < node->numArcs; arcUpto++) {
      std::shared_ptr<Builder::Arc<T>> *const arc = node->arcs[arcUpto];
      if (arc->label != scratchArc->label ||
          !arc->output.equals(scratchArc->output) ||
          (std::static_pointer_cast<Builder::CompiledNode>(arc->target))
                  ->node != scratchArc->target ||
          !arc->nextFinalOutput.equals(scratchArc->nextFinalOutput) ||
          arc->isFinal != scratchArc->isFinal()) {
        return false;
      }

      if (scratchArc->isLast()) {
        if (arcUpto == node->numArcs - 1) {
          return true;
        } else {
          return false;
        }
      }
      fst->readNextRealArc(scratchArc, in_);
    }

    return false;
  }

  // hash code for an unfrozen node.  This must be identical
  // to the frozen case (below)!!
  int64_t hash(std::shared_ptr<Builder::UnCompiledNode<T>> node)
  {
    constexpr int PRIME = 31;
    // System.out.println("hash unfrozen");
    int64_t h = 0;
    // TODO: maybe if number of arcs is high we can safely subsample?
    for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++) {
      std::shared_ptr<Builder::Arc<T>> *const arc = node->arcs[arcIdx];
      // System.out.println("  label=" + arc.label + " target=" +
      // ((Builder.CompiledNode) arc.target).node + " h=" + h + " output=" +
      // fst.outputs.outputToString(arc.output) + " isFinal?=" + arc.isFinal);
      h = PRIME * h + arc->label;
      int64_t n =
          (std::static_pointer_cast<Builder::CompiledNode>(arc->target))->node;
      h = PRIME * h + static_cast<int>(n ^ (n >> 32));
      h = PRIME * h + arc->output.hashCode();
      h = PRIME * h + arc->nextFinalOutput.hashCode();
      if (arc->isFinal) {
        h += 17;
      }
    }
    // System.out.println("  ret " + (h&Integer.MAX_VALUE));
    return h & std::numeric_limits<int64_t>::max();
  }

  // hash code for a frozen node
  int64_t hash(int64_t node) 
  {
    constexpr int PRIME = 31;
    // System.out.println("hash frozen node=" + node);
    int64_t h = 0;
    fst->readFirstRealTargetArc(node, scratchArc, in_);
    while (true) {
      // System.out.println("  label=" + scratchArc.label + " target=" +
      // scratchArc.target + " h=" + h + " output=" +
      // fst.outputs.outputToString(scratchArc.output) + " next?=" +
      // scratchArc.flag(4) + " final?=" + scratchArc.isFinal() + " pos=" +
      // in.getPosition());
      h = PRIME * h + scratchArc->label;
      h = PRIME * h +
          static_cast<int>(scratchArc->target ^ (scratchArc->target >> 32));
      h = PRIME * h + scratchArc->output.hashCode();
      h = PRIME * h + scratchArc->nextFinalOutput.hashCode();
      if (scratchArc->isFinal()) {
        h += 17;
      }
      if (scratchArc->isLast()) {
        break;
      }
      fst->readNextRealArc(scratchArc, in_);
    }
    // System.out.println("  ret " + (h&Integer.MAX_VALUE));
    return h & std::numeric_limits<int64_t>::max();
  }

public:
  int64_t
  add(std::shared_ptr<Builder<T>> builder,
      std::shared_ptr<Builder::UnCompiledNode<T>> nodeIn) 
  {
    // System.out.println("hash: add count=" + count + " vs " + table.size() + "
    // mask=" + mask);
    constexpr int64_t h = hash(nodeIn);
    int64_t pos = h & mask;
    int c = 0;
    while (true) {
      constexpr int64_t v = table->get(pos);
      if (v == 0) {
        // freeze & add
        constexpr int64_t node = fst->addNode(builder, nodeIn);
        // System.out.println("  now freeze node=" + node);
        assert((hash(node) == h, L"frozenHash=" + std::to_wstring(hash(node)) +
                                     L" vs h=" + std::to_wstring(h)));
        count++;
        table->set(pos, node);
        // Rehash at 2/3 occupancy:
        if (count > 2 * table->size() / 3) {
          rehash();
        }
        return node;
      } else if (nodesEqual(nodeIn, v)) {
        // same node is already here
        return v;
      }

      // quadratic probe
      pos = (pos + (++c)) & mask;
    }
  }

  // called only by rehash
private:
  void addNew(int64_t address) 
  {
    int64_t pos = hash(address) & mask;
    int c = 0;
    while (true) {
      if (table->get(pos) == 0) {
        table->set(pos, address);
        break;
      }

      // quadratic probe
      pos = (pos + (++c)) & mask;
    }
  }

  void rehash() 
  {
    std::shared_ptr<PagedGrowableWriter> *const oldTable = table;

    table = std::make_shared<PagedGrowableWriter>(
        2 * oldTable->size(), 1 << 30, PackedInts::bitsRequired(count),
        PackedInts::COMPACT);
    mask = table->size() - 1;
    for (int64_t idx = 0; idx < oldTable->size(); idx++) {
      constexpr int64_t address = oldTable->get(idx);
      if (address != 0) {
        addNew(address);
      }
    }
  }
};

} // namespace org::apache::lucene::util::fst
