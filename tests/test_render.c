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
    render_context* rs = render_init(w, h);
    TEST_ASSERT_NOT_NULL(rs);
    TEST_ASSERT_TRUE(render_grid(rs, &(board){ .height = 15, .width = 40 }));
    poll_events(rs->renderer);
    render_destroy(rs);
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
                 .body.size = 5 };

    render_context* rs = render_init(w, h);
    TEST_ASSERT_NOT_NULL(rs);
    TEST_ASSERT_TRUE(render_snake(rs, &sn));
    TEST_ASSERT_TRUE(render_grid(rs, &(board){ .height = 15, .width = 40 }));
    poll_events(rs->renderer);
    render_destroy(rs);
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
    return UNITY_END();
}

