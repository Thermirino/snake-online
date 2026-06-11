#include <SDL_events.h>
#include <stdlib.h>
#include <SDL.h>
#include <unity.h>
#include <render.h>

#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void poll_events(SDL_Renderer* renderer)
{
    SDL_RenderPresent(renderer);
    SDL_Event e;

    SDL_PollEvent(&e);
    while (e.type != SDL_QUIT)
    {
        SDL_PollEvent(&e);
    }
}

void test_render_grid(void)
{
    int w = 1024;
    int h = 512;
    render_context rctx;

    TEST_ASSERT_TRUE(render_init(&rctx, w, h));
    TEST_ASSERT_NOT_NULL(rctx.window);
    TEST_ASSERT_NOT_NULL(rctx.renderer);
    TEST_ASSERT_NOT_NULL(rctx.text_font);
    TEST_ASSERT_TRUE(render_grid(&rctx, &(board){ .height = 15, .width = 40 }));
    poll_events(rctx.renderer);
    render_destroy(&rctx);
}

void test_render_snake(void)
{
    int w = 1024;
    int h = 512;
    point points[5] = {
        { 3, 3 }, { 4, 3 }, { 5, 3 }, { 5, 4 }, { 5, 5 }
    };
    snake sn = { .dir = DIR_LEFT,
                 .body.points = points,
                 .body.capacity = 5,
                 .body.size = 5,
                 .color = GREEN};
    render_context rctx;

    TEST_ASSERT_TRUE(render_init(&rctx, w, h));
    TEST_ASSERT_NOT_NULL(rctx.window);
    TEST_ASSERT_NOT_NULL(rctx.renderer);
    TEST_ASSERT_NOT_NULL(rctx.text_font);
    TEST_ASSERT_TRUE(render_snake(&rctx, &sn));
    TEST_ASSERT_TRUE(render_grid(&rctx, &(board){ .height = 15, .width = 40 }));
    poll_events(rctx.renderer);
    render_destroy(&rctx);
}

void test_render_snakes(void)
{
    int w = 1024;
    int h = 512;
    snake s1 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {3, 3}, {4, 3}, {5, 3}, {5, 4}, {5,5}},
                 .body.capacity = 5,
                 .body.size = 5,
                 .color = GREEN};
    snake s2 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {7, 1}, {7, 2}, {7, 3}},
                 .body.capacity = 3,
                 .body.size = 3,
                 .color = PURPLE};
    snake s3 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {10, 5}, {10, 6}, {10, 7}, {10, 8}, {10, 9}, {11, 9}},
                 .body.capacity = 6,
                 .body.size = 6,
                 .color = ORANGE};
    snake snakes[] = { s1, s2, s3 };
    render_context rctx;

    TEST_ASSERT_TRUE(render_init(&rctx, w, h));
    TEST_ASSERT_TRUE(render_snakes(&rctx, snakes, sizeof(snakes)/sizeof(*snakes)));
    TEST_ASSERT_TRUE(render_grid(&rctx, &(board){ .height = 15, .width = 40 }));
    poll_events(rctx.renderer);
    render_destroy(&rctx);
}

void test_render_game(void)
{
    int w = 1024;
    int h = 512;
    board brd = { .width = 15, .height = 40 };
    snake s1 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {3, 3}, {4, 3}, {5, 3}, {5, 4}, {5,5}},
                 .body.capacity = 5,
                 .body.size = 5,
                 .color = GREEN};
    snake s2 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {7, 1}, {7, 2}, {7, 3}},
                 .body.capacity = 3,
                 .body.size = 3,
                 .color = PURPLE};
    snake s3 = { .dir = DIR_LEFT,
                 .body.points = (point[]){ {10, 5}, {10, 6}, {10, 7}, {10, 8}, {10, 9}, {11, 9}},
                 .body.capacity = 6,
                 .body.size = 6,
                 .color = ORANGE};
    snake snakes[] = { s1, s2, s3 };
    game_state gs = {
        .brd = brd,
        .snakes = snakes,
        .snakes_capacity = 3,
        .snakes_size = 3
    };
    render_context rctx;

    double dt = 0;
    TEST_ASSERT_TRUE(render_init(&rctx, w, h));
    TEST_ASSERT_TRUE(render_game(&rctx, &gs, 0, dt));
    poll_events(rctx.renderer);
    render_destroy(&rctx);
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
    RUN_TEST(test_render_grid);
    RUN_TEST(test_render_snake);
    RUN_TEST(test_render_snakes);
    RUN_TEST(test_render_game);
    return UNITY_END();
}

