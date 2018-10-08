using namespace std;

#include "SimpleTransLog.h"

namespace org::apache::lucene::replicator::nrt
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using DataInput = org::apache::lucene::store::DataInput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;

SimpleTransLog::SimpleTransLog(shared_ptr<Path> path) 
    : channel(FileChannel::open(path, StandardOpenOption::READ,
                                StandardOpenOption::WRITE,
                                StandardOpenOption::CREATE_NEW))
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t SimpleTransLog::getNextLocation() 
{
  return channel->position();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
SimpleTransLog::addDocument(const wstring &id,
                            shared_ptr<Document> doc) 
{
  assert(buffer->getFilePointer() == 0);
  buffer->writeByte(OP_ADD_DOCUMENT);
  encode(id, doc);
  return flushBuffer();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t
SimpleTransLog::updateDocument(const wstring &id,
                               shared_ptr<Document> doc) 
{
  assert(buffer->getFilePointer() == 0);
  buffer->writeByte(OP_UPDATE_DOCUMENT);
  encode(id, doc);
  return flushBuffer();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t SimpleTransLog::deleteDocuments(const wstring &id) 
{
  assert(buffer->getFilePointer() == 0);
  buffer->writeByte(OP_DELETE_DOCUMENTS);
  buffer->writeString(id);
  return flushBuffer();
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t SimpleTransLog::flushBuffer() 
{
  int64_t pos = channel->position();
  int len = static_cast<int>(buffer->getFilePointer());
  std::deque<char> bytes(len);
  buffer->writeTo(bytes, 0);
  buffer->reset();

  intBuffer[0] = static_cast<char>(len >> 24);
  intBuffer[1] = static_cast<char>(len >> 16);
  intBuffer[2] = static_cast<char>(len >> 8);
  intBuffer[3] = static_cast<char>(len);
  intByteBuffer->limit(4);
  intByteBuffer->position(0);

  writeBytesToChannel(intByteBuffer);
  writeBytesToChannel(ByteBuffer::wrap(bytes));

  return pos;
}

void SimpleTransLog::writeBytesToChannel(shared_ptr<ByteBuffer> src) throw(
    IOException)
{
  int left = src->limit();
  while (left != 0) {
    left -= channel->write(src);
  }
}

void SimpleTransLog::readBytesFromChannel(
    int64_t pos, shared_ptr<ByteBuffer> dest) 
{
  int left = dest->limit() - dest->position();
  int64_t end = pos + left;
  while (pos < end) {
    int inc = channel->read(dest, pos);
    if (inc < 0) {
      // C++ TODO: The following line could not be converted:
      throw java.io.EOFException();
    }
    pos += inc;
  }
}

void SimpleTransLog::replay(shared_ptr<NodeProcess> primary, int64_t start,
                            int64_t end) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Connection c = new
  // Connection(primary.tcpPort))
  {
    Connection c = Connection(primary->tcpPort);
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING);
    std::deque<char> intBuffer(4);
    shared_ptr<ByteBuffer> intByteBuffer = ByteBuffer::wrap(intBuffer);
    shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>();

    int64_t pos = start;
    while (pos < end) {
      intByteBuffer->position(0);
      intByteBuffer->limit(4);
      readBytesFromChannel(pos, intByteBuffer);
      pos += 4;
      int len = ((intBuffer[0] & 0xff) << 24) | (intBuffer[1] & 0xff) << 16 |
                (intBuffer[2] & 0xff) << 8 | (intBuffer[3] & 0xff);

      std::deque<char> bytes(len);
      readBytesFromChannel(pos, ByteBuffer::wrap(bytes));
      pos += len;

      in_->reset(bytes);

      char op = in_->readByte();
      // System.out.println("xlog: replay op=" + op);
      switch (op) {
      case 0:
        // We replay add as update:
        replayAddDocument(c, primary, in_);
        break;

      case 1:
        // We replay add as update:
        replayAddDocument(c, primary, in_);
        break;

      case 2:
        replayDeleteDocuments(c, primary, in_);
        break;

      default:
        throw make_shared<CorruptIndexException>(
            L"invalid operation " + to_wstring(op), in_);
      }
    }
    assert(pos == end);
    // System.out.println("xlog: done replay");
    c->out->writeByte(SimplePrimaryNode::CMD_INDEXING_DONE);
    c->flush();
    // System.out.println("xlog: done flush");
    c->in_->readByte();
    // System.out.println("xlog: done readByte");
  }
}

void SimpleTransLog::replayAddDocument(
    shared_ptr<Connection> c, shared_ptr<NodeProcess> primary,
    shared_ptr<DataInput> in_) 
{
  wstring id = in_->readString();

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"docid", id, Field::Store::YES));

  wstring title = readNullableString(in_);
  if (title != L"") {
    doc->push_back(make_shared<StringField>(L"title", title, Field::Store::NO));
    doc->push_back(
        make_shared<TextField>(L"titleTokenized", title, Field::Store::NO));
  }
  wstring body = readNullableString(in_);
  if (body != L"") {
    doc->push_back(make_shared<TextField>(L"body", body, Field::Store::NO));
  }
  wstring marker = readNullableString(in_);
  if (marker != L"") {
    // TestStressNRTReplication.message("xlog: replay marker=" + id);
    doc->push_back(
        make_shared<StringField>(L"marker", marker, Field::Store::YES));
  }

  // For both add and update originally, we use updateDocument to replay,
  // because the doc could in fact already be in the index:
  // nocomit what if this fails?
  primary->addOrUpdateDocument(c, doc, false);
}

void SimpleTransLog::replayDeleteDocuments(
    shared_ptr<Connection> c, shared_ptr<NodeProcess> primary,
    shared_ptr<DataInput> in_) 
{
  wstring id = in_->readString();
  // nocomit what if this fails?
  primary->deleteDocument(c, id);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimpleTransLog::encode(const wstring &id,
                            shared_ptr<Document> doc) 
{
  assert((id == doc[L"docid"], L"id=" + id + L" vs docid=" + doc[L"docid"]));
  buffer->writeString(id);
  writeNullableString(doc[L"title"]);
  writeNullableString(doc[L"body"]);
  writeNullableString(doc[L"marker"]);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void SimpleTransLog::writeNullableString(const wstring &s) 
{
  if (s == L"") {
    buffer->writeByte(static_cast<char>(0));
  } else {
    buffer->writeByte(static_cast<char>(1));
    buffer->writeString(s);
  }
}

wstring
SimpleTransLog::readNullableString(shared_ptr<DataInput> in_) 
{
  char b = in_->readByte();
  if (b == 0) {
    return L"";
  } else if (b == 1) {
    return in_->readString();
  } else {
    throw make_shared<CorruptIndexException>(
        L"invalid string lead byte " + to_wstring(b), in_);
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
SimpleTransLog::~SimpleTransLog() { channel->close(); }
} // namespace org::apache::lucene::replicator::nrt