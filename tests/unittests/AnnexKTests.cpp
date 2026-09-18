#include "utils/annex_k.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace MAT;

TEST(AnnexKTests, memcpy_s)
{
    volatile size_t dest_size =10;
    volatile size_t src_size = 5;
    std::unique_ptr<void, decltype(&free)> dest(malloc(sizeof(char) * dest_size), &free);
    std::unique_ptr<void, decltype(&free)> src(malloc(sizeof(char) * src_size), &free);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(src.get(), 5, "TEST", 5), 0);
    rsize_t dest_len = dest_size;
    rsize_t src_len = src_size-1;

    // success tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len, src.get(), 0), 0);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len, src.get(), src_len + 1), 0);
    EXPECT_EQ(strlen(static_cast<char *>(dest.get())), strlen("TEST"));
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len + 2, src.get(), src_len + 1), 0);
    EXPECT_EQ(strlen(static_cast<char *>(dest.get())), strlen("TEST"));

    // error tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), 3, src.get(), src_len), EINVAL);
    EXPECT_EQ(static_cast<char *>(dest.get())[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(NULL, 3, src.get(), src_len), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len, NULL, src_len), EINVAL);
    EXPECT_EQ(static_cast<char *>(dest.get())[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len, src.get(), dest_len + 1), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest.get(), dest_len, static_cast<char *>(dest.get()) + 1, src_len + 1), EINVAL);
}
