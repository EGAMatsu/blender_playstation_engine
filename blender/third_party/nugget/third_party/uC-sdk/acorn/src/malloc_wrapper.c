#include <malloc.h>
#include <FreeRTOS.h>
#include <semphr.h>

static xSemaphoreHandle malloc_sem = NULL;
static malloc_t old_malloc;
static realloc_t old_realloc;
static free_t old_free;

static void * malloc_wrap(size_t s) {
    void * r;
    xSemaphoreTakeRecursive(malloc_sem, portMAX_DELAY);
    r = old_malloc(s);
    xSemaphoreGiveRecursive(malloc_sem);
    return r;
}

static void * realloc_wrap(void * p, size_t s) {
    void * r;
    xSemaphoreTakeRecursive(malloc_sem, portMAX_DELAY);
    r = old_realloc(p, s);
    xSemaphoreGiveRecursive(malloc_sem);
    return r;
}

static void free_wrap(void * p) {
    xSemaphoreTakeRecursive(malloc_sem, portMAX_DELAY);
    old_free(p);
    xSemaphoreGiveRecursive(malloc_sem);
}

void init_malloc_wrapper() {
    malloc_sem = xSemaphoreCreateRecursiveMutex();
    old_malloc = malloc_ptr;
    old_realloc = realloc_ptr;
    old_free = free_ptr;
    malloc_ptr = malloc_wrap;
    realloc_ptr = realloc_wrap;
    free_ptr = free_wrap;
}

