using namespace std;

#include "TSTLookup.h"

namespace org::apache::lucene::search::suggest::tst
{
using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using SortedInputIterator =
    org::apache::lucene::search::suggest::SortedInputIterator;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

TSTLookup::TSTLookup() : TSTLookup(nullptr, nullptr) {}

TSTLookup::TSTLookup(shared_ptr<Directory> tempDir,
                     const wstring &tempFileNamePrefix)
    : tempDir(tempDir), tempFileNamePrefix(tempFileNamePrefix)
{
}

const shared_ptr<java::util::Comparator<
    std::shared_ptr<org::apache::lucene::util::BytesRef>>>
    TSTLookup::utf8SortedAsUTF16SortOrder = [&](a, b) {
      const std::deque<char> aBytes = a::bytes;
      int aUpto = a::offset;
      const std::deque<char> bBytes = b::bytes;
      int bUpto = b::offset;

      constexpr int aStop = aUpto + min(a->length, b->length);

      while (aUpto < aStop) {
        int aByte = aBytes[aUpto++] & 0xff;
        int bByte = bBytes[bUpto++] & 0xff;

        if (aByte != bByte) {

          // See
          // http://icu-project.org/docs/papers/utf16_code_point_order.html#utf-8-in-utf-16-order

          // We know the terms are not equal, but, we may
          // have to carefully fixup the bytes at the
          // difference to match UTF16's sort order:

          // NOTE: instead of moving supplementary code points (0xee and 0xef)
          // to the unused 0xfe and 0xff, we move them to the unused 0xfc and
          // 0xfd [reserved for future 6-TangibleTempchar character sequences]
          // this reserves 0xff for preflex's term reordering (surrogate dance),
          // and if unicode grows such that 6-TangibleTempchar sequences are
          // needed we have much bigger problems anyway.
          if (aByte >= 0xee && bByte >= 0xee) {
            if ((aByte & 0xfe) == 0xee) {
              aByte += 0xe;
            }
            if ((bByte & 0xfe) == 0xee) {
              bByte += 0xe;
            }
          }
          return aByte - bByte;
        }
      }

      // One is a prefix of the other, or, they are equal:
      return a->length - b->length;
    };

void TSTLookup::build(shared_ptr<InputIterator> iterator) 
{
  if (iterator->hasPayloads()) {
    throw invalid_argument(L"this suggester doesn't support payloads");
  }
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  root = make_shared<TernaryTreeNode>();

  // make sure it's sorted and the comparator uses UTF16 sort order
  iterator = make_shared<SortedInputIterator>(
      tempDir, tempFileNamePrefix, iterator, utf8SortedAsUTF16SortOrder);
  count = 0;
  deque<wstring> tokens = deque<wstring>();
  deque<std::shared_ptr<Number>> vals = deque<std::shared_ptr<Number>>();
  shared_ptr<BytesRef> spare;
  shared_ptr<CharsRefBuilder> charsSpare = make_shared<CharsRefBuilder>();
  while ((spare = iterator->next()) != nullptr) {
    charsSpare->copyUTF8Bytes(spare);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    tokens.push_back(charsSpare->toString());
    vals.push_back(static_cast<int64_t>(iterator->weight()));
    count++;
  }
  autocomplete->balancedTree(tokens.toArray(), vals.toArray(), 0,
                             tokens.size() - 1, root);
}

bool TSTLookup::add(shared_ptr<std::wstring> key, any value)
{
  autocomplete->insert(root, key, value, 0);
  // XXX we don't know if a new node was created
  return true;
}

any TSTLookup::get(shared_ptr<std::wstring> key)
{
  deque<std::shared_ptr<TernaryTreeNode>> deque =
      autocomplete->prefixCompletion(root, key, 0);
  if (deque.empty() || deque.empty()) {
    return nullptr;
  }
  for (auto n : deque) {
    if (charSeqEquals(n->token, key)) {
      return n->val;
    }
  }
  return nullptr;
}

bool TSTLookup::charSeqEquals(shared_ptr<std::wstring> left,
                              shared_ptr<std::wstring> right)
{
  int len = left->length();
  if (len != right->length()) {
    return false;
  }
  for (int i = 0; i < len; i++) {
    if (left->charAt(i) != right->charAt(i)) {
      return false;
    }
  }
  return true;
}

deque<std::shared_ptr<Lookup::LookupResult>>
TSTLookup::lookup(shared_ptr<std::wstring> key,
                  shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                  bool onlyMorePopular, int num)
{
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  deque<std::shared_ptr<TernaryTreeNode>> deque =
      autocomplete->prefixCompletion(root, key, 0);
  deque<std::shared_ptr<Lookup::LookupResult>> res =
      deque<std::shared_ptr<Lookup::LookupResult>>();
  if (deque.empty() || deque.empty()) {
    return res;
  }
  int maxCnt = min(num, deque.size());
  if (onlyMorePopular) {
    shared_ptr<Lookup::LookupPriorityQueue> queue =
        make_shared<Lookup::LookupPriorityQueue>(num);

    for (auto ttn : deque) {
      queue->insertWithOverflow(make_shared<Lookup::LookupResult>(
          ttn->token,
          (any_cast<std::shared_ptr<Number>>(ttn->val)).longValue()));
    }
    for (auto lr : queue->getResults()) {
      res.push_back(lr);
    }
  } else {
    for (int i = 0; i < maxCnt; i++) {
      shared_ptr<TernaryTreeNode> ttn = deque[i];
      res.push_back(make_shared<Lookup::LookupResult>(
          ttn->token,
          (any_cast<std::shared_ptr<Number>>(ttn->val)).longValue()));
    }
  }
  return res;
}

void TSTLookup::readRecursively(
    shared_ptr<DataInput> in_,
    shared_ptr<TernaryTreeNode> node) 
{
  node->splitchar = in_->readString()[0];
  char mask = in_->readByte();
  if ((mask & HAS_TOKEN) != 0) {
    node->token = in_->readString();
  }
  if ((mask & HAS_VALUE) != 0) {
    node->val = static_cast<int64_t>(in_->readLong());
  }
  if ((mask & LO_KID) != 0) {
    node->loKid = make_shared<TernaryTreeNode>();
    readRecursively(in_, node->loKid);
  }
  if ((mask & EQ_KID) != 0) {
    node->eqKid = make_shared<TernaryTreeNode>();
    readRecursively(in_, node->eqKid);
  }
  if ((mask & HI_KID) != 0) {
    node->hiKid = make_shared<TernaryTreeNode>();
    readRecursively(in_, node->hiKid);
  }
}

void TSTLookup::writeRecursively(
    shared_ptr<DataOutput> out,
    shared_ptr<TernaryTreeNode> node) 
{
  // write out the current node
  out->writeString(wstring(std::deque<wchar_t>{node->splitchar}, 0, 1));
  // prepare a mask of kids
  char mask = 0;
  if (node->eqKid != nullptr) {
    mask |= EQ_KID;
  }
  if (node->loKid != nullptr) {
    mask |= LO_KID;
  }
  if (node->hiKid != nullptr) {
    mask |= HI_KID;
  }
  if (node->token != L"") {
    mask |= HAS_TOKEN;
  }
  if (node->val != nullptr) {
    mask |= HAS_VALUE;
  }
  out->writeByte(mask);
  if (node->token != L"") {
    out->writeString(node->token);
  }
  if (node->val != nullptr) {
    out->writeLong((any_cast<std::shared_ptr<Number>>(node->val)).longValue());
  }
  // recurse and write kids
  if (node->loKid != nullptr) {
    writeRecursively(out, node->loKid);
  }
  if (node->eqKid != nullptr) {
    writeRecursively(out, node->eqKid);
  }
  if (node->hiKid != nullptr) {
    writeRecursively(out, node->hiKid);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool TSTLookup::store(shared_ptr<DataOutput> output) 
{
  output->writeVLong(count);
  writeRecursively(output, root);
  return true;
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool TSTLookup::load(shared_ptr<DataInput> input) 
{
  count = input->readVLong();
  root = make_shared<TernaryTreeNode>();
  readRecursively(input, root);
  return true;
}

int64_t TSTLookup::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this());
  if (root != nullptr) {
    mem += root->sizeInBytes();
  }
  return mem;
}

int64_t TSTLookup::getCount() { return count; }
} // namespace org::apache::lucene::search::suggest::tst