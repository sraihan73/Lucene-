using namespace std;

#include "Consts.h"

namespace org::apache::lucene::facet::taxonomy::directory
{
using BytesRef = org::apache::lucene::util::BytesRef;
const wstring Consts::FULL = L"$full_path$";
const wstring Consts::FIELD_PAYLOADS = L"$payloads$";
const wstring Consts::PAYLOAD_PARENT = L"p";
const shared_ptr<org::apache::lucene::util::BytesRef>
    Consts::PAYLOAD_PARENT_BYTES_REF =
        make_shared<org::apache::lucene::util::BytesRef>(PAYLOAD_PARENT);
} // namespace org::apache::lucene::facet::taxonomy::directory