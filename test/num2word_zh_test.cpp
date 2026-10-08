#include "test.h"
#include <phonemis/lang/zh/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

using namespace zh;

// Expected values are cn2an.transform(text, "an2cn") outputs.
REGISTER_TEST(num2word_zh_cardinal_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"零", layer.transform(U"0"));
    ASSERT_EQUALS(U"十二", layer.transform(U"12"));
    ASSERT_EQUALS(U"一百", layer.transform(U"100"));
    ASSERT_EQUALS(U"一百一十", layer.transform(U"110"));
    ASSERT_EQUALS(U"一千零一", layer.transform(U"1001"));
    ASSERT_EQUALS(U"一万零一十", layer.transform(U"10010"));
    ASSERT_EQUALS(U"二万零五百", layer.transform(U"20500"));
    ASSERT_EQUALS(U"十万", layer.transform(U"100000"));
    ASSERT_EQUALS(U"一千零二十万三千零四十", layer.transform(U"10203040"));
    ASSERT_EQUALS(U"一亿", layer.transform(U"100000000"));
    ASSERT_EQUALS(U"十亿零一万", layer.transform(U"1000010000"));
    ASSERT_EQUALS(U"七千五百九十", layer.transform(U"07５90"));
    ASSERT_EQUALS(U"零点五", layer.transform(U"0.5"));
    ASSERT_EQUALS(U"三点一四一五九二六五三五八九七九三二", layer.transform(U"3.14159265358979323846"));
    ASSERT_EQUALS(U"他有三个苹果。", layer.transform(U"他有3个苹果。"));

    // More than 16 digits are left as they are.
    ASSERT_EQUALS(U"12345678901234567", layer.transform(U"12345678901234567"));

    return true;
}

REGISTER_TEST(num2word_zh_special_forms_test)
{
    Num2Word layer;

    ASSERT_EQUALS(U"二零二四年五月一日", layer.transform(U"2024年5月1日"));
    ASSERT_EQUALS(U"三分之一", layer.transform(U"1/3"));
    ASSERT_EQUALS(U"百分之负五", layer.transform(U"-5%"));
    ASSERT_EQUALS(U"二十五摄氏度", layer.transform(U"25℃"));
    ASSERT_EQUALS(U"三到五个", layer.transform(U"3-5个"));
    ASSERT_EQUALS(U"一点五到三点五公斤", layer.transform(U"1.5-3.5公斤"));

    // Without a measure word, a dash is a minus sign.
    ASSERT_EQUALS(U"九十九a一负六", layer.transform(U"9９a1-6"));

    return true;
}

} // namespace phonemis::test
