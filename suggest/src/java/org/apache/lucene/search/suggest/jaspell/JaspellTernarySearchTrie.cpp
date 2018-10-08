using namespace std;

#include "JaspellTernarySearchTrie.h"

namespace org::apache::lucene::search::suggest::jaspell
{
using Accountable = org::apache::lucene::util::Accountable;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

JaspellTernarySearchTrie::TSTNode::TSTNode(wchar_t splitchar,
                                           shared_ptr<TSTNode> parent)
{
  this->splitchar = splitchar;
  relatives[PARENT] = parent;
}

int64_t JaspellTernarySearchTrie::TSTNode::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this()) +
                  RamUsageEstimator::shallowSizeOf(relatives);
  // We don't need to add parent since our parent added itself:
  for (int i = 1; i < 4; i++) {
    shared_ptr<TSTNode> node = relatives[i];
    if (node != nullptr) {
      mem += node->ramBytesUsed();
    }
  }
  return mem;
}

int JaspellTernarySearchTrie::compareCharsAlphabetically(wchar_t cCompare2,
                                                         wchar_t cRef)
{
  return towlower(cCompare2) - towlower(cRef);
}

JaspellTernarySearchTrie::JaspellTernarySearchTrie()
    : JaspellTernarySearchTrie(Locale::ROOT)
{
}

JaspellTernarySearchTrie::JaspellTernarySearchTrie(shared_ptr<Locale> locale)
    : locale(locale)
{
}

void JaspellTernarySearchTrie::setRoot(shared_ptr<TSTNode> newRoot)
{
  rootNode = newRoot;
}

shared_ptr<TSTNode> JaspellTernarySearchTrie::getRoot() { return rootNode; }

JaspellTernarySearchTrie::JaspellTernarySearchTrie(shared_ptr<Path> file) throw(
    IOException)
    : JaspellTernarySearchTrie(file, false)
{
}

JaspellTernarySearchTrie::JaspellTernarySearchTrie(
    shared_ptr<Path> file, bool compression) 
    : JaspellTernarySearchTrie()
{
  shared_ptr<BufferedReader> in_;
  if (compression) {
    in_ = make_shared<BufferedReader>(IOUtils::getDecodingReader(
        make_shared<GZIPInputStream>(Files::newInputStream(file)),
        StandardCharsets::UTF_8));
  } else {
    in_ = Files::newBufferedReader(file, StandardCharsets::UTF_8);
  }
  try {
    wstring word;
    int pos;
    optional<float> occur, one = optional<float>(1);
    while ((word = in_->readLine()) != L"") {
      pos = (int)word.find(L"\t");
      occur = one;
      if (pos != -1) {
        occur = stof(word.substr(pos + 1)->trim());
        word = word.substr(0, pos);
      }
      wstring key = word.toLowerCase(locale);
      if (rootNode == nullptr) {
        rootNode = make_shared<TSTNode>(key[0], nullptr);
      }
      shared_ptr<TSTNode> node = nullptr;
      if (key.length() > 0 && rootNode != nullptr) {
        shared_ptr<TSTNode> currentNode = rootNode;
        int charIndex = 0;
        while (true) {
          if (currentNode == nullptr) {
            break;
          }
          int charComp = compareCharsAlphabetically(key[charIndex],
                                                    currentNode->splitchar);
          if (charComp == 0) {
            charIndex++;
            if (charIndex == key.length()) {
              node = currentNode;
              break;
            }
            currentNode = currentNode->relatives[TSTNode::EQKID];
          } else if (charComp < 0) {
            currentNode = currentNode->relatives[TSTNode::LOKID];
          } else {
            currentNode = currentNode->relatives[TSTNode::HIKID];
          }
        }
        optional<float> occur2 = nullopt;
        if (node != nullptr) {
          occur2 = (any_cast<optional<float>>(node->data));
        }
        if (occur2) {
          occur += occur2.value();
        }
        currentNode =
            getOrCreateNode(StringHelper::trim(word)->toLowerCase(locale));
        currentNode->data = occur;
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({in_});
  }
}

void JaspellTernarySearchTrie::deleteNode(shared_ptr<TSTNode> nodeToDelete)
{
  if (nodeToDelete == nullptr) {
    return;
  }
  nodeToDelete->data = nullptr;
  while (nodeToDelete != nullptr) {
    nodeToDelete = deleteNodeRecursion(nodeToDelete);
    // deleteNodeRecursion(nodeToDelete);
  }
}

shared_ptr<TSTNode>
JaspellTernarySearchTrie::deleteNodeRecursion(shared_ptr<TSTNode> currentNode)
{
  if (currentNode == nullptr) {
    return nullptr;
  }
  if (currentNode->relatives[TSTNode::EQKID] != nullptr ||
      currentNode->data != nullptr) {
    return nullptr;
  }
  // can't delete this node if it has a non-null eq kid or data
  shared_ptr<TSTNode> currentParent = currentNode->relatives[TSTNode::PARENT];
  bool lokidNull = currentNode->relatives[TSTNode::LOKID] == nullptr;
  bool hikidNull = currentNode->relatives[TSTNode::HIKID] == nullptr;
  int childType;
  if (currentParent->relatives[TSTNode::LOKID] == currentNode) {
    childType = TSTNode::LOKID;
  } else if (currentParent->relatives[TSTNode::EQKID] == currentNode) {
    childType = TSTNode::EQKID;
  } else if (currentParent->relatives[TSTNode::HIKID] == currentNode) {
    childType = TSTNode::HIKID;
  } else {
    rootNode.reset();
    return nullptr;
  }
  if (lokidNull && hikidNull) {
    currentParent->relatives[childType].reset();
    return currentParent;
  }
  if (lokidNull) {
    currentParent->relatives[childType] =
        currentNode->relatives[TSTNode::HIKID];
    currentNode->relatives[TSTNode::HIKID]->relatives[TSTNode::PARENT] =
        currentParent;
    return currentParent;
  }
  if (hikidNull) {
    currentParent->relatives[childType] =
        currentNode->relatives[TSTNode::LOKID];
    currentNode->relatives[TSTNode::LOKID]->relatives[TSTNode::PARENT] =
        currentParent;
    return currentParent;
  }
  int deltaHi = currentNode->relatives[TSTNode::HIKID]->splitchar -
                currentNode->splitchar;
  int deltaLo = currentNode->splitchar -
                currentNode->relatives[TSTNode::LOKID]->splitchar;
  int movingKid;
  shared_ptr<TSTNode> targetNode;
  if (deltaHi == deltaLo) {
    if (Math::random() < 0.5) {
      deltaHi++;
    } else {
      deltaLo++;
    }
  }
  if (deltaHi > deltaLo) {
    movingKid = TSTNode::HIKID;
    targetNode = currentNode->relatives[TSTNode::LOKID];
  } else {
    movingKid = TSTNode::LOKID;
    targetNode = currentNode->relatives[TSTNode::HIKID];
  }
  while (targetNode->relatives[movingKid] != nullptr) {
    targetNode = targetNode->relatives[movingKid];
  }
  targetNode->relatives[movingKid] = currentNode->relatives[movingKid];
  currentParent->relatives[childType] = targetNode;
  targetNode->relatives[TSTNode::PARENT] = currentParent;
  if (!lokidNull) {
    currentNode->relatives[TSTNode::LOKID].reset();
  }
  if (!hikidNull) {
    currentNode->relatives[TSTNode::HIKID].reset();
  }
  return currentParent;
}

any JaspellTernarySearchTrie::get(shared_ptr<std::wstring> key)
{
  shared_ptr<TSTNode> node = getNode(key);
  if (node == nullptr) {
    return nullptr;
  }
  return node->data;
}

optional<float> JaspellTernarySearchTrie::getAndIncrement(const wstring &key)
{
  wstring key2 = StringHelper::trim(key)->toLowerCase(locale);
  shared_ptr<TSTNode> node = getNode(key2);
  if (node == nullptr) {
    return nullopt;
  }
  optional<float> aux = any_cast<optional<float>>(node->data);
  if (!aux) {
    aux = optional<float>(1);
  } else {
    aux = optional<float>(aux.value() + 1);
  }
  put(key2, aux);
  return aux;
}

wstring JaspellTernarySearchTrie::getKey(shared_ptr<TSTNode> node)
{
  shared_ptr<StringBuilder> getKeyBuffer = make_shared<StringBuilder>();
  getKeyBuffer->setLength(0);
  getKeyBuffer->append(L"" + StringHelper::toString(node->splitchar));
  shared_ptr<TSTNode> currentNode;
  shared_ptr<TSTNode> lastNode;
  currentNode = node->relatives[TSTNode::PARENT];
  lastNode = node;
  while (currentNode != nullptr) {
    if (currentNode->relatives[TSTNode::EQKID] == lastNode) {
      getKeyBuffer->append(L"" +
                           StringHelper::toString(currentNode->splitchar));
    }
    lastNode = currentNode;
    currentNode = currentNode->relatives[TSTNode::PARENT];
  }
  getKeyBuffer->reverse();
  return getKeyBuffer->toString();
}

shared_ptr<TSTNode>
JaspellTernarySearchTrie::getNode(shared_ptr<std::wstring> key)
{
  return getNode(key, rootNode);
}

shared_ptr<TSTNode>
JaspellTernarySearchTrie::getNode(shared_ptr<std::wstring> key,
                                  shared_ptr<TSTNode> startNode)
{
  if (key == nullptr || startNode == nullptr || key->length() == 0) {
    return nullptr;
  }
  shared_ptr<TSTNode> currentNode = startNode;
  int charIndex = 0;
  while (true) {
    if (currentNode == nullptr) {
      return nullptr;
    }
    int charComp = compareCharsAlphabetically(key->charAt(charIndex),
                                              currentNode->splitchar);
    if (charComp == 0) {
      charIndex++;
      if (charIndex == key->length()) {
        return currentNode;
      }
      currentNode = currentNode->relatives[TSTNode::EQKID];
    } else if (charComp < 0) {
      currentNode = currentNode->relatives[TSTNode::LOKID];
    } else {
      currentNode = currentNode->relatives[TSTNode::HIKID];
    }
  }
}

shared_ptr<TSTNode> JaspellTernarySearchTrie::getOrCreateNode(
    shared_ptr<std::wstring> key) 
{
  if (key == nullptr) {
    throw make_shared<NullPointerException>(
        L"attempt to get or create node with null key");
  }
  if (key->length() == 0) {
    throw invalid_argument(
        L"attempt to get or create node with key of zero length");
  }
  if (rootNode == nullptr) {
    rootNode = make_shared<TSTNode>(key->charAt(0), nullptr);
  }
  shared_ptr<TSTNode> currentNode = rootNode;
  int charIndex = 0;
  while (true) {
    int charComp = compareCharsAlphabetically(key->charAt(charIndex),
                                              currentNode->splitchar);
    if (charComp == 0) {
      charIndex++;
      if (charIndex == key->length()) {
        return currentNode;
      }
      if (currentNode->relatives[TSTNode::EQKID] == nullptr) {
        currentNode->relatives[TSTNode::EQKID] =
            make_shared<TSTNode>(key->charAt(charIndex), currentNode);
      }
      currentNode = currentNode->relatives[TSTNode::EQKID];
    } else if (charComp < 0) {
      if (currentNode->relatives[TSTNode::LOKID] == nullptr) {
        currentNode->relatives[TSTNode::LOKID] =
            make_shared<TSTNode>(key->charAt(charIndex), currentNode);
      }
      currentNode = currentNode->relatives[TSTNode::LOKID];
    } else {
      if (currentNode->relatives[TSTNode::HIKID] == nullptr) {
        currentNode->relatives[TSTNode::HIKID] =
            make_shared<TSTNode>(key->charAt(charIndex), currentNode);
      }
      currentNode = currentNode->relatives[TSTNode::HIKID];
    }
  }
}

deque<wstring> JaspellTernarySearchTrie::matchAlmost(const wstring &key)
{
  return matchAlmost(key, defaultNumReturnValues);
}

deque<wstring>
JaspellTernarySearchTrie::matchAlmost(shared_ptr<std::wstring> key,
                                      int numReturnValues)
{
  return matchAlmostRecursion(rootNode, 0, matchAlmostDiff, key,
                              ((numReturnValues < 0) ? -1 : numReturnValues),
                              deque<wstring>(), false);
}

deque<wstring> JaspellTernarySearchTrie::matchAlmostRecursion(
    shared_ptr<TSTNode> currentNode, int charIndex, int d,
    shared_ptr<std::wstring> matchAlmostKey, int matchAlmostNumReturnValues,
    deque<wstring> &matchAlmostResult2, bool upTo)
{
  if ((currentNode == nullptr) ||
      (matchAlmostNumReturnValues != -1 &&
       matchAlmostResult2.size() >= matchAlmostNumReturnValues) ||
      (d < 0) || (charIndex >= matchAlmostKey->length())) {
    return matchAlmostResult2;
  }
  int charComp = compareCharsAlphabetically(matchAlmostKey->charAt(charIndex),
                                            currentNode->splitchar);
  deque<wstring> &matchAlmostResult = matchAlmostResult2;
  if ((d > 0) || (charComp < 0)) {
    matchAlmostResult = matchAlmostRecursion(
        currentNode->relatives[TSTNode::LOKID], charIndex, d, matchAlmostKey,
        matchAlmostNumReturnValues, matchAlmostResult, upTo);
  }
  int nextD = (charComp == 0) ? d : d - 1;
  bool cond = (upTo) ? (nextD >= 0) : (nextD == 0);
  if ((matchAlmostKey->length() == charIndex + 1) && cond &&
      (currentNode->data != nullptr)) {
    matchAlmostResult.push_back(getKey(currentNode));
  }
  matchAlmostResult = matchAlmostRecursion(
      currentNode->relatives[TSTNode::EQKID], charIndex + 1, nextD,
      matchAlmostKey, matchAlmostNumReturnValues, matchAlmostResult, upTo);
  if ((d > 0) || (charComp > 0)) {
    matchAlmostResult = matchAlmostRecursion(
        currentNode->relatives[TSTNode::HIKID], charIndex, d, matchAlmostKey,
        matchAlmostNumReturnValues, matchAlmostResult, upTo);
  }
  return matchAlmostResult;
}

deque<wstring> JaspellTernarySearchTrie::matchPrefix(const wstring &prefix)
{
  return matchPrefix(prefix, defaultNumReturnValues);
}

deque<wstring>
JaspellTernarySearchTrie::matchPrefix(shared_ptr<std::wstring> prefix,
                                      int numReturnValues)
{
  deque<wstring> sortKeysResult = deque<wstring>();
  shared_ptr<TSTNode> startNode = getNode(prefix);
  if (startNode == nullptr) {
    return sortKeysResult;
  }
  if (startNode->data != nullptr) {
    sortKeysResult.push_back(getKey(startNode));
  }
  return sortKeysRecursion(startNode->relatives[TSTNode::EQKID],
                           ((numReturnValues < 0) ? -1 : numReturnValues),
                           sortKeysResult);
}

int JaspellTernarySearchTrie::numDataNodes() { return numDataNodes(rootNode); }

int JaspellTernarySearchTrie::numDataNodes(shared_ptr<TSTNode> startingNode)
{
  return recursiveNodeCalculator(startingNode, true, 0);
}

int JaspellTernarySearchTrie::numNodes() { return numNodes(rootNode); }

int JaspellTernarySearchTrie::numNodes(shared_ptr<TSTNode> startingNode)
{
  return recursiveNodeCalculator(startingNode, false, 0);
}

void JaspellTernarySearchTrie::put(shared_ptr<std::wstring> key, any value)
{
  getOrCreateNode(key)->data = value;
}

int JaspellTernarySearchTrie::recursiveNodeCalculator(
    shared_ptr<TSTNode> currentNode, bool checkData, int numNodes2)
{
  if (currentNode == nullptr) {
    return numNodes2;
  }
  int numNodes = recursiveNodeCalculator(currentNode->relatives[TSTNode::LOKID],
                                         checkData, numNodes2);
  numNodes = recursiveNodeCalculator(currentNode->relatives[TSTNode::EQKID],
                                     checkData, numNodes);
  numNodes = recursiveNodeCalculator(currentNode->relatives[TSTNode::HIKID],
                                     checkData, numNodes);
  if (checkData) {
    if (currentNode->data != nullptr) {
      numNodes++;
    }
  } else {
    numNodes++;
  }
  return numNodes;
}

void JaspellTernarySearchTrie::remove(const wstring &key)
{
  deleteNode(getNode(StringHelper::trim(key)->toLowerCase(locale)));
}

void JaspellTernarySearchTrie::setMatchAlmostDiff(int diff)
{
  if (diff < 0) {
    matchAlmostDiff = 0;
  } else if (diff > 3) {
    matchAlmostDiff = 3;
  } else {
    matchAlmostDiff = diff;
  }
}

void JaspellTernarySearchTrie::setNumReturnValues(int num)
{
  defaultNumReturnValues = (num < 0) ? -1 : num;
}

deque<wstring>
JaspellTernarySearchTrie::sortKeys(shared_ptr<TSTNode> startNode,
                                   int numReturnValues)
{
  return sortKeysRecursion(startNode,
                           ((numReturnValues < 0) ? -1 : numReturnValues),
                           deque<wstring>());
}

deque<wstring>
JaspellTernarySearchTrie::sortKeysRecursion(shared_ptr<TSTNode> currentNode,
                                            int sortKeysNumReturnValues,
                                            deque<wstring> &sortKeysResult2)
{
  if (currentNode == nullptr) {
    return sortKeysResult2;
  }
  deque<wstring> sortKeysResult =
      sortKeysRecursion(currentNode->relatives[TSTNode::LOKID],
                        sortKeysNumReturnValues, sortKeysResult2);
  if (sortKeysNumReturnValues != -1 &&
      sortKeysResult.size() >= sortKeysNumReturnValues) {
    return sortKeysResult;
  }
  if (currentNode->data != nullptr) {
    sortKeysResult.push_back(getKey(currentNode));
  }
  sortKeysResult = sortKeysRecursion(currentNode->relatives[TSTNode::EQKID],
                                     sortKeysNumReturnValues, sortKeysResult);
  return sortKeysRecursion(currentNode->relatives[TSTNode::HIKID],
                           sortKeysNumReturnValues, sortKeysResult);
}

int64_t JaspellTernarySearchTrie::ramBytesUsed()
{
  int64_t mem = RamUsageEstimator::shallowSizeOf(shared_from_this());
  shared_ptr<TSTNode> *const root = getRoot();
  if (root != nullptr) {
    mem += root->ramBytesUsed();
  }
  return mem;
}
} // namespace org::apache::lucene::search::suggest::jaspell