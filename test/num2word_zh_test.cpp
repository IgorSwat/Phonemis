#include "processing.h"
#include "test.h"
#include <phonemis/lang/zh/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace zh;

// Expected values are apply(cn2an, text, "an2cn") outputs.
REGISTER_TEST(num2word_zh_cardinal_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"零", apply(layer, U"0"));
    ASSERT_EQUALS(U"十二", apply(layer, U"12"));
    ASSERT_EQUALS(U"一百", apply(layer, U"100"));
    ASSERT_EQUALS(U"一百一十", apply(layer, U"110"));
    ASSERT_EQUALS(U"一千零一", apply(layer, U"1001"));
    ASSERT_EQUALS(U"一万零一十", apply(layer, U"10010"));
    ASSERT_EQUALS(U"二万零五百", apply(layer, U"20500"));
    ASSERT_EQUALS(U"十万", apply(layer, U"100000"));
    ASSERT_EQUALS(U"一千零二十万三千零四十", apply(layer, U"10203040"));
    ASSERT_EQUALS(U"一亿", apply(layer, U"100000000"));
    ASSERT_EQUALS(U"十亿零一万", apply(layer, U"1000010000"));
    ASSERT_EQUALS(U"七千五百九十", apply(layer, U"07５90"));
    ASSERT_EQUALS(U"零点五", apply(layer, U"0.5"));
    ASSERT_EQUALS(U"三点一四一五九二六五三五八九七九三二", apply(layer, U"3.14159265358979323846"));
    ASSERT_EQUALS(U"他有三个苹果。", apply(layer, U"他有3个苹果。"));

    // More than 16 digits are left as they are.
    ASSERT_EQUALS(U"12345678901234567", apply(layer, U"12345678901234567"));

    return true;
}

REGISTER_TEST(num2word_zh_special_forms_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"二零二四年五月一日", apply(layer, U"2024年5月1日"));
    ASSERT_EQUALS(U"三分之一", apply(layer, U"1/3"));
    ASSERT_EQUALS(U"百分之负五", apply(layer, U"-5%"));
    ASSERT_EQUALS(U"二十五摄氏度", apply(layer, U"25℃"));
    ASSERT_EQUALS(U"三到五个", apply(layer, U"3-5个"));
    ASSERT_EQUALS(U"一点五到三点五公斤", apply(layer, U"1.5-3.5公斤"));

    // Without a measure word, a dash is a minus sign.
    ASSERT_EQUALS(U"九十九a一负六", apply(layer, U"9９a1-6"));

    return true;
}

} // namespace phonemis::test
