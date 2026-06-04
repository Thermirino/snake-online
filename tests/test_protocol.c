#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <protocol.h>

#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void test_serialize(void)
{
    game_state gs;
    memset(&gs, 0, sizeof(gs));
    int width = 20;
    int height = 30;
    uint8_t* buf;
    size_t size;

    TEST_ASSERT_TRUE(game_state_init(&gs, width, height));
    TEST_ASSERT_TRUE(game_state_serialize(&gs, &buf, &size));
    game_state_destroy(&gs);

    game_state gs2;
    memset(&gs2, 0, sizeof(gs));
    TEST_ASSERT_TRUE(game_state_deserialize(buf, &gs2));
    TEST_ASSERT_EQUAL(width, gs2.brd.width);
    TEST_ASSERT_EQUAL(height, gs2.brd.height);
    TEST_ASSERT_EQUAL(0, gs2.snakes_capacity);
    TEST_ASSERT_EQUAL(0, gs2.snakes_size);
    TEST_ASSERT_EQUAL(0, memcmp(&gs, &gs2, sizeof(gs)));
    free(buf);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char** argv)
{
#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_serialize);
    return UNITY_END();
}

