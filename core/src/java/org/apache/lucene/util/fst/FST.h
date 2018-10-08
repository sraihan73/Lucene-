#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}

#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using Constants = org::apache::lucene::util::Constants;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

// TODO: break this into WritableFST and ReadOnlyFST.. then
// we can have subclasses of ReadOnlyFST to handle the
// different byte[] level encodings (packed or
// not)... and things like nodeCount, arcCount are read only

// TODO: if FST is pure prefix trie we can do a more compact
// job, ie, once we are at a 'suffix only', just store the
// completion labels as a string not as a series of arcs.

// NOTE: while the FST is able to represent a non-final
// dead-end state (NON_FINAL_END_NODE=0), the layers above
// (FSTEnum, Util) have problems with this!!

/** Represents an finite state machine (FST), using a
 *  compact byte[] format.
 *  <p> The format is similar to what's used by Morfologik
 *  (http://sourceforge.net/projects/morfologik).
 *
 *  <p> See the {@link org.apache.lucene.util.fst package
 *      documentation} for some simple examples.
 *
 * @lucene.experimental
 */
template <typename T>
class FST final : public std::enable_shared_from_this<FST>, public Accountable
{
  GET_CLASS_NAME(FST)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(FST::typeid);
  static const int64_t ARC_SHALLOW_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(Arc::typeid);

  /** Specifies allowed range of each int input label for
   *  this FST. */
public:
  enum class INPUT_TYPE { GET_CLASS_NAME(INPUT_TYPE) BYTE1, BYTE2, BYTE4 };

public:
  static const int BIT_FINAL_ARC = 1 << 0;
  static const int BIT_LAST_ARC = 1 << 1;
  static const int BIT_TARGET_NEXT = 1 << 2;

  // TODO: we can free up a bit if we can nuke this:
  static const int BIT_STOP_NODE = 1 << 3;

  /** This flag is set if the arc has an output. */
  static const int BIT_ARC_HAS_OUTPUT = 1 << 4;

  static const int BIT_ARC_HAS_FINAL_OUTPUT = 1 << 5;

  // We use this as a marker (because this one flag is
  // illegal by itself ...):
private:
  static const char ARCS_AS_FIXED_ARRAY = BIT_ARC_HAS_FINAL_OUTPUT;

  /**
   * @see #shouldExpand(Builder, Builder.UnCompiledNode)
   */
public:
  static constexpr int FIXED_ARRAY_SHALLOW_DISTANCE = 3; // 0 => only root node.

  /**
   * @see #shouldExpand(Builder, Builder.UnCompiledNode)
   */
  static constexpr int FIXED_ARRAY_NUM_ARCS_SHALLOW = 5;

  /**
   * @see #shouldExpand(Builder, Builder.UnCompiledNode)
   */
  static constexpr int FIXED_ARRAY_NUM_ARCS_DEEP = 10;

  // Increment version to change it
private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring FILE_FORMAT_NAME = L"FST";
  static constexpr int VERSION_START = 0;

  /** Changed numBytesPerArc for array'd case from byte to int. */
  static constexpr int VERSION_INT_NUM_BYTES_PER_ARC = 1;

  /** Write BYTE2 labels as 2-byte short, not vInt. */
  static constexpr int VERSION_SHORT_BYTE2_LABELS = 2;

  /** Added optional packed format. */
  static constexpr int VERSION_PACKED = 3;

  /** Changed from int to vInt for encoding arc targets.
   *  Also changed maxBytesPerArc from int to vInt in the array case. */
  static constexpr int VERSION_VINT_TARGET = 4;

  /** Don't store arcWithOutputCount anymore */
  static constexpr int VERSION_NO_NODE_ARC_COUNTS = 5;

  static constexpr int VERSION_PACKED_REMOVED = 6;

  static constexpr int VERSION_CURRENT = VERSION_PACKED_REMOVED;

  // Never serialized; just used to represent the virtual
  // final node w/ no arcs:
  static constexpr int64_t FINAL_END_NODE = -1;

  // Never serialized; just used to represent the virtual
  // non-final node w/ no arcs:
  static constexpr int64_t NON_FINAL_END_NODE = 0;

  /** If arc has this label then that arc is final/accepted */
public:
  static constexpr int END_LABEL = -1;

  const INPUT_TYPE inputType;

  // if non-null, this FST accepts the empty string and
  // produces this output
  T emptyOutput;

  /** A {@link BytesStore}, used during building, or during reading when
   *  the FST is very large (more than 1 GB).  If the FST is less than 1
   *  GB then bytesArray is set instead. */
  const std::shared_ptr<BytesStore> bytes;

  /** Used at read time when the FST fits into a single byte[]. */
  std::deque<char> const bytesArray;

private:
  int64_t startNode = -1;

public:
  const std::shared_ptr<Outputs<T>> outputs;

private:
  std::deque<Arc<T>> cachedRootArcs;

  /** Represents a single arc. */
public:
  template <typename T>
  class Arc final : public std::enable_shared_from_this<Arc>
  {
    GET_CLASS_NAME(Arc)
  public:
    int label = 0;
    T output;

    /** To node (ord or address) */
    int64_t target = 0;

    char flags = 0;
    T nextFinalOutput;

    // address (into the byte[]), or ord/address if label == END_LABEL
    int64_t nextArc = 0;

    /** Where the first arc in the array starts; only valid if
     *  bytesPerArc != 0 */
    int64_t posArcsStart = 0;

    /** Non-zero if this arc is part of an array, which means all
     *  arcs for the node are encoded with a fixed number of bytes so
     *  that we can random access by index.  We do when there are enough
     *  arcs leaving one node.  It wastes some bytes but gives faster
     *  lookups. */
    int bytesPerArc = 0;

    /** Where we are in the array; only valid if bytesPerArc != 0. */
    int arcIdx = 0;

    /** How many arcs in the array; only valid if bytesPerArc != 0. */
    int numArcs = 0;

    /** Returns this */
    std::shared_ptr<Arc<T>> copyFrom(std::shared_ptr<Arc<T>> other)
    {
      label = other->label;
      target = other->target;
      flags = other->flags;
      output = other->output;
      nextFinalOutput = other->nextFinalOutput;
      nextArc = other->nextArc;
      bytesPerArc = other->bytesPerArc;
      if (bytesPerArc != 0) {
        posArcsStart = other->posArcsStart;
        arcIdx = other->arcIdx;
        numArcs = other->numArcs;
      }
      return shared_from_this();
    }

    bool flag(int flag) { return FST::flag(flags, flag); }

    bool isLast() { return flag(BIT_LAST_ARC); }

    bool isFinal() { return flag(BIT_FINAL_ARC); }

    std::wstring toString() override
    {
      std::shared_ptr<StringBuilder> b = std::make_shared<StringBuilder>();
      b->append(L" target=" + std::to_wstring(target));
      b->append(L" label=0x" + Integer::toHexString(label));
      if (flag(BIT_FINAL_ARC)) {
        b->append(L" final");
      }
      if (flag(BIT_LAST_ARC)) {
        b->append(L" last");
      }
      if (flag(BIT_TARGET_NEXT)) {
        b->append(L" targetNext");
      }
      if (flag(BIT_STOP_NODE)) {
        b->append(L" stop");
      }
      if (flag(BIT_ARC_HAS_OUTPUT)) {
        b->append(L" output=" + output);
      }
      if (flag(BIT_ARC_HAS_FINAL_OUTPUT)) {
        b->append(L" nextFinalOutput=" + nextFinalOutput);
      }
      if (bytesPerArc != 0) {
        b->append(L" arcArray(idx=" + std::to_wstring(arcIdx) + L" of " +
                  std::to_wstring(numArcs) + L")");
      }
      return b->toString();
    }
  };

private:
  static bool flag(int flags, int bit) { return (flags & bit) != 0; }

  const int version;

  // make a new empty FST, for building; Builder invokes
  // this ctor
public:
  FST(INPUT_TYPE inputType, std::shared_ptr<Outputs<T>> outputs,
      int bytesPageBits)
      : inputType(inputType),
        bytes(std::make_shared<BytesStore>(bytesPageBits)),
        bytesArray(bytesArray.clear()), outputs(outputs),
        version(VERSION_CURRENT)
  {
    // pad: ensure no node gets address 0 which is reserved to mean
    // the stop state w/ no arcs
    bytes->writeByte(static_cast<char>(0));

    emptyOutput = nullptr;
  }

  static const int DEFAULT_MAX_BLOCK_BITS = Constants::JRE_IS_64BIT ? 30 : 28;

  /** Load a previously saved FST. */
  FST(std::shared_ptr<DataInput> in_, std::shared_ptr<Outputs<T>> outputs)
   : FST(in_, outputs, DEFAULT_MAX_BLOCK_BITS)
  {
  }

  /** Load a previously saved FST; maxBlockBits allows you to
   *  control the size of the byte[] pages used to hold the FST bytes. */
  FST(std::shared_ptr<DataInput> in_, std::shared_ptr<Outputs<T>> outputs,
      int maxBlockBits)
  
      : outputs(outputs),
        version(CodecUtil::checkHeader(in_, FILE_FORMAT_NAME, VERSION_PACKED,
                                       VERSION_CURRENT))
  {

    if (maxBlockBits < 1 || maxBlockBits > 30) {
      throw std::invalid_argument("maxBlockBits should be 1 .. 30; got " +
                                  std::to_wstring(maxBlockBits));
    }

    // NOTE: only reads most recent format; we don't have
    // back-compat promise for FSTs (they are experimental):
    if (version < VERSION_PACKED_REMOVED) {
      if (in_->readByte() == 1) {
        throw std::make_shared<CorruptIndexException>(
            L"Cannot read packed FSTs anymore", in_);
      }
    }
    if (in_->readByte() == 1) {
      // accepts empty string
      // 1 KB blocks:
      std::shared_ptr<BytesStore> emptyBytes = std::make_shared<BytesStore>(10);
      int numBytes = in_->readVInt();
      emptyBytes->copyBytes(in_, numBytes);

      // De-serialize empty-string output:
      std::shared_ptr<BytesReader> reader = emptyBytes->getReverseReader();
      // NoOutputs uses 0 bytes when writing its output,
      // so we have to check here else BytesStore gets
      // angry:
      if (numBytes > 0) {
        reader->setPosition(numBytes - 1);
      }
      emptyOutput = outputs->readFinalOutput(reader);
    } else {
      emptyOutput = nullptr;
    }
    constexpr char t = in_->readByte();
    switch (t) {
    case 0:
      inputType = INPUT_TYPE::BYTE1;
      break;
    case 1:
      inputType = INPUT_TYPE::BYTE2;
      break;
    case 2:
      inputType = INPUT_TYPE::BYTE4;
      break;
    default:
      throw std::make_shared<IllegalStateException>(L"invalid input type " +
                                                    std::to_wstring(t));
    }
    startNode = in_->readVLong();
    if (version < VERSION_NO_NODE_ARC_COUNTS) {
      in_->readVLong();
      in_->readVLong();
      in_->readVLong();
    }

    int64_t numBytes = in_->readVLong();
    if (numBytes > 1 << maxBlockBits) {
      // FST is big: we need multiple pages
      bytes = std::make_shared<BytesStore>(in_, numBytes, 1 << maxBlockBits);
      bytesArray.clear();
    } else {
      // FST fits into a single block: use ByteArrayBytesStoreReader for less
      // overhead
      bytes.reset();
      bytesArray = std::deque<char>(static_cast<int>(numBytes));
      in_->readBytes(bytesArray, 0, bytesArray.size());
    }

    cacheRootArcs();
  }

  INPUT_TYPE getInputType() { return inputType; }

private:
  int64_t ramBytesUsed(std::deque<Arc<T>> &arcs)
  {
    int64_t size = 0;
    if (arcs.size() > 0) {
      size += RamUsageEstimator::shallowSizeOf(arcs);
      for (auto arc : arcs) {
        if (arc != nullptr) {
          size += ARC_SHALLOW_RAM_BYTES_USED;
          if (arc->output != nullptr && arc->output != outputs->getNoOutput()) {
            size += outputs->ramBytesUsed(arc->output);
          }
          if (arc->nextFinalOutput != nullptr &&
              arc->nextFinalOutput != outputs->getNoOutput()) {
            size += outputs->ramBytesUsed(arc->nextFinalOutput);
          }
        }
      }
    }
    return size;
  }

  int cachedArcsBytesUsed = 0;

public:
  int64_t ramBytesUsed() override
  {
    int64_t size = BASE_RAM_BYTES_USED;
    if (bytesArray.size() > 0) {
      size += bytesArray.size();
    } else {
      size += bytes->ramBytesUsed();
    }
    size += cachedArcsBytesUsed;
    return size;
  }

  std::wstring toString() override
  {
    return getClass().getSimpleName() + L"(input=" + inputType + L",output=" +
           outputs;
  }

  void finish(int64_t newStartNode) 
  {
    assert(newStartNode <= bytes->getPosition());
    if (startNode != -1) {
      throw std::make_shared<IllegalStateException>(L"already finished");
    }
    if (newStartNode == FINAL_END_NODE && emptyOutput != nullptr) {
      newStartNode = 0;
    }
    startNode = newStartNode;
    bytes->finish();
    cacheRootArcs();
  }

  // Optionally caches first 128 labels
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private void
  // cacheRootArcs() throws java.io.IOException
  void cacheRootArcs() 
  {
    // We should only be called once per FST:
    assert(cachedArcsBytesUsed == 0);

    std::shared_ptr<Arc<T>> *const arc = std::make_shared<Arc<T>>();
    getFirstArc(arc);
    if (targetHasArcs(arc)) {
      std::shared_ptr<BytesReader> *const in_ = getBytesReader();
      std::deque<Arc<T>> arcs = static_cast<std::deque<Arc<T>>>(
          std::deque<std::shared_ptr<Arc>>(0x80));
      readFirstRealTargetArc(arc->target, arc, in_);
      int count = 0;
      while (true) {
        assert(arc->label != END_LABEL);
        if (arc->label < arcs.size()) {
          arcs[arc->label] = (std::make_shared<Arc<T>>())->copyFrom(arc);
        } else {
          break;
        }
        if (arc->isLast()) {
          break;
        }
        readNextRealArc(arc, in_);
        count++;
      }

      int cacheRAM = static_cast<int>(ramBytesUsed(arcs));

      // Don't cache if there are only a few arcs or if the cache would use >
      // 20% RAM of the FST itself:
      if (count >= FIXED_ARRAY_NUM_ARCS_SHALLOW &&
          cacheRAM < ramBytesUsed() / 5) {
        cachedRootArcs = arcs;
        cachedArcsBytesUsed = cacheRAM;
      }
    }
  }

public:
  T getEmptyOutput() { return emptyOutput; }

  void setEmptyOutput(T v) 
  {
    if (emptyOutput != nullptr) {
      emptyOutput = outputs->merge(emptyOutput, v);
    } else {
      emptyOutput = v;
    }
  }

  void save(std::shared_ptr<DataOutput> out) 
  {
    if (startNode == -1) {
      throw std::make_shared<IllegalStateException>(L"call finish first");
    }
    CodecUtil::writeHeader(out, FILE_FORMAT_NAME, VERSION_CURRENT);
    // TODO: really we should encode this as an arc, arriving
    // to the root node, instead of special casing here:
    if (emptyOutput != nullptr) {
      // Accepts empty string
      out->writeByte(static_cast<char>(1));

      // Serialize empty-string output:
      std::shared_ptr<RAMOutputStream> ros =
          std::make_shared<RAMOutputStream>();
      outputs->writeFinalOutput(emptyOutput, ros);

      std::deque<char> emptyOutputBytes(
          static_cast<int>(ros->getFilePointer()));
      ros->writeTo(emptyOutputBytes, 0);

      // reverse
      constexpr int stopAt = emptyOutputBytes.size() / 2;
      int upto = 0;
      while (upto < stopAt) {
        constexpr char b = emptyOutputBytes[upto];
        emptyOutputBytes[upto] =
            emptyOutputBytes[emptyOutputBytes.size() - upto - 1];
        emptyOutputBytes[emptyOutputBytes.size() - upto - 1] = b;
        upto++;
      }
      out->writeVInt(emptyOutputBytes.size());
      out->writeBytes(emptyOutputBytes, 0, emptyOutputBytes.size());
    } else {
      out->writeByte(static_cast<char>(0));
    }
    constexpr char t;
    if (inputType == INPUT_TYPE::BYTE1) {
      t = 0;
    } else if (inputType == INPUT_TYPE::BYTE2) {
      t = 1;
    } else {
      t = 2;
    }
    out->writeByte(t);
    out->writeVLong(startNode);
    if (bytes != nullptr) {
      int64_t numBytes = bytes->getPosition();
      out->writeVLong(numBytes);
      bytes->writeTo(out);
    } else {
      assert(bytesArray.size() > 0);
      out->writeVLong(bytesArray.size());
      out->writeBytes(bytesArray, 0, bytesArray.size());
    }
  }

  /**
   * Writes an automaton to a file.
   */
  void save(std::shared_ptr<Path> path) 
  {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.OutputStream os = new
    // java.io.BufferedOutputStream(java.nio.file.Files.newOutputStream(path)))
    {
      java::io::OutputStream os = java::io::BufferedOutputStream(
          java::nio::file::Files::newOutputStream(path));
      save(std::make_shared<OutputStreamDataOutput>(os));
    }
  }

  /**
   * Reads an automaton from a file.
   */
  template <typename T>
  static std::shared_ptr<FST<T>>
  read(std::shared_ptr<Path> path,
       std::shared_ptr<Outputs<T>> outputs) 
  {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream is =
    // java.nio.file.Files.newInputStream(path))
    {
      java::io::InputStream is = java::nio::file::Files::newInputStream(path);
      return std::make_shared<FST<T>>(
          std::make_shared<InputStreamDataInput>(
              std::make_shared<BufferedInputStream>(is)),
          outputs);
    }
  }

private:
  void writeLabel(std::shared_ptr<DataOutput> out, int v) 
  {
    assert((v >= 0, L"v=" + std::to_wstring(v)));
    if (inputType == INPUT_TYPE::BYTE1) {
      assert((v <= 255, L"v=" + std::to_wstring(v)));
      out->writeByte(static_cast<char>(v));
    } else if (inputType == INPUT_TYPE::BYTE2) {
      assert((v <= 65535, L"v=" + std::to_wstring(v)));
      out->writeShort(static_cast<short>(v));
    } else {
      out->writeVInt(v);
    }
  }

  /** Reads one BYTE1/2/4 label from the provided {@link DataInput}. */
public:
  int readLabel(std::shared_ptr<DataInput> in_) 
  {
    constexpr int v;
    if (inputType == INPUT_TYPE::BYTE1) {
      // Unsigned byte:
      v = in_->readByte() & 0xFF;
    } else if (inputType == INPUT_TYPE::BYTE2) {
      // Unsigned short:
      v = in_->readShort() & 0xFFFF;
    } else {
      v = in_->readVInt();
    }
    return v;
  }

  /** returns true if the node at this address has any
   *  outgoing arcs */
  template <typename T>
  static bool targetHasArcs(std::shared_ptr<Arc<T>> arc)
  {
    return arc->target > 0;
  }

  // serializes new node by appending its bytes to the end
  // of the current byte[]
  int64_t
  addNode(std::shared_ptr<Builder<T>> builder,
          std::shared_ptr<Builder::UnCompiledNode<T>> nodeIn) 
  {
    T NO_OUTPUT = outputs->getNoOutput();

    // System.out.println("FST.addNode pos=" + bytes.getPosition() + " numArcs="
    // + nodeIn.numArcs);
    if (nodeIn->numArcs == 0) {
      if (nodeIn->isFinal) {
        return FINAL_END_NODE;
      } else {
        return NON_FINAL_END_NODE;
      }
    }

    constexpr int64_t startAddress = builder->bytes->getPosition();
    // System.out.println("  startAddr=" + startAddress);

    constexpr bool doFixedArray = shouldExpand(builder, nodeIn);
    if (doFixedArray) {
      // System.out.println("  fixedArray");
      if (builder->reusedBytesPerArc.size() < nodeIn->numArcs) {
        builder->reusedBytesPerArc =
            std::deque<int>(ArrayUtil::oversize(nodeIn->numArcs, 1));
      }
    }

    builder->arcCount += nodeIn->numArcs;

    constexpr int lastArc = nodeIn->numArcs - 1;

    int64_t lastArcStart = builder->bytes->getPosition();
    int maxBytesPerArc = 0;
    for (int arcIdx = 0; arcIdx < nodeIn->numArcs; arcIdx++) {
      std::shared_ptr<Builder::Arc<T>> *const arc = nodeIn->arcs[arcIdx];
      std::shared_ptr<Builder::CompiledNode> *const target =
          std::static_pointer_cast<Builder::CompiledNode>(arc->target);
      int flags = 0;
      // System.out.println("  arc " + arcIdx + " label=" + arc.label + " ->
      // target=" + target.node);

      if (arcIdx == lastArc) {
        flags += BIT_LAST_ARC;
      }

      if (builder->lastFrozenNode == target->node && !doFixedArray) {
        // TODO: for better perf (but more RAM used) we
        // could avoid this except when arc is "near" the
        // last arc:
        flags += BIT_TARGET_NEXT;
      }

      if (arc->isFinal) {
        flags += BIT_FINAL_ARC;
        if (arc->nextFinalOutput != NO_OUTPUT) {
          flags += BIT_ARC_HAS_FINAL_OUTPUT;
        }
      } else {
        assert(arc->nextFinalOutput == NO_OUTPUT);
      }

      bool targetHasArcs = target->node > 0;

      if (!targetHasArcs) {
        flags += BIT_STOP_NODE;
      }

      if (arc->output != NO_OUTPUT) {
        flags += BIT_ARC_HAS_OUTPUT;
      }

      builder->bytes->writeByte(static_cast<char>(flags));
      writeLabel(builder->bytes, arc->label);

      // System.out.println("  write arc: label=" + (char) arc.label + " flags="
      // + flags + " target=" + target.node + " pos=" + bytes.getPosition() + "
      // output=" + outputs.outputToString(arc.output));

      if (arc->output != NO_OUTPUT) {
        outputs->write(arc->output, builder->bytes);
        // System.out.println("    write output");
      }

      if (arc->nextFinalOutput != NO_OUTPUT) {
        // System.out.println("    write final output");
        outputs->writeFinalOutput(arc->nextFinalOutput, builder->bytes);
      }

      if (targetHasArcs && (flags & BIT_TARGET_NEXT) == 0) {
        assert(target->node > 0);
        // System.out.println("    write target");
        builder->bytes->writeVLong(target->node);
      }

      // just write the arcs "like normal" on first pass,
      // but record how many bytes each one took, and max
      // byte size:
      if (doFixedArray) {
        builder->reusedBytesPerArc[arcIdx] =
            static_cast<int>(builder->bytes->getPosition() - lastArcStart);
        lastArcStart = builder->bytes->getPosition();
        maxBytesPerArc =
            std::max(maxBytesPerArc, builder->reusedBytesPerArc[arcIdx]);
        // System.out.println("    bytes=" + builder.reusedBytesPerArc[arcIdx]);
      }
    }

    // TODO: try to avoid wasteful cases: disable doFixedArray in that case
    /*
     *
     * LUCENE-4682: what is a fair heuristic here?
     * It could involve some of these:
     * 1. how "busy" the node is: nodeIn.inputCount relative to
    frontier[0].inputCount?
     * 2. how much binSearch saves over scan: nodeIn.numArcs
     * 3. waste: numBytes vs numBytesExpanded
     *
     * the one below just looks at #3
    if (doFixedArray) {
      // rough heuristic: make this 1.25 "waste factor" a parameter to the phd
    ctor???? int numBytes = lastArcStart - startAddress; int numBytesExpanded =
    maxBytesPerArc * nodeIn.numArcs; if (numBytesExpanded > numBytes*1.25) {
        doFixedArray = false;
      }
    }
    */

    if (doFixedArray) {
      constexpr int MAX_HEADER_SIZE =
          11; // header(byte) + numArcs(vint) + numBytes(vint)
      assert(maxBytesPerArc > 0);
      // 2nd pass just "expands" all arcs to take up a fixed
      // byte size

      // System.out.println("write int @pos=" + (fixedArrayStart-4) + "
      // numArcs=" + nodeIn.numArcs);
      // create the header
      // TODO: clean this up: or just rewind+reuse and deal with it
      std::deque<char> header(MAX_HEADER_SIZE);
      std::shared_ptr<ByteArrayDataOutput> bad =
          std::make_shared<ByteArrayDataOutput>(header);
      // write a "false" first arc:
      bad->writeByte(ARCS_AS_FIXED_ARRAY);
      bad->writeVInt(nodeIn->numArcs);
      bad->writeVInt(maxBytesPerArc);
      int headerLen = bad->getPosition();

      constexpr int64_t fixedArrayStart = startAddress + headerLen;

      // expand the arcs in place, backwards
      int64_t srcPos = builder->bytes->getPosition();
      int64_t destPos = fixedArrayStart + nodeIn->numArcs * maxBytesPerArc;
      assert(destPos >= srcPos);
      if (destPos > srcPos) {
        builder->bytes->skipBytes(static_cast<int>(destPos - srcPos));
        for (int arcIdx = nodeIn->numArcs - 1; arcIdx >= 0; arcIdx--) {
          destPos -= maxBytesPerArc;
          srcPos -= builder->reusedBytesPerArc[arcIdx];
          // System.out.println("  repack arcIdx=" + arcIdx + " srcPos=" +
          // srcPos + " destPos=" + destPos);
          if (srcPos != destPos) {
            // System.out.println("  copy len=" +
            // builder.reusedBytesPerArc[arcIdx]);
            assert((destPos > srcPos,
                    L"destPos=" + std::to_wstring(destPos) + L" srcPos=" +
                        std::to_wstring(srcPos) + L" arcIdx=" +
                        std::to_wstring(arcIdx) + L" maxBytesPerArc=" +
                        std::to_wstring(maxBytesPerArc) +
                        L" reusedBytesPerArc[arcIdx]=" +
                        std::to_wstring(builder->reusedBytesPerArc[arcIdx]) +
                        L" nodeIn.numArcs=" +
                        std::to_wstring(nodeIn->numArcs)));
            builder->bytes->copyBytes(srcPos, destPos,
                                      builder->reusedBytesPerArc[arcIdx]);
          }
        }
      }

      // now write the header
      builder->bytes->writeBytes(startAddress, header, 0, headerLen);
    }

    constexpr int64_t thisNodeAddress = builder->bytes->getPosition() - 1;

    builder->bytes->reverse(startAddress, thisNodeAddress);

    builder->nodeCount++;
    return thisNodeAddress;
  }

  /** Fills virtual 'start' arc, ie, an empty incoming arc to
   *  the FST's start node */
  std::shared_ptr<Arc<T>> getFirstArc(std::shared_ptr<Arc<T>> arc)
  {
    T NO_OUTPUT = outputs->getNoOutput();

    if (emptyOutput != nullptr) {
      arc->flags = BIT_FINAL_ARC | BIT_LAST_ARC;
      arc->nextFinalOutput = emptyOutput;
      if (emptyOutput != NO_OUTPUT) {
        arc->flags |= BIT_ARC_HAS_FINAL_OUTPUT;
      }
    } else {
      arc->flags = BIT_LAST_ARC;
      arc->nextFinalOutput = NO_OUTPUT;
    }
    arc->output = NO_OUTPUT;

    // If there are no nodes, ie, the FST only accepts the
    // empty string, then startNode is 0
    arc->target = startNode;
    return arc;
  }

  /** Follows the <code>follow</code> arc and reads the last
   *  arc of its target; this changes the provided
   *  <code>arc</code> (2nd arg) in-place and returns it.
   *
   * @return Returns the second argument
   * (<code>arc</code>). */
  std::shared_ptr<Arc<T>>
  readLastTargetArc(std::shared_ptr<Arc<T>> follow, std::shared_ptr<Arc<T>> arc,
                    std::shared_ptr<BytesReader> in_) 
  {
    // System.out.println("readLast");
    if (!targetHasArcs(follow)) {
      // System.out.println("  end node");
      assert(follow->isFinal());
      arc->label = END_LABEL;
      arc->target = FINAL_END_NODE;
      arc->output = follow->nextFinalOutput;
      arc->flags = BIT_LAST_ARC;
      return arc;
    } else {
      in_->setPosition(follow->target);
      constexpr char b = in_->readByte();
      if (b == ARCS_AS_FIXED_ARRAY) {
        // array: jump straight to end
        arc->numArcs = in_->readVInt();
        if (version >= VERSION_VINT_TARGET) {
          arc->bytesPerArc = in_->readVInt();
        } else {
          arc->bytesPerArc = in_->readInt();
        }
        // System.out.println("  array numArcs=" + arc.numArcs + " bpa=" +
        // arc.bytesPerArc);
        arc->posArcsStart = in_->getPosition();
        arc->arcIdx = arc->numArcs - 2;
      } else {
        arc->flags = b;
        // non-array: linear scan
        arc->bytesPerArc = 0;
        // System.out.println("  scan");
        while (!arc->isLast()) {
          // skip this arc:
          readLabel(in_);
          if (arc->flag(BIT_ARC_HAS_OUTPUT)) {
            outputs->skipOutput(in_);
          }
          if (arc->flag(BIT_ARC_HAS_FINAL_OUTPUT)) {
            outputs->skipFinalOutput(in_);
          }
          if (arc->flag(BIT_STOP_NODE)) {
          } else if (arc->flag(BIT_TARGET_NEXT)) {
          } else {
            readUnpackedNodeTarget(in_);
          }
          arc->flags = in_->readByte();
        }
        // Undo the byte flags we read:
        in_->skipBytes(-1);
        arc->nextArc = in_->getPosition();
      }
      readNextRealArc(arc, in_);
      assert(arc->isLast());
      return arc;
    }
  }

private:
  int64_t
  readUnpackedNodeTarget(std::shared_ptr<BytesReader> in_) 
  {
    int64_t target;
    if (version < VERSION_VINT_TARGET) {
      target = in_->readInt();
    } else {
      target = in_->readVLong();
    }
    return target;
  }

  /**
   * Follow the <code>follow</code> arc and read the first arc of its target;
   * this changes the provided <code>arc</code> (2nd arg) in-place and returns
   * it.
   *
   * @return Returns the second argument (<code>arc</code>).
   */
public:
  std::shared_ptr<Arc<T>>
  readFirstTargetArc(std::shared_ptr<Arc<T>> follow,
                     std::shared_ptr<Arc<T>> arc,
                     std::shared_ptr<BytesReader> in_) 
  {
    // int pos = address;
    // System.out.println("    readFirstTarget follow.target=" + follow.target +
    // " isFinal=" + follow.isFinal());
    if (follow->isFinal()) {
      // Insert "fake" final first arc:
      arc->label = END_LABEL;
      arc->output = follow->nextFinalOutput;
      arc->flags = BIT_FINAL_ARC;
      if (follow->target <= 0) {
        arc->flags |= BIT_LAST_ARC;
      } else {
        // NOTE: nextArc is a node (not an address!) in this case:
        arc->nextArc = follow->target;
      }
      arc->target = FINAL_END_NODE;
      // System.out.println("    insert isFinal; nextArc=" + follow.target + "
      // isLast=" + arc.isLast() + " output=" +
      // outputs.outputToString(arc.output));
      return arc;
    } else {
      return readFirstRealTargetArc(follow->target, arc, in_);
    }
  }

  std::shared_ptr<Arc<T>>
  readFirstRealTargetArc(int64_t node, std::shared_ptr<Arc<T>> arc,
                         std::shared_ptr<BytesReader> in_) 
  {
    constexpr int64_t address = node;
    in_->setPosition(address);
    // System.out.println("  readFirstRealTargtArc address="
    //+ address);
    // System.out.println("   flags=" + arc.flags);

    if (in_->readByte() == ARCS_AS_FIXED_ARRAY) {
      // System.out.println("  fixedArray");
      // this is first arc in a fixed-array
      arc->numArcs = in_->readVInt();
      if (version >= VERSION_VINT_TARGET) {
        arc->bytesPerArc = in_->readVInt();
      } else {
        arc->bytesPerArc = in_->readInt();
      }
      arc->arcIdx = -1;
      arc->nextArc = arc->posArcsStart = in_->getPosition();
      // System.out.println("  bytesPer=" + arc.bytesPerArc + " numArcs=" +
      // arc.numArcs + " arcsStart=" + pos);
    } else {
      // arc.flags = b;
      arc->nextArc = address;
      arc->bytesPerArc = 0;
    }

    return readNextRealArc(arc, in_);
  }

  /**
   * Checks if <code>arc</code>'s target state is in expanded (or deque)
   * format.
   *
   * @return Returns <code>true</code> if <code>arc</code> points to a state in
   * an expanded array format.
   */
  bool isExpandedTarget(std::shared_ptr<Arc<T>> follow,
                        std::shared_ptr<BytesReader> in_) 
  {
    if (!targetHasArcs(follow)) {
      return false;
    } else {
      in_->setPosition(follow->target);
      return in_->readByte() == ARCS_AS_FIXED_ARRAY;
    }
  }

  /** In-place read; returns the arc. */
  std::shared_ptr<Arc<T>>
  readNextArc(std::shared_ptr<Arc<T>> arc,
              std::shared_ptr<BytesReader> in_) 
  {
    if (arc->label == END_LABEL) {
      // This was a fake inserted "final" arc
      if (arc->nextArc <= 0) {
        throw std::invalid_argument(
            "cannot readNextArc when arc.isLast()=true");
      }
      return readFirstRealTargetArc(arc->nextArc, arc, in_);
    } else {
      return readNextRealArc(arc, in_);
    }
  }

  /** Peeks at next arc's label; does not alter arc.  Do
   *  not call this if arc.isLast()! */
  int readNextArcLabel(std::shared_ptr<Arc<T>> arc,
                       std::shared_ptr<BytesReader> in_) 
  {
    assert(!arc->isLast());

    if (arc->label == END_LABEL) {
      // System.out.println("    nextArc fake " +
      // arc.nextArc);

      int64_t pos = arc->nextArc;
      in_->setPosition(pos);

      constexpr char b = in_->readByte();
      if (b == ARCS_AS_FIXED_ARRAY) {
        // System.out.println("    nextArc fixed array");
        in_->readVInt();

        // Skip bytesPerArc:
        if (version >= VERSION_VINT_TARGET) {
          in_->readVInt();
        } else {
          in_->readInt();
        }
      } else {
        in_->setPosition(pos);
      }
    } else {
      if (arc->bytesPerArc != 0) {
        // System.out.println("    nextArc real array");
        // arcs are at fixed entries
        in_->setPosition(arc->posArcsStart);
        in_->skipBytes((1 + arc->arcIdx) * arc->bytesPerArc);
      } else {
        // arcs are packed
        // System.out.println("    nextArc real packed");
        in_->setPosition(arc->nextArc);
      }
    }
    // skip flags
    in_->readByte();
    return readLabel(in_);
  }

  /** Never returns null, but you should never call this if
   *  arc.isLast() is true. */
  std::shared_ptr<Arc<T>>
  readNextRealArc(std::shared_ptr<Arc<T>> arc,
                  std::shared_ptr<BytesReader> in_) 
  {

    // TODO: can't assert this because we call from readFirstArc
    // assert !flag(arc.flags, BIT_LAST_ARC);

    // this is a continuing arc in a fixed array
    if (arc->bytesPerArc != 0) {
      // arcs are at fixed entries
      arc->arcIdx++;
      assert(arc->arcIdx < arc->numArcs);
      in_->setPosition(arc->posArcsStart);
      in_->skipBytes(arc->arcIdx * arc->bytesPerArc);
    } else {
      // arcs are packed
      in_->setPosition(arc->nextArc);
    }
    arc->flags = in_->readByte();
    arc->label = readLabel(in_);

    if (arc->flag(BIT_ARC_HAS_OUTPUT)) {
      arc->output = outputs->read(in_);
    } else {
      arc->output = outputs->getNoOutput();
    }

    if (arc->flag(BIT_ARC_HAS_FINAL_OUTPUT)) {
      arc->nextFinalOutput = outputs->readFinalOutput(in_);
    } else {
      arc->nextFinalOutput = outputs->getNoOutput();
    }

    if (arc->flag(BIT_STOP_NODE)) {
      if (arc->flag(BIT_FINAL_ARC)) {
        arc->target = FINAL_END_NODE;
      } else {
        arc->target = NON_FINAL_END_NODE;
      }
      arc->nextArc = in_->getPosition();
    } else if (arc->flag(BIT_TARGET_NEXT)) {
      arc->nextArc = in_->getPosition();
      // TODO: would be nice to make this lazy -- maybe
      // caller doesn't need the target and is scanning arcs...
      if (!arc->flag(BIT_LAST_ARC)) {
        if (arc->bytesPerArc == 0) {
          // must scan
          seekToNextNode(in_);
        } else {
          in_->setPosition(arc->posArcsStart);
          in_->skipBytes(arc->bytesPerArc * arc->numArcs);
        }
      }
      arc->target = in_->getPosition();
    } else {
      arc->target = readUnpackedNodeTarget(in_);
      arc->nextArc = in_->getPosition();
    }
    return arc;
  }

  // LUCENE-5152: called only from asserts, to validate that the
  // non-cached arc lookup would produce the same result, to
  // catch callers that illegally modify shared structures with
  // the result (we shallow-clone the Arc itself, but e.g. a BytesRef
  // output is still shared):
private:
  bool assertRootCachedArc(int label,
                           std::shared_ptr<Arc<T>> cachedArc) 
  {
    std::shared_ptr<Arc<T>> arc = std::make_shared<Arc<T>>();
    getFirstArc(arc);
    std::shared_ptr<BytesReader> in_ = getBytesReader();
    std::shared_ptr<Arc<T>> result = findTargetArc(label, arc, arc, in_, false);
    if (result == nullptr) {
      assert(cachedArc == nullptr);
    } else {
      assert(cachedArc != nullptr);
      assert(cachedArc->arcIdx == result->arcIdx);
      assert(cachedArc->bytesPerArc == result->bytesPerArc);
      assert(cachedArc->flags == result->flags);
      assert(cachedArc->label == result->label);
      assert(cachedArc->nextArc == result->nextArc);
      assert(cachedArc->nextFinalOutput.equals(result->nextFinalOutput));
      assert(cachedArc->numArcs == result->numArcs);
      assert(cachedArc->output.equals(result->output));
      assert(cachedArc->posArcsStart == result->posArcsStart);
      assert(cachedArc->target == result->target);
    }

    return true;
  }

  // TODO: could we somehow [partially] tableize arc lookups
  // like automaton?

  /** Finds an arc leaving the incoming arc, replacing the arc in place.
   *  This returns null if the arc was not found, else the incoming arc. */
public:
  std::shared_ptr<Arc<T>>
  findTargetArc(int labelToMatch, std::shared_ptr<Arc<T>> follow,
                std::shared_ptr<Arc<T>> arc,
                std::shared_ptr<BytesReader> in_) 
  {
    return findTargetArc(labelToMatch, follow, arc, in_, true);
  }

  /** Finds an arc leaving the incoming arc, replacing the arc in place.
   *  This returns null if the arc was not found, else the incoming arc. */
private:
  std::shared_ptr<Arc<T>> findTargetArc(int labelToMatch,
                                        std::shared_ptr<Arc<T>> follow,
                                        std::shared_ptr<Arc<T>> arc,
                                        std::shared_ptr<BytesReader> in_,
                                        bool useRootArcCache) 
  {

    if (labelToMatch == END_LABEL) {
      if (follow->isFinal()) {
        if (follow->target <= 0) {
          arc->flags = BIT_LAST_ARC;
        } else {
          arc->flags = 0;
          // NOTE: nextArc is a node (not an address!) in this case:
          arc->nextArc = follow->target;
        }
        arc->output = follow->nextFinalOutput;
        arc->label = END_LABEL;
        return arc;
      } else {
        return nullptr;
      }
    }

    // Short-circuit if this arc is in the root arc cache:
    if (useRootArcCache && cachedRootArcs.size() > 0 &&
        follow->target == startNode && labelToMatch < cachedRootArcs.size()) {
      std::shared_ptr<Arc<T>> *const result = cachedRootArcs[labelToMatch];

      // LUCENE-5152: detect tricky cases where caller
      // modified previously returned cached root-arcs:
      assert((assertRootCachedArc(labelToMatch, result)));

      if (result == nullptr) {
        return nullptr;
      } else {
        arc->copyFrom(result);
        return arc;
      }
    }

    if (!targetHasArcs(follow)) {
      return nullptr;
    }

    in_->setPosition(follow->target);

    // System.out.println("fta label=" + (char) labelToMatch);

    if (in_->readByte() == ARCS_AS_FIXED_ARRAY) {
      // Arcs are full array; do binary search:
      arc->numArcs = in_->readVInt();
      if (version >= VERSION_VINT_TARGET) {
        arc->bytesPerArc = in_->readVInt();
      } else {
        arc->bytesPerArc = in_->readInt();
      }
      arc->posArcsStart = in_->getPosition();
      int low = 0;
      int high = arc->numArcs - 1;
      while (low <= high) {
        // System.out.println("    cycle");
        int mid =
            static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
        in_->setPosition(arc->posArcsStart);
        in_->skipBytes(arc->bytesPerArc * mid + 1);
        int midLabel = readLabel(in_);
        constexpr int cmp = midLabel - labelToMatch;
        if (cmp < 0) {
          low = mid + 1;
        } else if (cmp > 0) {
          high = mid - 1;
        } else {
          arc->arcIdx = mid - 1;
          // System.out.println("    found!");
          return readNextRealArc(arc, in_);
        }
      }

      return nullptr;
    }

    // Linear scan
    readFirstRealTargetArc(follow->target, arc, in_);

    while (true) {
      // System.out.println("  non-bs cycle");
      // TODO: we should fix this code to not have to create
      // object for the output of every arc we scan... only
      // for the matching arc, if found
      if (arc->label == labelToMatch) {
        // System.out.println("    found!");
        return arc;
      } else if (arc->label > labelToMatch) {
        return nullptr;
      } else if (arc->isLast()) {
        return nullptr;
      } else {
        readNextRealArc(arc, in_);
      }
    }
  }

  void seekToNextNode(std::shared_ptr<BytesReader> in_) 
  {

    while (true) {

      constexpr int flags = in_->readByte();
      readLabel(in_);

      if (flag(flags, BIT_ARC_HAS_OUTPUT)) {
        outputs->skipOutput(in_);
      }

      if (flag(flags, BIT_ARC_HAS_FINAL_OUTPUT)) {
        outputs->skipFinalOutput(in_);
      }

      if (!flag(flags, BIT_STOP_NODE) && !flag(flags, BIT_TARGET_NEXT)) {
        readUnpackedNodeTarget(in_);
      }

      if (flag(flags, BIT_LAST_ARC)) {
        return;
      }
    }
  }

  /**
   * Nodes will be expanded if their depth (distance from the root node) is
   * &lt;= this value and their number of arcs is &gt;=
   * {@link #FIXED_ARRAY_NUM_ARCS_SHALLOW}.
   *
   * <p>
   * Fixed array consumes more RAM but enables binary search on the arcs
   * (instead of a linear scan) on lookup by arc label.
   *
   * @return <code>true</code> if <code>node</code> should be stored in an
   *         expanded (array) form.
   *
   * @see #FIXED_ARRAY_NUM_ARCS_DEEP
   * @see Builder.UnCompiledNode#depth
   */
  bool shouldExpand(std::shared_ptr<Builder<T>> builder,
                    std::shared_ptr<Builder::UnCompiledNode<T>> node)
  {
    return builder->allowArrayArcs &&
           ((node->depth <= FIXED_ARRAY_SHALLOW_DISTANCE &&
             node->numArcs >= FIXED_ARRAY_NUM_ARCS_SHALLOW) ||
            node->numArcs >= FIXED_ARRAY_NUM_ARCS_DEEP);
  }

  /** Returns a {@link BytesReader} for this FST, positioned at
   *  position 0. */
public:
  std::shared_ptr<BytesReader> getBytesReader()
  {
    if (bytesArray.size() > 0) {
      return std::make_shared<ReverseBytesReader>(bytesArray);
    } else {
      return bytes->getReverseReader();
    }
  }

  /** Reads bytes stored in an FST. */
public:
  class BytesReader : public DataInput
  {
    GET_CLASS_NAME(BytesReader)
    /** Get current read position. */
  public:
    virtual int64_t getPosition() = 0;

    /** Set current read position. */
    virtual void setPosition(int64_t pos) = 0;

    /** Returns true if this reader uses reversed bytes
     *  under-the-hood. */
    virtual bool reversed() = 0;

  protected:
    std::shared_ptr<BytesReader> shared_from_this()
    {
      return std::static_pointer_cast<BytesReader>(
          org.apache.lucene.store.DataInput::shared_from_this());
    }
  };

  /*
  public void countSingleChains() throws IOException {
    // TODO: must assert this FST was built with
    // "willRewrite"

    final List<ArcAndState<T>> queue = new ArrayList<>();

    // TODO: use bitset to not revisit nodes already
    // visited

    FixedBitSet seen = new FixedBitSet(1+nodeCount);
    int saved = 0;

    queue.add(new ArcAndState<T>(getFirstArc(new Arc<T>()), new IntsRef()));
    Arc<T> scratchArc = new Arc<>();
    while(queue.size() > 0) {
      //System.out.println("cycle size=" + queue.size());
      //for(ArcAndState<T> ent : queue) {
      //  System.out.println("  " + Util.toBytesRef(ent.chain, new BytesRef()));
      //  }
      final ArcAndState<T> arcAndState = queue.get(queue.size()-1);
      seen.set(arcAndState.arc.node);
      final BytesRef br = Util.toBytesRef(arcAndState.chain, new BytesRef());
      if (br.length > 0 && br.bytes[br.length-1] == -1) {
        br.length--;
      }
      //System.out.println("  top node=" + arcAndState.arc.target + " chain=" +
  br.utf8ToString()); if (targetHasArcs(arcAndState.arc) &&
  !seen.get(arcAndState.arc.target)) {
        // push
        readFirstTargetArc(arcAndState.arc, scratchArc);
        //System.out.println("  push label=" + (char) scratchArc.label);
        //System.out.println("    tonode=" + scratchArc.target + " last?=" +
  scratchArc.isLast());

        final IntsRef chain = IntsRef.deepCopyOf(arcAndState.chain);
        chain.grow(1+chain.length);
        // TODO
        //assert scratchArc.label != END_LABEL;
        chain.ints[chain.length] = scratchArc.label;
        chain.length++;

        if (scratchArc.isLast()) {
          if (scratchArc.target != -1 && inCounts[scratchArc.target] == 1) {
            //System.out.println("    append");
          } else {
            if (arcAndState.chain.length > 1) {
              saved += chain.length-2;
              try {
                System.out.println("chain: " + Util.toBytesRef(chain, new
  BytesRef()).utf8ToString()); } catch (AssertionError ae) {
                System.out.println("chain: " + Util.toBytesRef(chain, new
  BytesRef()));
              }
            }
            chain.length = 0;
          }
        } else {
          //System.out.println("    reset");
          if (arcAndState.chain.length > 1) {
            saved += arcAndState.chain.length-2;
            try {
              System.out.println("chain: " + Util.toBytesRef(arcAndState.chain,
  new BytesRef()).utf8ToString()); } catch (AssertionError ae) {
              System.out.println("chain: " + Util.toBytesRef(arcAndState.chain,
  new BytesRef()));
            }
          }
          if (scratchArc.target != -1 && inCounts[scratchArc.target] != 1) {
            chain.length = 0;
          } else {
            chain.ints[0] = scratchArc.label;
            chain.length = 1;
          }
        }
        // TODO: instead of new Arc() we can re-use from
        // a by-depth array
        queue.add(new ArcAndState<T>(new Arc<T>().copyFrom(scratchArc), chain));
      } else if (!arcAndState.arc.isLast()) {
        // next
        readNextArc(arcAndState.arc);
        //System.out.println("  next label=" + (char) arcAndState.arc.label + "
  len=" + arcAndState.chain.length); if (arcAndState.chain.length != 0) {
          arcAndState.chain.ints[arcAndState.chain.length-1] =
  arcAndState.arc.label;
        }
      } else {
        if (arcAndState.chain.length > 1) {
          saved += arcAndState.chain.length-2;
          System.out.println("chain: " + Util.toBytesRef(arcAndState.chain, new
  BytesRef()).utf8ToString());
        }
        // pop
        //System.out.println("  pop");
        queue.remove(queue.size()-1);
        while(queue.size() > 0 && queue.get(queue.size()-1).arc.isLast()) {
          queue.remove(queue.size()-1);
        }
        if (queue.size() > 0) {
          final ArcAndState<T> arcAndState2 = queue.get(queue.size()-1);
          readNextArc(arcAndState2.arc);
          //System.out.println("  read next=" + (char) arcAndState2.arc.label +
  " queue=" + queue.size()); assert arcAndState2.arc.label != END_LABEL; if
  (arcAndState2.chain.length != 0) {
            arcAndState2.chain.ints[arcAndState2.chain.length-1] =
  arcAndState2.arc.label;
          }
        }
      }
    }

    System.out.println("TOT saved " + saved);
  }
 */
};

} // #include  "core/src/java/org/apache/lucene/util/fst/
