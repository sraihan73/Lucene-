using namespace std;

#include "Row.h"
#include "Cell.h"

namespace org::egothor::stemmer
{

Row::Row(shared_ptr<DataInput> is) 
{
  for (int i = is->readInt(); i > 0; i--) {
    wchar_t ch = is->readChar();
    shared_ptr<Cell> c = make_shared<Cell>();
    c->cmd = is->readInt();
    c->cnt = is->readInt();
    c->ref = is->readInt();
    c->skip = is->readInt();
    cells.emplace(ch, c);
  }
}

Row::Row() {}

Row::Row(shared_ptr<Row> old) { cells = old->cells; }

void Row::setCmd(optional<wchar_t> &way, int cmd)
{
  shared_ptr<Cell> c = at(way);
  if (c == nullptr) {
    c = make_shared<Cell>();
    c->cmd = cmd;
    cells.emplace(way, c);
  } else {
    c->cmd = cmd;
  }
  c->cnt = (cmd >= 0) ? 1 : 0;
}

void Row::setRef(optional<wchar_t> &way, int ref)
{
  shared_ptr<Cell> c = at(way);
  if (c == nullptr) {
    c = make_shared<Cell>();
    c->ref = ref;
    cells.emplace(way, c);
  } else {
    c->ref = ref;
  }
}

int Row::getCells()
{
  Iterator<wchar_t> i = cells.keySet().begin();
  int size = 0;
  for (; i->hasNext();) {
    optional<wchar_t> c = i->next();
    shared_ptr<Cell> e = at(c);
    if (e->cmd >= 0 || e->ref >= 0) {
      size++;
    }
  }
  return size;
}

int Row::getCellsPnt()
{
  Iterator<wchar_t> i = cells.keySet().begin();
  int size = 0;
  for (; i->hasNext();) {
    optional<wchar_t> c = i->next();
    shared_ptr<Cell> e = at(c);
    if (e->ref >= 0) {
      size++;
    }
  }
  return size;
}

int Row::getCellsVal()
{
  Iterator<wchar_t> i = cells.keySet().begin();
  int size = 0;
  for (; i->hasNext();) {
    optional<wchar_t> c = i->next();
    shared_ptr<Cell> e = at(c);
    if (e->cmd >= 0) {
      size++;
    }
  }
  return size;
}

int Row::getCmd(optional<wchar_t> &way)
{
  shared_ptr<Cell> c = at(way);
  return (c == nullptr) ? -1 : c->cmd;
}

int Row::getCnt(optional<wchar_t> &way)
{
  shared_ptr<Cell> c = at(way);
  return (c == nullptr) ? -1 : c->cnt;
}

int Row::getRef(optional<wchar_t> &way)
{
  shared_ptr<Cell> c = at(way);
  return (c == nullptr) ? -1 : c->ref;
}

void Row::store(shared_ptr<DataOutput> os) 
{
  os->writeInt(cells.size());
  Iterator<wchar_t> i = cells.keySet().begin();
  for (; i->hasNext();) {
    optional<wchar_t> c = i->next();
    shared_ptr<Cell> e = at(c);
    if (e->cmd < 0 && e->ref < 0) {
      continue;
    }

    os->writeChar(c.value());
    os->writeInt(e->cmd);
    os->writeInt(e->cnt);
    os->writeInt(e->ref);
    os->writeInt(e->skip);
  }
}

int Row::uniformCmd(bool eqSkip)
{
  Iterator<std::shared_ptr<Cell>> i = cells.values().begin();
  int ret = -1;
  uniformCnt = 1;
  uniformSkip = 0;
  for (; i->hasNext();) {
    shared_ptr<Cell> c = i->next();
    if (c->ref >= 0) {
      return -1;
    }
    if (c->cmd >= 0) {
      if (ret < 0) {
        ret = c->cmd;
        uniformSkip = c->skip;
      } else if (ret == c->cmd) {
        if (eqSkip) {
          if (uniformSkip == c->skip) {
            uniformCnt++;
          } else {
            return -1;
          }
        } else {
          uniformCnt++;
        }
      } else {
        return -1;
      }
    }
  }
  return ret;
}

void Row::print(shared_ptr<PrintStream> out)
{
  for (shared_ptr<Iterator<wchar_t>> i = cells.keySet().begin();
       i->hasNext();) {
    optional<wchar_t> ch = i->next();
    shared_ptr<Cell> c = at(ch);
    out->print(L"[" + ch + L":" + c + L"]");
  }
  out->println();
}

shared_ptr<Cell> Row::at(optional<wchar_t> &index) { return cells[index]; }
} // namespace org::egothor::stemmer