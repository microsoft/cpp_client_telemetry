#include "utils/annex_k.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace MAT;

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 26483  //ignore array out of bound
#elif defined(__GNUC__) && !defined(__clang__) && !defined(__apple_build_version__)
#  if __GNUC__ >= 11
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Warray-bounds"
#  endif
#elif defined(__clang__) || defined(__apple_build_version__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Warray-bounds"
#endif


TEST(AnnexKTests, memcpy_s)
{
    char    dest[10];
    char    *src = (char*) malloc(sizeof(char) * 5);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(src, 5, "TEST", 5), 0);
    rsize_t dest_len = sizeof(dest);
    rsize_t src_len = sizeof(src);

    // success tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len, src, 0), 0);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, src_len + 1), 0);
    EXPECT_EQ(strlen(dest), strlen("TEST"));
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, dest_len + 2, src, src_len + 1), 0);
    EXPECT_EQ(strlen(dest), strlen("TEST"));

    // error tests
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(dest, 3, src, src_len), EINVAL);
    EXPECT_EQ(dest[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s(NULL, 3, src, src_len), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, NULL, src_len), EINVAL);
    EXPECT_EQ(dest[0], '\0');
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, src, dest_len + 1 ), EINVAL);
    EXPECT_EQ(BoundCheckFunctions::oneds_memcpy_s( dest, dest_len, dest + 1, src_len + 1 ), EINVAL);
}

#if defined(__GNUC__) && !defined(__clang__) && !defined(__apple_build_version__)
#  if __GNUC__ >= 11
#    pragma GCC diagnostic pop
#  endif
#elif defined(__clang__) || defined(__apple_build_version__)
#  pragma clang diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
