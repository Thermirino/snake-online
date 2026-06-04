#include <unity.h>
#include <server.h>

#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC)
#include <stdlib.h>
#include <crtdbg.h>
#endif

static void test_server(void)
{
    const char* port = "12345";
    int width = 20;
    int height = 10;
    TEST_ASSERT_TRUE(server_run(port, width, height));
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
    RUN_TEST(test_server);
    return UNITY_END();
}

