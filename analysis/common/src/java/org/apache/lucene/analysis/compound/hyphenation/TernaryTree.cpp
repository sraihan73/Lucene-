using namespace std;

#include "TernaryTree.h"
#include "CharVector.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{

TernaryTree::TernaryTree() { init(); }

void TernaryTree::init()
{
  root = 0;
  freenode = 1;
  length = 0;
  lo = std::deque<wchar_t>(BLOCK_SIZE);
  hi = std::deque<wchar_t>(BLOCK_SIZE);
  eq = std::deque<wchar_t>(BLOCK_SIZE);
  sc = std::deque<wchar_t>(BLOCK_SIZE);
  kv = make_shared<CharVector>();
}

void TernaryTree::insert(const wstring &key, wchar_t val)
{
  // make sure we have enough room in the arrays
  int len = key.length() + 1; // maximum number of nodes that may be generated
  if (freenode + len > eq.size()) {
    redimNodeArrays(eq.size() + BLOCK_SIZE);
  }
  std::deque<wchar_t> strkey(len--);
  key.getChars(0, len, strkey, 0);
  strkey[len] = 0;
  root = insert(root, strkey, 0, val);
}

void TernaryTree::insert(std::deque<wchar_t> &key, int start, wchar_t val)
{
  int len = strlen(key) + 1;
  if (freenode + len > eq.size()) {
    redimNodeArrays(eq.size() + BLOCK_SIZE);
  }
  root = insert(root, key, start, val);
}

wchar_t TernaryTree::insert(wchar_t p, std::deque<wchar_t> &key, int start,
                            wchar_t val)
{
  int len = strlen(key, start);
  if (p == 0) {
    // this means there is no branch, this node will start a new branch.
    // Instead of doing that, we store the key somewhere else and create
    // only one node with a pointer to the key
    p = freenode++;
    eq[p] = val; // holds data
    length++;
    hi[p] = 0;
    if (len > 0) {
      sc[p] = 0xFFFF; // indicates branch is compressed
      lo[p] = static_cast<wchar_t>(
          kv->alloc(len + 1)); // use 'lo' to hold pointer to key
      strcpy(kv->getArray(), lo[p], key, start);
    } else {
      sc[p] = 0;
      lo[p] = 0;
    }
    return p;
  }

  if (sc[p] == 0xFFFF) {
    // branch is compressed: need to decompress
    // this will generate garbage in the external key array
    // but we can do some garbage collection later
    wchar_t pp = freenode++;
    lo[pp] = lo[p]; // previous pointer to key
    eq[pp] = eq[p]; // previous pointer to data
    lo[p] = 0;
    if (len > 0) {
      sc[p] = kv->get(lo[pp]);
      eq[p] = pp;
      lo[pp]++;
      if (kv->get(lo[pp]) == 0) {
        // key completly decompressed leaving garbage in key array
        lo[pp] = 0;
        sc[pp] = 0;
        hi[pp] = 0;
      } else {
        // we only got first char of key, rest is still there
        sc[pp] = 0xFFFF;
      }
    } else {
      // In this case we can save a node by swapping the new node
      // with the compressed node
      sc[pp] = 0xFFFF;
      hi[p] = pp;
      sc[p] = 0;
      eq[p] = val;
      length++;
      return p;
    }
  }
  wchar_t s = key[start];
  if (s < sc[p]) {
    lo[p] = insert(lo[p], key, start, val);
  } else if (s == sc[p]) {
    if (s != 0) {
      eq[p] = insert(eq[p], key, start + 1, val);
    } else {
      // key already in tree, overwrite data
      eq[p] = val;
    }
  } else {
    hi[p] = insert(hi[p], key, start, val);
  }
  return p;
}

int TernaryTree::strcmp(std::deque<wchar_t> &a, int startA,
                        std::deque<wchar_t> &b, int startB)
{
  for (; a[startA] == b[startB]; startA++, startB++) {
    if (a[startA] == 0) {
      return 0;
    }
  }
  return a[startA] - b[startB];
}

int TernaryTree::strcmp(const wstring &str, std::deque<wchar_t> &a, int start)
{
  int i, d, len = str.length();
  for (i = 0; i < len; i++) {
    d = static_cast<int>(str[i]) - a[start + i];
    if (d != 0) {
      return d;
    }
    if (a[start + i] == 0) {
      return d;
    }
  }
  if (a[start + i] != 0) {
    return -a[start + i];
  }
  return 0;
}

void TernaryTree::strcpy(std::deque<wchar_t> &dst, int di,
                         std::deque<wchar_t> &src, int si)
{
  while (src[si] != 0) {
    dst[di++] = src[si++];
  }
  dst[di] = 0;
}

int TernaryTree::strlen(std::deque<wchar_t> &a, int start)
{
  int len = 0;
  for (int i = start; i < a.size() && a[i] != 0; i++) {
    len++;
  }
  return len;
}

int TernaryTree::strlen(std::deque<wchar_t> &a) { return strlen(a, 0); }

int TernaryTree::find(const wstring &key)
{
  int len = key.length();
  std::deque<wchar_t> strkey(len + 1);
  key.getChars(0, len, strkey, 0);
  strkey[len] = 0;

  return find(strkey, 0);
}

int TernaryTree::find(std::deque<wchar_t> &key, int start)
{
  int d;
  wchar_t p = root;
  int i = start;
  wchar_t c;

  while (p != 0) {
    if (sc[p] == 0xFFFF) {
      if (strcmp(key, i, kv->getArray(), lo[p]) == 0) {
        return eq[p];
      } else {
        return -1;
      }
    }
    c = key[i];
    d = c - sc[p];
    if (d == 0) {
      if (c == 0) {
        return eq[p];
      }
      i++;
      p = eq[p];
    } else if (d < 0) {
      p = lo[p];
    } else {
      p = hi[p];
    }
  }
  return -1;
}

bool TernaryTree::knows(const wstring &key) { return (find(key) >= 0); }

void TernaryTree::redimNodeArrays(int newsize)
{
  int len = newsize < lo.size() ? newsize : lo.size();
  std::deque<wchar_t> na(newsize);
  System::arraycopy(lo, 0, na, 0, len);
  lo = na;
  na = std::deque<wchar_t>(newsize);
  System::arraycopy(hi, 0, na, 0, len);
  hi = na;
  na = std::deque<wchar_t>(newsize);
  System::arraycopy(eq, 0, na, 0, len);
  eq = na;
  na = std::deque<wchar_t>(newsize);
  System::arraycopy(sc, 0, na, 0, len);
  sc = na;
}

int TernaryTree::size() { return length; }

shared_ptr<TernaryTree> TernaryTree::clone()
{
  shared_ptr<TernaryTree> t = make_shared<TernaryTree>();
  t->lo = this->lo.clone();
  t->hi = this->hi.clone();
  t->eq = this->eq.clone();
  t->sc = this->sc.clone();
  t->kv = this->kv->clone();
  t->root = this->root;
  t->freenode = this->freenode;
  t->length = this->length;

  return t;
}

void TernaryTree::insertBalanced(std::deque<wstring> &k,
                                 std::deque<wchar_t> &v, int offset, int n)
{
  int m;
  if (n < 1) {
    return;
  }
  m = n >> 1;

  insert(k[m + offset], v[m + offset]);
  insertBalanced(k, v, offset, m);

  insertBalanced(k, v, offset + m + 1, n - m - 1);
}

void TernaryTree::balance()
{
  // System.out.print("Before root splitchar = ");
  // System.out.println(sc[root]);

  int i = 0, n = length;
  std::deque<wstring> k(n);
  std::deque<wchar_t> v(n);
  Iterator iter = make_shared<Iterator>(shared_from_this());
  while (iter->hasMoreElements()) {
    v[i] = iter->getValue();
    k[i++] = iter->nextElement();
  }
  init();
  insertBalanced(k, v, 0, n);

  // With uniform letter distribution sc[root] should be around 'm'
  // System.out.print("After root splitchar = ");
  // System.out.println(sc[root]);
}

void TernaryTree::trimToSize()
{
  // first balance the tree for best performance
  balance();

  // redimension the node arrays
  redimNodeArrays(freenode);

  // ok, compact kv array
  shared_ptr<CharVector> kx = make_shared<CharVector>();
  kx->alloc(1);
  shared_ptr<TernaryTree> map_obj = make_shared<TernaryTree>();
  compact(kx, map_obj, root);
  kv = kx;
  kv->trimToSize();
}

void TernaryTree::compact(shared_ptr<CharVector> kx,
                          shared_ptr<TernaryTree> map_obj, wchar_t p)
{
  int k;
  if (p == 0) {
    return;
  }
  if (sc[p] == 0xFFFF) {
    k = map_obj->find(kv->getArray(), lo[p]);
    if (k < 0) {
      k = kx->alloc(strlen(kv->getArray(), lo[p]) + 1);
      strcpy(kx->getArray(), k, kv->getArray(), lo[p]);
      map_obj->insert(kx->getArray(), k, static_cast<wchar_t>(k));
    }
    lo[p] = static_cast<wchar_t>(k);
  } else {
    compact(kx, map_obj, lo[p]);
    if (sc[p] != 0) {
      compact(kx, map_obj, eq[p]);
    }
    compact(kx, map_obj, hi[p]);
  }
}

shared_ptr<java::util::Iterator<wstring>> TernaryTree::keys()
{
  return make_shared<Iterator>(shared_from_this());
}

TernaryTree::Iterator::Item::Item(
    shared_ptr<TernaryTree::Iterator> outerInstance)
    : outerInstance(outerInstance)
{
  parent = 0;
  child = 0;
}

TernaryTree::Iterator::Item::Item(
    shared_ptr<TernaryTree::Iterator> outerInstance, wchar_t p, wchar_t c)
    : outerInstance(outerInstance)
{
  parent = p;
  child = c;
}

shared_ptr<Item> TernaryTree::Iterator::Item::clone()
{
  return make_shared<Item>(outerInstance, parent, child);
}

TernaryTree::Iterator::Iterator(shared_ptr<TernaryTree> outerInstance)
    : outerInstance(outerInstance)
{
  cur = -1;
  ns = stack<std::shared_ptr<Item>>();
  ks = make_shared<StringBuilder>();
  rewind();
}

void TernaryTree::Iterator::rewind()
{
  ns.removeAllElements();
  ks->setLength(0);
  cur = outerInstance->root;
  run();
}

wstring TernaryTree::Iterator::nextElement()
{
  wstring res = curkey;
  cur = up();
  run();
  return res;
}

wchar_t TernaryTree::Iterator::getValue()
{
  if (cur >= 0) {
    return outerInstance->eq[cur];
  }
  return 0;
}

bool TernaryTree::Iterator::hasMoreElements() { return (cur != -1); }

int TernaryTree::Iterator::up()
{
  shared_ptr<Item> i = make_shared<Item>(shared_from_this());
  int res = 0;

  if (ns.empty()) {
    return -1;
  }

  if (cur != 0 && outerInstance->sc[cur] == 0) {
    return outerInstance->lo[cur];
  }

  bool climb = true;

  while (climb) {
    i = ns.pop();
    i->child++;
    switch (i->child) {
    case 1:
      if (outerInstance->sc[i->parent] != 0) {
        res = outerInstance->eq[i->parent];
        ns.push(i->clone());
        ks->append(outerInstance->sc[i->parent]);
      } else {
        i->child++;
        ns.push(i->clone());
        res = outerInstance->hi[i->parent];
      }
      climb = false;
      break;

    case 2:
      res = outerInstance->hi[i->parent];
      ns.push(i->clone());
      if (ks->length() > 0) {
        ks->setLength(ks->length() - 1); // pop
      }
      climb = false;
      break;

    default:
      if (ns.empty()) {
        return -1;
      }
      climb = true;
      break;
    }
  }
  return res;
}

int TernaryTree::Iterator::run()
{
  if (cur == -1) {
    return -1;
  }

  bool leaf = false;
  while (true) {
    // first go down on low branch until leaf or compressed branch
    while (cur != 0) {
      if (outerInstance->sc[cur] == 0xFFFF) {
        leaf = true;
        break;
      }
      ns.push(make_shared<Item>(shared_from_this(), static_cast<wchar_t>(cur),
                                L'\u0000'));
      if (outerInstance->sc[cur] == 0) {
        leaf = true;
        break;
      }
      cur = outerInstance->lo[cur];
    }
    if (leaf) {
      break;
    }
    // nothing found, go up one node and try again
    cur = up();
    if (cur == -1) {
      return -1;
    }
  }
  // The current node should be a data node and
  // the key should be in the key stack (at least partially)
  shared_ptr<StringBuilder> buf = make_shared<StringBuilder>(ks->toString());
  if (outerInstance->sc[cur] == 0xFFFF) {
    int p = outerInstance->lo[cur];
    while (outerInstance->kv->get(p) != 0) {
      buf->append(outerInstance->kv->get(p++));
    }
  }
  curkey = buf->toString();
  return 0;
}

void TernaryTree::printStats(shared_ptr<PrintStream> out)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Number of keys = " + Integer::toString(length));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Node count = " + Integer::toString(freenode));
  // System.out.println("Array length = " + Integer.toString(eq.length));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Key Array length = " + Integer::toString(kv->length()));

  /*
   * for(int i=0; i<kv.length(); i++) if ( kv.get(i) != 0 )
   * System.out.print(kv.get(i)); else System.out.println("");
   * System.out.println("Keys:"); for(Enumeration enum = keys();
   * enum.hasMoreElements(); ) System.out.println(enum.nextElement());
   */
}
} // namespace org::apache::lucene::analysis::compound::hyphenation