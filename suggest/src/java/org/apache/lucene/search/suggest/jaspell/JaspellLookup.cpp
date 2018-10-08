using namespace std;

#include "JaspellLookup.h"

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
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

JaspellLookup::JaspellLookup() {}

void JaspellLookup::build(shared_ptr<InputIterator> iterator) 
{
  if (iterator->hasPayloads()) {
    throw invalid_argument(L"this suggester doesn't support payloads");
  }
  if (iterator->hasContexts()) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  count = 0;
  trie = make_shared<JaspellTernarySearchTrie>();
  trie->setMatchAlmostDiff(editDistance);
  shared_ptr<BytesRef> spare;
  shared_ptr<CharsRefBuilder> *const charsSpare =
      make_shared<CharsRefBuilder>();

  while ((spare = iterator->next()) != nullptr) {
    constexpr int64_t weight = iterator->weight();
    if (spare->length == 0) {
      continue;
    }
    charsSpare->copyUTF8Bytes(spare);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    trie->put(charsSpare->toString(), static_cast<int64_t>(weight));
    count++;
  }
}

bool JaspellLookup::add(shared_ptr<std::wstring> key, any value)
{
  trie->put(key, value);
  // XXX
  return false;
}

any JaspellLookup::get(shared_ptr<std::wstring> key) { return trie->get(key); }

deque<std::shared_ptr<Lookup::LookupResult>>
JaspellLookup::lookup(shared_ptr<std::wstring> key,
                      shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                      bool onlyMorePopular, int num)
{
  if (contexts != nullptr) {
    throw invalid_argument(L"this suggester doesn't support contexts");
  }
  deque<std::shared_ptr<Lookup::LookupResult>> res =
      deque<std::shared_ptr<Lookup::LookupResult>>();
  deque<wstring> deque;
  int count = onlyMorePopular ? num * 2 : num;
  if (usePrefix) {
    deque = trie->matchPrefix(key, count);
  } else {
    deque = trie->matchAlmost(key, count);
  }
  if (deque.empty() || deque.empty()) {
    return res;
  }
  int maxCnt = min(num, deque.size());
  if (onlyMorePopular) {
    shared_ptr<Lookup::LookupPriorityQueue> queue =
        make_shared<Lookup::LookupPriorityQueue>(num);
    for (auto s : deque) {
      int64_t freq =
          (any_cast<std::shared_ptr<Number>>(trie->get(s))).longValue();
      queue->insertWithOverflow(
          make_shared<Lookup::LookupResult>(make_shared<CharsRef>(s), freq));
    }
    for (auto lr : queue->getResults()) {
      res.push_back(lr);
    }
  } else {
    for (int i = 0; i < maxCnt; i++) {
      wstring s = deque[i];
      int64_t freq =
          (any_cast<std::shared_ptr<Number>>(trie->get(s))).longValue();
      res.push_back(
          make_shared<Lookup::LookupResult>(make_shared<CharsRef>(s), freq));
    }
  }
  return res;
}

void JaspellLookup::readRecursively(shared_ptr<DataInput> in_,
                                    shared_ptr<TSTNode> node) 
{
  node->splitchar = in_->readString()[0];
  char mask = in_->readByte();
  if ((mask & HAS_VALUE) != 0) {
    node->data = static_cast<int64_t>(in_->readLong());
  }
  if ((mask & LO_KID) != 0) {
    shared_ptr<TSTNode> kid = make_shared<TSTNode>(L'\0', node);
    node->relatives[TSTNode::LOKID] = kid;
    readRecursively(in_, kid);
  }
  if ((mask & EQ_KID) != 0) {
    shared_ptr<TSTNode> kid = make_shared<TSTNode>(L'\0', node);
    node->relatives[TSTNode::EQKID] = kid;
    readRecursively(in_, kid);
  }
  if ((mask & HI_KID) != 0) {
    shared_ptr<TSTNode> kid = make_shared<TSTNode>(L'\0', node);
    node->relatives[TSTNode::HIKID] = kid;
    readRecursively(in_, kid);
  }
}

void JaspellLookup::writeRecursively(
    shared_ptr<DataOutput> out, shared_ptr<TSTNode> node) 
{
  if (node == nullptr) {
    return;
  }
  out->writeString(wstring(std::deque<wchar_t>{node->splitchar}, 0, 1));
  char mask = 0;
  if (node->relatives[TSTNode::LOKID] != nullptr) {
    mask |= LO_KID;
  }
  if (node->relatives[TSTNode::EQKID] != nullptr) {
    mask |= EQ_KID;
  }
  if (node->relatives[TSTNode::HIKID] != nullptr) {
    mask |= HI_KID;
  }
  if (node->data != nullptr) {
    mask |= HAS_VALUE;
  }
  out->writeByte(mask);
  if (node->data != nullptr) {
    out->writeLong((any_cast<std::shared_ptr<Number>>(node->data)).longValue());
  }
  writeRecursively(out, node->relatives[TSTNode::LOKID]);
  writeRecursively(out, node->relatives[TSTNode::EQKID]);
  writeRecursively(out, node->relatives[TSTNode::HIKID]);
}

bool JaspellLookup::store(shared_ptr<DataOutput> output) 
{
  output->writeVLong(count);
  shared_ptr<TSTNode> root = trie->getRoot();
  if (root == nullptr) { // empty tree
    return false;
  }
  writeRecursively(output, root);
  return true;
}

bool JaspellLookup::load(shared_ptr<DataInput> input) 
{
  count = input->readVLong();
  shared_ptr<TSTNode> root = make_shared<TSTNode>(L'\0', nullptr);
  readRecursively(input, root);
  trie->setRoot(root);
  return true;
}

int64_t JaspellLookup::ramBytesUsed() { return trie->ramBytesUsed(); }

int64_t JaspellLookup::getCount() { return count; }
} // namespace org::apache::lucene::search::suggest::jaspell