#include "CuTest.h"

// library that we are going to test
#include "http.h"

void test_alloc_free_buffer(CuTest *tc) {
    struct http_response response = {NULL, 0, 0, NULL, NULL};
    int rc = alloc_buffer(&response, 100);
    CuAssertIntEquals(tc, 100, rc);
    CuAssertPtrNotNull(tc, response.buffer);

    rc = realloc_buffer(&response);
    CuAssertIntEquals(tc, 200, rc);
    CuAssertIntEquals(tc, 0, strlen(response.buffer));

    free_buffer(&response);
    CuAssertIntEquals(tc, 0, response.size);
}

void test_parse_response(CuTest *tc) {

    struct http_response response = {NULL, 0, 0, NULL, NULL};
    response.buffer = strdup(
        "HTTP/1.1 200 OK\nContent-Length: 5\r\n\r\n{\"a\": \"b\"}"
    );
    response.size = strlen(response.buffer) + 1;
    parse_response(&response);
    CuAssertIntEquals(tc, 200, response.status);
    CuAssertStrEquals(tc, "HTTP/1.1 200 OK", response.message);
    CuAssertStrEquals(tc, "{\"a\": \"b\"}", response.body);
}

CuSuite* HttpTestSuite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_alloc_free_buffer);
    SUITE_ADD_TEST(suite, test_parse_response);
    return suite;
}
