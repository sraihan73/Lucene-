using namespace std;

#include "FSTTermOutputs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"

namespace org::apache::lucene::codecs::memory
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using Outputs = org::apache::lucene::util::fst::Outputs;
const shared_ptr<TermData> FSTTermOutputs::NO_OUTPUT = make_shared<TermData>();

FSTTermOutputs::TermData::TermData()
{
  this->longs.clear();
  this->bytes.clear();
  this->docFreq = 0;
  this->totalTermFreq = -1;
}

FSTTermOutputs::TermData::TermData(std::deque<int64_t> &longs,
                                   std::deque<char> &bytes, int docFreq,
                                   int64_t totalTermFreq)
{
  this->longs = longs;
  this->bytes = bytes;
  this->docFreq = docFreq;
  this->totalTermFreq = totalTermFreq;
}

int64_t FSTTermOutputs::TermData::ramBytesUsed()
{
  int64_t ramBytesUsed = BASE_RAM_BYTES_USED;
  if (longs.size() > 0) {
    ramBytesUsed += RamUsageEstimator::sizeOf(longs);
  }
  if (bytes.size() > 0) {
    ramBytesUsed += RamUsageEstimator::sizeOf(bytes);
  }
  return ramBytesUsed;
}

int FSTTermOutputs::TermData::hashCode()
{
  int hash = 0;
  if (longs.size() > 0) {
    constexpr int end = longs.size();
    for (int i = 0; i < end; i++) {
      hash -= longs[i];
    }
  }
  if (bytes.size() > 0) {
    hash = -hash;
    constexpr int end = bytes.size();
    for (int i = 0; i < end; i++) {
      hash += bytes[i];
    }
  }
  hash += docFreq + totalTermFreq;
  return hash;
}

wstring FSTTermOutputs::TermData::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FSTTermOutputs$TermData longs=" + Arrays->toString(longs) +
         L" bytes=" + Arrays->toString(bytes) + L" docFreq=" +
         to_wstring(docFreq) + L" totalTermFreq=" + to_wstring(totalTermFreq);
}

bool FSTTermOutputs::TermData::equals(any other_)
{
  if (other_ == shared_from_this()) {
    return true;
  } else if (!(std::dynamic_pointer_cast<FSTTermOutputs::TermData>(other_) !=
               nullptr)) {
    return false;
  }
  shared_ptr<TermData> other = any_cast<std::shared_ptr<TermData>>(other_);
  return statsEqual(shared_from_this(), other) &&
         longsEqual(shared_from_this(), other) &&
         bytesEqual(shared_from_this(), other);
}

FSTTermOutputs::FSTTermOutputs(shared_ptr<FieldInfo> fieldInfo, int longsSize)
    : hasPos(fieldInfo->getIndexOptions() != IndexOptions::DOCS),
      longsSize(longsSize)
{
}

int64_t FSTTermOutputs::ramBytesUsed(shared_ptr<TermData> output)
{
  return output->ramBytesUsed();
}

shared_ptr<TermData> FSTTermOutputs::common(shared_ptr<TermData> t1,
                                            shared_ptr<TermData> t2)
{
  // if (TEST) System.out.print("common("+t1+", "+t2+") = ");
  if (t1 == NO_OUTPUT || t2 == NO_OUTPUT) {
    // if (TEST) System.out.println("ret:"+NO_OUTPUT);
    return NO_OUTPUT;
  }
  assert(t1->longs.size() == t2->longs.size());

  std::deque<int64_t> min = t1->longs, max = t2->longs;
  int pos = 0;
  shared_ptr<TermData> ret;

  while (pos < longsSize && min[pos] == max[pos]) {
    pos++;
  }
  if (pos < longsSize) { // unequal long[]
    if (min[pos] > max[pos]) {
      min = t2->longs;
      max = t1->longs;
    }
    // check whether strictly smaller
    while (pos < longsSize && min[pos] <= max[pos]) {
      pos++;
    }
    if (pos < longsSize || allZero(min)) { // not comparable or all-zero
      ret = NO_OUTPUT;
    } else {
      ret = make_shared<TermData>(min, nullptr, 0, -1);
    }
  } else { // equal long[]
    if (statsEqual(t1, t2) && bytesEqual(t1, t2)) {
      ret = t1;
    } else if (allZero(min)) {
      ret = NO_OUTPUT;
    } else {
      ret = make_shared<TermData>(min, nullptr, 0, -1);
    }
  }
  // if (TEST) System.out.println("ret:"+ret);
  return ret;
}

shared_ptr<TermData> FSTTermOutputs::subtract(shared_ptr<TermData> t1,
                                              shared_ptr<TermData> t2)
{
  // if (TEST) System.out.print("subtract("+t1+", "+t2+") = ");
  if (t2 == NO_OUTPUT) {
    // if (TEST) System.out.println("ret:"+t1);
    return t1;
  }
  assert(t1->longs.size() == t2->longs.size());

  int pos = 0;
  int64_t diff = 0;
  std::deque<int64_t> share(longsSize);

  while (pos < longsSize) {
    share[pos] = t1->longs[pos] - t2->longs[pos];
    diff += share[pos];
    pos++;
  }

  shared_ptr<TermData> ret;
  if (diff == 0 && statsEqual(t1, t2) && bytesEqual(t1, t2)) {
    ret = NO_OUTPUT;
  } else {
    ret =
        make_shared<TermData>(share, t1->bytes, t1->docFreq, t1->totalTermFreq);
  }
  // if (TEST) System.out.println("ret:"+ret);
  return ret;
}

shared_ptr<TermData> FSTTermOutputs::add(shared_ptr<TermData> t1,
                                         shared_ptr<TermData> t2)
{
  // if (TEST) System.out.print("add("+t1+", "+t2+") = ");
  if (t1 == NO_OUTPUT) {
    // if (TEST) System.out.println("ret:"+t2);
    return t2;
  } else if (t2 == NO_OUTPUT) {
    // if (TEST) System.out.println("ret:"+t1);
    return t1;
  }
  assert(t1->longs.size() == t2->longs.size());

  int pos = 0;
  std::deque<int64_t> accum(longsSize);

  while (pos < longsSize) {
    accum[pos] = t1->longs[pos] + t2->longs[pos];
    pos++;
  }

  shared_ptr<TermData> ret;
  if (t2->bytes.size() > 0 || t2->docFreq > 0) {
    ret =
        make_shared<TermData>(accum, t2->bytes, t2->docFreq, t2->totalTermFreq);
  } else {
    ret =
        make_shared<TermData>(accum, t1->bytes, t1->docFreq, t1->totalTermFreq);
  }
  // if (TEST) System.out.println("ret:"+ret);
  return ret;
}

void FSTTermOutputs::write(shared_ptr<TermData> data,
                           shared_ptr<DataOutput> out) 
{
  assert(hasPos || data->totalTermFreq == -1);
  int bit0 = allZero(data->longs) ? 0 : 1;
  int bit1 = ((data->bytes.empty() || data->bytes.empty()) ? 0 : 1) << 1;
  int bit2 = ((data->docFreq == 0) ? 0 : 1) << 2;
  int bits = bit0 | bit1 | bit2;
  if (bit1 > 0) { // determine extra length
    if (data->bytes.size() < 32) {
      bits |= (data->bytes.size() << 3);
      out->writeByte(static_cast<char>(bits));
    } else {
      out->writeByte(static_cast<char>(bits));
      out->writeVInt(data->bytes.size());
    }
  } else {
    out->writeByte(static_cast<char>(bits));
  }
  if (bit0 > 0) { // not all-zero case
    for (int pos = 0; pos < longsSize; pos++) {
      out->writeVLong(data->longs[pos]);
    }
  }
  if (bit1 > 0) { // bytes exists
    out->writeBytes(data->bytes, 0, data->bytes.size());
  }
  if (bit2 > 0) { // stats exist
    if (hasPos) {
      if (data->docFreq == data->totalTermFreq) {
        out->writeVInt((data->docFreq << 1) | 1);
      } else {
        out->writeVInt((data->docFreq << 1));
        out->writeVLong(data->totalTermFreq - data->docFreq);
      }
    } else {
      out->writeVInt(data->docFreq);
    }
  }
}

shared_ptr<TermData>
FSTTermOutputs::read(shared_ptr<DataInput> in_) 
{
  std::deque<int64_t> longs(longsSize);
  std::deque<char> bytes;
  int docFreq = 0;
  int64_t totalTermFreq = -1;
  int bits = in_->readByte() & 0xff;
  int bit0 = bits & 1;
  int bit1 = bits & 2;
  int bit2 = bits & 4;
  int bytesSize = (static_cast<int>(static_cast<unsigned int>(bits) >> 3));
  if (bit1 > 0 && bytesSize == 0) { // determine extra length
    bytesSize = in_->readVInt();
  }
  if (bit0 > 0) { // not all-zero case
    for (int pos = 0; pos < longsSize; pos++) {
      longs[pos] = in_->readVLong();
    }
  }
  if (bit1 > 0) { // bytes exists
    bytes = std::deque<char>(bytesSize);
    in_->readBytes(bytes, 0, bytesSize);
  }
  if (bit2 > 0) { // stats exist
    int code = in_->readVInt();
    if (hasPos) {
      totalTermFreq = docFreq =
          static_cast<int>(static_cast<unsigned int>(code) >> 1);
      if ((code & 1) == 0) {
        totalTermFreq += in_->readVLong();
      }
    } else {
      docFreq = code;
    }
  }
  return make_shared<TermData>(longs, bytes, docFreq, totalTermFreq);
}

void FSTTermOutputs::skipOutput(shared_ptr<DataInput> in_) 
{
  int bits = in_->readByte() & 0xff;
  int bit0 = bits & 1;
  int bit1 = bits & 2;
  int bit2 = bits & 4;
  int bytesSize = (static_cast<int>(static_cast<unsigned int>(bits) >> 3));
  if (bit1 > 0 && bytesSize == 0) { // determine extra length
    bytesSize = in_->readVInt();
  }
  if (bit0 > 0) { // not all-zero case
    for (int pos = 0; pos < longsSize; pos++) {
      in_->readVLong();
    }
  }
  if (bit1 > 0) { // bytes exists
    in_->skipBytes(bytesSize);
  }
  if (bit2 > 0) { // stats exist
    int code = in_->readVInt();
    if (hasPos && (code & 1) == 0) {
      in_->readVLong();
    }
  }
}

shared_ptr<TermData> FSTTermOutputs::getNoOutput() { return NO_OUTPUT; }

wstring FSTTermOutputs::outputToString(shared_ptr<TermData> data)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return data->toString();
}

bool FSTTermOutputs::statsEqual(shared_ptr<TermData> t1,
                                shared_ptr<TermData> t2)
{
  return t1->docFreq == t2->docFreq && t1->totalTermFreq == t2->totalTermFreq;
}

bool FSTTermOutputs::bytesEqual(shared_ptr<TermData> t1,
                                shared_ptr<TermData> t2)
{
  if (t1->bytes.empty() && t2->bytes.empty()) {
    return true;
  }
  return t1->bytes.size() > 0 && t2->bytes.size() > 0 &&
         Arrays::equals(t1->bytes, t2->bytes);
}

bool FSTTermOutputs::longsEqual(shared_ptr<TermData> t1,
                                shared_ptr<TermData> t2)
{
  if (t1->longs.empty() && t2->longs.empty()) {
    return true;
  }
  return t1->longs.size() > 0 && t2->longs.size() > 0 &&
         Arrays::equals(t1->longs, t2->longs);
}

bool FSTTermOutputs::allZero(std::deque<int64_t> &l)
{
  for (int i = 0; i < l.size(); i++) {
    if (l[i] != 0) {
      return false;
    }
  }
  return true;
}
} // namespace org::apache::lucene::codecs::memory