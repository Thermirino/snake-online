#include <string.h>
#include <unity.h>
#include <game.h>

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

static void test_game_state(void)
{
    game_state gs;
    int width = 20;
    int height = 15;

    TEST_ASSERT_FALSE(game_state_init(NULL, width, height));

    TEST_ASSERT_TRUE(game_state_init(&gs, width, height));
    TEST_ASSERT_EQUAL(width, gs.brd.width);
    TEST_ASSERT_EQUAL(height, gs.brd.height);
    TEST_ASSERT_NULL(gs.snakes);
    TEST_ASSERT_EQUAL(0, gs.snakes_capacity);
    TEST_ASSERT_EQUAL(0, gs.snakes_size);


    snake s1;
    uint32_t id1 = 1;
    int y = 5, x = 8;
    color_name color = GREEN;
    TEST_ASSERT_FALSE(game_state_add_snake(&gs, NULL));
    TEST_ASSERT_FALSE(game_state_add_snake(NULL, &s1));
    TEST_ASSERT_TRUE(snake_init(&s1, id1, DIR_UP, y, x, color));
    TEST_ASSERT(game_state_add_snake(&gs, &s1));
    TEST_ASSERT_NOT_NULL(gs.snakes);
    TEST_ASSERT_GREATER_OR_EQUAL(1, gs.snakes_capacity);
    TEST_ASSERT_EQUAL(1, gs.snakes_size);
    snake_is_equal(&s1, &gs.snakes[0]);

    snake s2;
    uint32_t id2 = 1;
    y = 3, x = 4;
    color = RED;
    TEST_ASSERT_TRUE(snake_init(&s2, id2, DIR_UP, y, x, color));
    TEST_ASSERT(game_state_add_snake(&gs, &s2));
    TEST_ASSERT_NOT_NULL(gs.snakes);
    TEST_ASSERT_GREATER_OR_EQUAL(2, gs.snakes_capacity);
    TEST_ASSERT_EQUAL(2, gs.snakes_size);
    snake_is_equal(&s1, &gs.snakes[0]);
    snake_is_equal(&s2, &gs.snakes[1]);

    game_state_destroy(&gs);
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
    RUN_TEST(test_game_state);
    return UNITY_END();
}

