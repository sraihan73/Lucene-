using namespace std;

#include "Trie.h"
#include "Cell.h"
#include "Reduce.h"
#include "Row.h"

namespace org::egothor::stemmer
{

Trie::Trie(shared_ptr<DataInput> is) 
{
  forward = is->readBoolean();
  root = is->readInt();
  for (int i = is->readInt(); i > 0; i--) {
    cmds.push_back(is->readUTF());
  }
  for (int i = is->readInt(); i > 0; i--) {
    rows.push_back(make_shared<Row>(is));
  }
}

Trie::Trie(bool forward)
{
  rows.push_back(make_shared<Row>());
  root = 0;
  this->forward = forward;
}

Trie::Trie(bool forward, int root, deque<std::shared_ptr<std::wstring>> &cmds,
           deque<std::shared_ptr<Row>> &rows)
{
  this->rows = rows;
  this->cmds = cmds;
  this->root = root;
  this->forward = forward;
}

std::deque<std::shared_ptr<std::wstring>>
Trie::getAll(shared_ptr<std::wstring> key)
{
  std::deque<int> res(key->length());
  int resc = 0;
  shared_ptr<Row> now = getRow(root);
  int w;
  shared_ptr<StrEnum> e =
      make_shared<StrEnum>(shared_from_this(), key, forward);
  bool br = false;

  for (int i = 0; i < key->length() - 1; i++) {
    optional<wchar_t> ch = optional<wchar_t>(e->next());
    w = now->getCmd(ch);
    if (w >= 0) {
      int n = w;
      for (int j = 0; j < resc; j++) {
        if (n == res[j]) {
          n = -1;
          break;
        }
      }
      if (n >= 0) {
        res[resc++] = n;
      }
    }
    w = now->getRef(ch);
    if (w >= 0) {
      now = getRow(w);
    } else {
      br = true;
      break;
    }
  }
  if (br == false) {
    w = now->getCmd(optional<wchar_t>(e->next()));
    if (w >= 0) {
      int n = w;
      for (int j = 0; j < resc; j++) {
        if (n == res[j]) {
          n = -1;
          break;
        }
      }
      if (n >= 0) {
        res[resc++] = n;
      }
    }
  }

  if (resc < 1) {
    return nullptr;
  }
  std::deque<std::shared_ptr<std::wstring>> R(resc);
  for (int j = 0; j < resc; j++) {
    R[j] = cmds[res[j]];
  }
  return R;
}

int Trie::getCells()
{
  int size = 0;
  for (auto row : rows) {
    size += row->getCells();
  }
  return size;
}

int Trie::getCellsPnt()
{
  int size = 0;
  for (auto row : rows) {
    size += row->getCellsPnt();
  }
  return size;
}

int Trie::getCellsVal()
{
  int size = 0;
  for (auto row : rows) {
    size += row->getCellsVal();
  }
  return size;
}

shared_ptr<std::wstring> Trie::getFully(shared_ptr<std::wstring> key)
{
  shared_ptr<Row> now = getRow(root);
  int w;
  shared_ptr<Cell> c;
  int cmd = -1;
  shared_ptr<StrEnum> e =
      make_shared<StrEnum>(shared_from_this(), key, forward);
  optional<wchar_t> ch = nullopt;
  optional<wchar_t> aux = nullopt;

  for (int i = 0; i < key->length();) {
    ch = optional<wchar_t>(e->next());
    i++;

    c = now->at(ch);
    if (c == nullptr) {
      return nullptr;
    }

    cmd = c->cmd;

    for (int skip = c->skip; skip > 0; skip--) {
      if (i < key->length()) {
        aux = optional<wchar_t>(e->next());
      } else {
        return nullptr;
      }
      i++;
    }

    w = now->getRef(ch);
    if (w >= 0) {
      now = getRow(w);
    } else if (i < key->length()) {
      return nullptr;
    }
  }
  return (cmd == -1) ? nullptr : cmds[cmd];
}

shared_ptr<std::wstring> Trie::getLastOnPath(shared_ptr<std::wstring> key)
{
  shared_ptr<Row> now = getRow(root);
  int w;
  shared_ptr<std::wstring> last = nullptr;
  shared_ptr<StrEnum> e =
      make_shared<StrEnum>(shared_from_this(), key, forward);

  for (int i = 0; i < key->length() - 1; i++) {
    optional<wchar_t> ch = optional<wchar_t>(e->next());
    w = now->getCmd(ch);
    if (w >= 0) {
      last = cmds[w];
    }
    w = now->getRef(ch);
    if (w >= 0) {
      now = getRow(w);
    } else {
      return last;
    }
  }
  w = now->getCmd(optional<wchar_t>(e->next()));
  return (w >= 0) ? cmds[w] : last;
}

shared_ptr<Row> Trie::getRow(int index)
{
  if (index < 0 || index >= rows.size()) {
    return nullptr;
  }
  return rows[index];
}

void Trie::store(shared_ptr<DataOutput> os) 
{
  os->writeBoolean(forward);
  os->writeInt(root);
  os->writeInt(cmds.size());
  for (auto cmd : cmds) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    os->writeUTF(cmd->toString());
  }

  os->writeInt(rows.size());
  for (auto row : rows) {
    row->store(os);
  }
}

void Trie::add(shared_ptr<std::wstring> key, shared_ptr<std::wstring> cmd)
{
  if (key == nullptr || cmd == nullptr) {
    return;
  }
  if (cmd->length() == 0) {
    return;
  }
  int id_cmd = cmds.find(cmd);
  if (id_cmd == -1) {
    id_cmd = cmds.size();
    cmds.push_back(cmd);
  }

  int node = root;
  shared_ptr<Row> r = getRow(node);

  shared_ptr<StrEnum> e =
      make_shared<StrEnum>(shared_from_this(), key, forward);

  for (int i = 0; i < e->length() - 1; i++) {
    optional<wchar_t> ch = optional<wchar_t>(e->next());
    node = r->getRef(ch);
    if (node >= 0) {
      r = getRow(node);
    } else {
      node = rows.size();
      shared_ptr<Row> n;
      rows.push_back(n = make_shared<Row>());
      r->setRef(ch, node);
      r = n;
    }
  }
  r->setCmd(optional<wchar_t>(e->next()), id_cmd);
}

shared_ptr<Trie> Trie::reduce(shared_ptr<Reduce> by)
{
  return by->optimize(shared_from_this());
}

void Trie::printInfo(shared_ptr<PrintStream> out,
                     shared_ptr<std::wstring> prefix)
{
  out->println(prefix + L"nds " + rows.size() + L" cmds " + cmds.size() +
               L" cells " + to_wstring(getCells()) + L" valcells " +
               to_wstring(getCellsVal()) + L" pntcells " +
               to_wstring(getCellsPnt()));
}

Trie::StrEnum::StrEnum(shared_ptr<Trie> outerInstance,
                       shared_ptr<std::wstring> s, bool up)
    : outerInstance(outerInstance)
{
  this->s = s;
  if (up) {
    from = 0;
    by = 1;
  } else {
    from = s->length() - 1;
    by = -1;
  }
}

int Trie::StrEnum::length() { return s->length(); }

wchar_t Trie::StrEnum::next()
{
  wchar_t ch = s->charAt(from);
  from += by;
  return ch;
}
} // namespace org::egothor::stemmer