
#include "utils/annex_k.hpp"
#include "common/Common.hpp"


using namespace testing;
using namespace MAT;

TEST(AnnexKTests, memcpy_s)
{

    // success tests
    char    dest[10];
    char    src[] = "VALUE";
    rsize_t dest_len = sizeof(dest);
    rsize_t src_len = sizeof(src);

    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len, src, 0), 0);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, src_len + 1), 0);
    EXPECT_EQ(strlen(dest), strlen("VALUE"));
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len + 2, src, src_len + 1), 0);
    EXPECT_EQ(strlen(dest), strlen("VALUE"));

    // error tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, 3, src, src_len), EINVAL);
    EXPECT_EQ(dest[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(NULL, 3, src, src_len), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, NULL, src_len), EINVAL);
    EXPECT_EQ(dest[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, dest_len + 1 ), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, dest + 1, src_len + 1 ), EINVAL);
}