#include <stdlib.h>
#include <string.h>
#include <unity.h>
#include <protocol.h>

#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void snake_is_equal(snake* s1, snake* s2)
{
    TEST_ASSERT_EQUAL(s1->id, s2->id);
    TEST_ASSERT_EQUAL(s1->dir, s2->dir);
    TEST_ASSERT_EQUAL(s1->body.size, s2->body.size);
    TEST_ASSERT_EQUAL(0, memcmp(s1->body.points, s2->body.points, s1->body.size * sizeof(point)));
    TEST_ASSERT_EQUAL(s1->color, s2->color);
}

static void test_serialize(void)
{
    game_state gs1;
    memset(&gs1, 0, sizeof(gs1));
    int width = 20;
    int height = 30;
    uint8_t* buf;
    size_t size;

    TEST_ASSERT_TRUE(game_state_init(&gs1, width, height));
    TEST_ASSERT_TRUE(game_state_serialize(&gs1, &buf, &size));

    game_state gs2;
    memset(&gs2, 0, sizeof(gs2));
    TEST_ASSERT_FALSE(game_state_deserialize(buf, 0, &gs2));
    TEST_ASSERT_FALSE(game_state_deserialize(buf, size - 1, &gs2));
    TEST_ASSERT_TRUE(game_state_deserialize(buf, size, &gs2));
    TEST_ASSERT_EQUAL(gs1.brd.width, gs2.brd.width);
    TEST_ASSERT_EQUAL(gs1.brd.height, gs2.brd.height);
    TEST_ASSERT_EQUAL(0, gs2.snakes_capacity);
    TEST_ASSERT_EQUAL(0, gs2.snakes_size);
    game_state_destroy(&gs2);
    free(buf);

    // add 2 snakes
    snake s1;
    int id1 = 1;
    int id2 = 2;
    int y = 5, x = 8;
    const char* nickname = NULL;
    color_name color = GREEN;
    TEST_ASSERT_TRUE(snake_init(&s1, id1, nickname, DIR_UP, y, x, color));
    TEST_ASSERT(game_state_add_snake(&gs1, &s1));
    snake s2;
    y = 3, x = 4;
    color = RED;
    TEST_ASSERT_TRUE(snake_init(&s2, id2, nickname, DIR_UP, y, x, color));
    TEST_ASSERT(game_state_add_snake(&gs1, &s2));

    TEST_ASSERT_TRUE(game_state_serialize(&gs1, &buf, &size));
    memset(&gs2, 0, sizeof(gs2));
    TEST_ASSERT_FALSE(game_state_deserialize(buf, size - 1, &gs2));
    TEST_ASSERT_TRUE(game_state_deserialize(buf, size, &gs2));
    TEST_ASSERT_EQUAL(gs1.brd.width, gs2.brd.width);
    TEST_ASSERT_EQUAL(gs1.brd.height, gs2.brd.height);
    TEST_ASSERT_GREATER_OR_EQUAL(2, gs2.snakes_capacity);
    TEST_ASSERT_EQUAL(gs1.snakes_size, gs2.snakes_size);
    for (size_t i = 0; i < gs2.snakes_size; i++) {
        snake_is_equal(&gs1.snakes[i], &gs2.snakes[i]);
    }
    game_state_destroy(&gs2);
    free(buf);

    game_state_destroy(&gs1);
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

