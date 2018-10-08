using namespace std;

#include "ArmenianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    ArmenianStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ArmenianStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"\u0580\u0578\u0580\u0564",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u0580\u0578\u0580\u0564", 0, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C\u056B", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056F\u056B", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0580\u0561\u056F",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0572", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056F\u0561\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0580\u0561\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0576", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u056F\u0565\u0576",
                                                    8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u0565\u0576",
                                                    8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0580\u0567\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0576", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0563\u056B\u0576", 12,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057E\u056B\u0576",
                                                    12, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056C\u0561\u0575\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0578\u0582\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057A\u0565\u057D", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u057E", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u057F", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u057E\u0565\u057F",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056F\u0578\u057F", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0562\u0561\u0580", -1,
                                                    1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ArmenianStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"\u0561", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0561", 0, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u0561", 0, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0565", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0580\u056B",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u056B", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u056B", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0565\u0581\u056B",
                                                    6, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0568\u0561\u056C", 8, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0561\u056C",
                                                    8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0576\u0561\u056C",
                                                    8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0576\u0561\u056C", 8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u056C", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0568\u0565\u056C", 13,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u056C", 13,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u0576\u0565\u056C",
                                                    15, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u0581\u0576\u0565\u056C", 16, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0579\u0565\u056C", 13,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0565\u056C", 13,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u057E\u0565\u056C", 19, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u0581\u057E\u0565\u056C", 19, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057F\u0565\u056C", 13,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u057F\u0565\u056C",
                                                    22, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057F\u0565\u056C",
                                                    22, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u056F\u0578\u057F\u0565\u056C", 24, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u056E", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0574", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0578\u0582\u0574",
                                                    27, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u0561\u0576", 29,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0561\u0576",
                                                    30, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0580\u056B\u0576", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u056B\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u056B\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u057E\u0565\u0581\u056B\u0576", 34, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C\u056B\u057D",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u056C\u056B\u057D",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u057E", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0561\u057E",
                                                    38, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u0561\u057E",
                                                    38, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C\u0578\u057E",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u056C\u0578\u057E",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0580", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0561\u0580",
                                                    43, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u0561\u0580",
                                                    43, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0580\u056B\u0580", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u056B\u0580",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u056B\u0580",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u057E\u0565\u0581\u056B\u0580", 48, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0580\u0565\u0581", 51, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u056C\u0578\u0582\u0581", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u056C\u0578\u0582\u0581", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C\u0578\u0582",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u056C\u0578\u0582",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0584", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u0561\u0584", 57,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u0561\u0584",
                                                    58, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0580\u056B\u0584", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0581\u056B\u0584",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0581\u056B\u0584",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u057E\u0565\u0581\u056B\u0584", 62, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0584", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u0561\u0576\u0584",
                                                    64, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0561\u0576\u0584", 65, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u0580\u056B\u0576\u0584", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0581\u056B\u0576\u0584", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u0581\u056B\u0576\u0584", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u057E\u0565\u0581\u056B\u0576\u0584", 69, 1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ArmenianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0580\u0564", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0575\u0569",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0570\u056B",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u056B", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u056C", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056F", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0575\u0561\u056F", 5, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0561\u056F",
                                                    5, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u056F", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u056F", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057A\u0561\u0576", 10,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057D\u057F\u0561\u0576",
                                                    10, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0580\u0561\u0576",
                                                    10, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0572\u0567\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0575\u0578\u0582\u0576",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0578\u0582\u0576", 15, 1, L"",
            methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056E\u0578", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0579", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u057D", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u057D\u057F",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0563\u0561\u0580", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0578\u0580", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u057E\u0578\u0580",
                                                    22, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0581", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0585\u0581",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0584", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0579\u0565\u0584", 27,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0584", 27, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u056C\u056B\u0584",
                                                    29, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u056B\u0584",
                                                    29, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u056E\u0584",
                                                    27, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0575\u0584",
                                                    27, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0576\u0584", 27,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0576\u0584", 27,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0576\u0584",
                                                    27, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0574\u0578\u0582\u0576\u0584", 36, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0579\u0584", 27,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0580\u0584", 27,
                                                    1, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    ArmenianStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"\u057D\u0561", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0574\u0562", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0564", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0564", 3, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0561\u0576\u0564", 4, 1, L"",
            methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u0576\u0564",
                                                    4, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057B\u0564", 3, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u0564", 3, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u0580\u0564",
                                                    8, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0564", 3, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0568", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0568", 11,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0561\u0576\u0568", 12, 1, L"",
            methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u0576\u0568",
                                                    12, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057B\u0568", 11,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u0568", 11,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u0580\u0568",
                                                    16, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u056B", 18, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u056B", 18,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u0580\u056B",
                                                    20, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0561\u0576\u0578\u0582\u0574", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0565\u0580\u0578\u0582\u0574", -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0576\u0565\u0580\u0578\u0582\u0574", 23, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576", 25, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0561\u0576", 26, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u0576", 26,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0576", 25, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u056B\u0576",
                                                    29, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0576\u0565\u0580\u056B\u0576", 30, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0561\u0576\u0576", 25, 1, L"",
            methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u0576", 25,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u0580\u0576",
                                                    33, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0576", 25,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057B", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0578\u0582\u0569\u0575\u0561\u0576\u057D", -1, 1, L"",
            methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0561\u0576\u057D",
                                                    -1, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057B\u057D", -1,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057E", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0561\u0576\u0578\u057E",
                                                    40, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u0578\u057E", 40,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u0578\u057E",
                                                    40, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0576\u0565\u0580\u0578\u057E", 43, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0576\u0565\u0580", 45,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u056B\u0581", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u057E\u0561\u0576\u056B\u0581", 48, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u057B\u056B\u0581",
                                                    48, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u057E\u056B\u0581", 48,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0565\u0580\u056B\u0581",
                                                    48, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(
            L"\u0576\u0565\u0580\u056B\u0581", 52, 1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0581\u056B\u0581", 48,
                                                    1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0581", 47, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0578\u0582\u0581", 47,
                                                    1, L"", methodObject)};
std::deque<wchar_t> const ArmenianStemmer::g_v = {209, 4, 128, 0, 18};

void ArmenianStemmer::copy_from(shared_ptr<ArmenianStemmer> other)
{
  I_p2 = other->I_p2;
  I_pV = other->I_pV;
  SnowballProgram::copy_from(other);
}

bool ArmenianStemmer::r_mark_regions()
{
  int v_1;
  // (, line 58
  I_pV = limit;
  I_p2 = limit;
  // do, line 62
  v_1 = cursor;
  do {
    // (, line 62
    // gopast, line 63
    while (true) {
      do {
        if (!(in_grouping(g_v, 1377, 1413))) {
          goto lab2Break;
        }
        goto golab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab1Continue:;
    }
  golab1Break:
    // setmark pV, line 63
    I_pV = cursor;
    // gopast, line 63
    while (true) {
      do {
        if (!(out_grouping(g_v, 1377, 1413))) {
          goto lab4Break;
        }
        goto golab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab3Continue:;
    }
  golab3Break:
    // gopast, line 64
    while (true) {
      do {
        if (!(in_grouping(g_v, 1377, 1413))) {
          goto lab6Break;
        }
        goto golab5Break;
      } while (false);
    lab6Continue:;
    lab6Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab5Continue:;
    }
  golab5Break:
    // gopast, line 64
    while (true) {
      do {
        if (!(out_grouping(g_v, 1377, 1413))) {
          goto lab8Break;
        }
        goto golab7Break;
      } while (false);
    lab8Continue:;
    lab8Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab7Continue:;
    }
  golab7Break:
    // setmark p2, line 64
    I_p2 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  return true;
}

bool ArmenianStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool ArmenianStemmer::r_adjective()
{
  int among_var;
  // (, line 72
  // [, line 73
  ket = cursor;
  // substring, line 73
  among_var = find_among_b(a_0, 23);
  if (among_var == 0) {
    return false;
  }
  // ], line 73
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 98
    // delete, line 98
    slice_del();
    break;
  }
  return true;
}

bool ArmenianStemmer::r_verb()
{
  int among_var;
  // (, line 102
  // [, line 103
  ket = cursor;
  // substring, line 103
  among_var = find_among_b(a_1, 71);
  if (among_var == 0) {
    return false;
  }
  // ], line 103
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 176
    // delete, line 176
    slice_del();
    break;
  }
  return true;
}

bool ArmenianStemmer::r_noun()
{
  int among_var;
  // (, line 180
  // [, line 181
  ket = cursor;
  // substring, line 181
  among_var = find_among_b(a_2, 40);
  if (among_var == 0) {
    return false;
  }
  // ], line 181
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 223
    // delete, line 223
    slice_del();
    break;
  }
  return true;
}

bool ArmenianStemmer::r_ending()
{
  int among_var;
  // (, line 227
  // [, line 228
  ket = cursor;
  // substring, line 228
  among_var = find_among_b(a_3, 57);
  if (among_var == 0) {
    return false;
  }
  // ], line 228
  bra = cursor;
  // call R2, line 228
  if (!r_R2()) {
    return false;
  }
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 287
    // delete, line 287
    slice_del();
    break;
  }
  return true;
}

bool ArmenianStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  int v_7;
  // (, line 292
  // do, line 294
  v_1 = cursor;
  do {
    // call mark_regions, line 294
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 295
  limit_backward = cursor;
  cursor = limit;
  // setlimit, line 295
  v_2 = limit - cursor;
  // tomark, line 295
  if (cursor < I_pV) {
    return false;
  }
  cursor = I_pV;
  v_3 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_2;
  // (, line 295
  // do, line 296
  v_4 = limit - cursor;
  do {
    // call ending, line 296
    if (!r_ending()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_4;
  // do, line 297
  v_5 = limit - cursor;
  do {
    // call verb, line 297
    if (!r_verb()) {
      goto lab2Break;
    }
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_5;
  // do, line 298
  v_6 = limit - cursor;
  do {
    // call adjective, line 298
    if (!r_adjective()) {
      goto lab3Break;
    }
  } while (false);
lab3Continue:;
lab3Break:
  cursor = limit - v_6;
  // do, line 299
  v_7 = limit - cursor;
  do {
    // call noun, line 299
    if (!r_noun()) {
      goto lab4Break;
    }
  } while (false);
lab4Continue:;
lab4Break:
  cursor = limit - v_7;
  limit_backward = v_3;
  cursor = limit_backward;
  return true;
}

bool ArmenianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<ArmenianStemmer>(o) != nullptr;
}

int ArmenianStemmer::hashCode()
{
  return ArmenianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext