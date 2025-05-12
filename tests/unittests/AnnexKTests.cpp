#include "utils/annex_k.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace MAT;

TEST(AnnexKTests, memcpy_s)
{
    volatile size_t dest_size =10;
    volatile size_t src_size = 5;
    void    *dest =  malloc(sizeof(char) * dest_size);
    void    *src = malloc(sizeof(char) * src_size);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(src, 5, "TEST", 5), 0);
    rsize_t dest_len = dest_size;
    rsize_t src_len = src_size-1;

    // success tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len, src, 0), 0);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, src_len + 1), 0);
    EXPECT_EQ(strlen((char *)dest), strlen("TEST"));
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len + 2, src, src_len + 1), 0);
    EXPECT_EQ(strlen((char *)dest), strlen("TEST"));

    // error tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, 3, src, src_len), EINVAL);
    EXPECT_EQ(((char *)dest)[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(NULL, 3, src, src_len), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, NULL, src_len), EINVAL);
    EXPECT_EQ(((char *)dest)[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, dest_len + 1 ), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, (void *)((char *)dest + 1), src_len + 1 ), EINVAL);
}
