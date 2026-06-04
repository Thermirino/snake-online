#include <unity.h>
#include <snake.h>

#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void test_snake_init(void)
{
    snake s;
    uint32_t id = 1;
    direction dir = DIR_UP;
    color_name color = GREEN;

    TEST_ASSERT_TRUE(snake_init(&s, id, dir, 0, 0, color));
    TEST_ASSERT_EQUAL(id, s.id);
    TEST_ASSERT_EQUAL(dir, s.dir);
    TEST_ASSERT_EQUAL(color, s.color);
    TEST_ASSERT_GREATER_OR_EQUAL(1, s.body.capacity);
    TEST_ASSERT_EQUAL(1, s.body.size);
    TEST_ASSERT_NOT_NULL(s.body.points);
    TEST_ASSERT_EQUAL(0, s.body.points[0].y);
    TEST_ASSERT_EQUAL(0, s.body.points[0].x);
    snake_destroy(&s);

    id = 2;
    TEST_ASSERT_TRUE(snake_init(&s, id, dir, 5, 5, color));
    TEST_ASSERT_EQUAL(id, s.id);
    TEST_ASSERT_EQUAL(dir, s.dir);
    TEST_ASSERT_EQUAL(color, s.color);
    TEST_ASSERT_GREATER_OR_EQUAL(1, s.body.capacity);
    TEST_ASSERT_EQUAL(1, s.body.size);
    TEST_ASSERT_NOT_NULL(s.body.points);
    TEST_ASSERT_EQUAL(5, s.body.points[0].y);
    TEST_ASSERT_EQUAL(5, s.body.points[0].x);
    snake_destroy(&s);

    TEST_ASSERT_FALSE(snake_init(&s, -1, dir, 1000, 1000, color));
    TEST_ASSERT_FALSE(snake_init(&s, id, 5, 1000, 1000, color));
    TEST_ASSERT_FALSE(snake_init(NULL, id, dir, 1000, 1000, color));
}

void test_snake_move(void)
{
    snake s;
    int id = 1;
    direction dir = DIR_DOWN;
    color_name color = GREEN;

    TEST_ASSERT_TRUE(snake_init(&s, id, dir, 5, 5, color));
    TEST_ASSERT_EQUAL(5, s.body.points[0].y);
    TEST_ASSERT_EQUAL(5, s.body.points[0].x);
    TEST_ASSERT_TRUE(snake_move(&s));
    TEST_ASSERT_EQUAL(6, s.body.points[0].y);
    TEST_ASSERT_EQUAL(5, s.body.points[0].x);

    TEST_ASSERT_TRUE(snake_change_direction(&s, DIR_RIGHT));
    TEST_ASSERT_TRUE(snake_move(&s));
    TEST_ASSERT_EQUAL(6, s.body.points[0].y);
    TEST_ASSERT_EQUAL(6, s.body.points[0].x);

    TEST_ASSERT_TRUE(snake_change_direction(&s, DIR_UP));
    TEST_ASSERT_TRUE(snake_move(&s));
    TEST_ASSERT_EQUAL(5, s.body.points[0].y);
    TEST_ASSERT_EQUAL(6, s.body.points[0].x);

    TEST_ASSERT_TRUE(snake_change_direction(&s, DIR_LEFT));
    TEST_ASSERT_TRUE(snake_move(&s));
    TEST_ASSERT_EQUAL(5, s.body.points[0].y);
    TEST_ASSERT_EQUAL(5, s.body.points[0].x);

    snake_destroy(&s);
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
    RUN_TEST(test_snake_init);
    RUN_TEST(test_snake_move);
    return UNITY_END();
}

