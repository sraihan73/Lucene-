using namespace std;

#include "NumberDateFormat.h"

namespace org::apache::lucene::queryparser::flexible::standard::config
{

NumberDateFormat::NumberDateFormat(shared_ptr<DateFormat> dateFormat)
    : dateFormat(dateFormat)
{
}

shared_ptr<StringBuilder>
NumberDateFormat::format(double number, shared_ptr<StringBuilder> toAppendTo,
                         shared_ptr<FieldPosition> pos)
{
  return dateFormat->format(Date(static_cast<int64_t>(number)), toAppendTo,
                            pos);
}

shared_ptr<StringBuilder>
NumberDateFormat::format(int64_t number, shared_ptr<StringBuilder> toAppendTo,
                         shared_ptr<FieldPosition> pos)
{
  return dateFormat->format(Date(number), toAppendTo, pos);
}

shared_ptr<Number>
NumberDateFormat::parse(const wstring &source,
                        shared_ptr<ParsePosition> parsePosition)
{
  constexpr Date date = dateFormat->parse(source, parsePosition);
  return (date == nullptr) ? nullptr : date.getTime();
}

shared_ptr<StringBuilder>
NumberDateFormat::format(any number, shared_ptr<StringBuilder> toAppendTo,
                         shared_ptr<FieldPosition> pos)
{
  return dateFormat->format(number, toAppendTo, pos);
}
} // namespace org::apache::lucene::queryparser::flexible::standard::config