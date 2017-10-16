#ifndef VIOLET_CORE_H
#define VIOLET_CORE_H

#include <assert.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#define NOMB
#define NOMINMAX
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef LARGE_INTEGER timepoint_t;

#else

#include <time.h>
typedef struct timespec timepoint_t;

#endif // _WIN32


/* Utility macros */

#define countof(x) (sizeof(x) / sizeof((x)[0]))

#define static_assert(cnd, msg) typedef int msg[(cnd) ? 1 : -1]

#define UNUSED(x) ((void)(x))
#define NOOP UNUSED(0)

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define LOCATION __FILE__ ":" STRINGIFY(__LINE__)

#ifndef NDEBUG
#define check(x) assert(x)
#else
#define check(x) x
#endif

#define memswp(a, b, type) \
	do { type tmp = a; a = b; b = tmp; } while(0)

#ifndef WIN32
#define thread_local __thread
#else
#define thread_local __declspec(thread)
#endif


/* Types */

typedef bool b8;
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t b32;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;
typedef float r32;
typedef double r64;

typedef union uintptr
{
	uintptr_t i;
	void *p;
} uintptr;

typedef union intptr
{
	intptr_t i;
	void *p;
} intptr;

static_assert(sizeof(r32) == 4, invalid_floating_point_size);
static_assert(sizeof(r64) == 8, invalid_double_size);

/* Error handling */

typedef void (*error_f)(const char *msg, void *udata);

struct error_handler
{
	error_f func;
	void *udata;
	const struct error_handler *prev;
};

extern thread_local const struct error_handler *g_error_handler;
#ifndef ERROR_HANDLER_STACK_SIZE
#define ERROR_HANDLER_STACK_SIZE 5
#endif
void error_handler_push(error_f func, void *udata);
void error_handler_pop(error_f func, void *udata);
#ifndef DEBUG
#define error(msg)          g_error_handler->func(msg, g_error_handler->udata);
#else
#define error(msg)          g_error_handler->func(LOCATION ": " msg, \
                                                  g_error_handler->udata);
#endif
#define error_if(cnd, msg)  do { if (cnd) error(msg); } while (0);

void error_catch(const char *msg, void *udata);

#define try \
	do { \
		jmp_buf jmpbuf; \
		temp_memory_save(); \
		error_handler_push(error_catch, &jmpbuf); \
		if (setjmp(jmpbuf) == 0)
#define catch \
		else
#define finally \
		error_handler_pop(error_catch, &jmpbuf); \
		temp_memory_restore(); \
	} while (0)


/* Memory allocation */

/* Provide a compile option VLT_TRACK_MEMORY to override the default alloc.
 * This enables memory tracking without any add'l usage code and
 * allows easier memory tracking for imported libraries.
 *
 * Defining VLT_TRACK_MEMORY_VERBOSE will log ALL allocations,
 * which is occasionally useful but is quite verbose.
 *
 * Defining VLT_ANALYZE_TEMP_MEMORY will help track down inefficient use
 * of the temporary allocator.
 * */

#ifdef VLT_TRACK_MEMORY
#define MEMCALL_LOCATION , LOCATION
#define MEMCALL_ARGS , const char *loc
#define MEMCALL_VARS , loc
#else
#define MEMCALL_LOCATION
#define MEMCALL_ARGS
#define MEMCALL_VARS
#endif

typedef struct allocator allocator_t;
typedef struct allocator
{
	void* (*malloc_)(size_t size, allocator_t *a  MEMCALL_ARGS);
	void* (*calloc_)(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS);
	void* (*realloc_)(void *ptr, size_t size, allocator_t *a  MEMCALL_ARGS);
	void  (*free_)(void *ptr, allocator_t *a  MEMCALL_ARGS);
	void *udata;
} allocator_t;

#define allocator_create(name, udata_) \
	(allocator_t){ \
		.malloc_  = name##_malloc, \
		.calloc_  = name##_calloc, \
		.realloc_ = name##_realloc, \
		.free_    = name##_free, \
		.udata    = udata_, \
	}

#define amalloc(sz, a)          (a)->malloc_(sz, a  MEMCALL_LOCATION)
#define acalloc(nmemb, sz, a)   (a)->calloc_(nmemb, sz, a  MEMCALL_LOCATION)
#define arealloc(p, sz, a)      (a)->realloc_(p, sz, a  MEMCALL_LOCATION)
#define afree(p, a)             (a)->free_(p, a  MEMCALL_LOCATION)

/* Default global allocator */
extern thread_local allocator_t *g_allocator;

/* Temporary memory global allocator */
extern thread_local allocator_t *g_temp_allocator;

/* Default allocator */
void *default_malloc(size_t size, allocator_t *a  MEMCALL_ARGS);
void *default_calloc(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS);
void *default_realloc(void *ptr, size_t size, allocator_t *a  MEMCALL_ARGS);
void  default_free(void *ptr, allocator_t *a  MEMCALL_ARGS);

/* Paged bump memory allocator */
void *pgb_malloc(size_t size, allocator_t *a  MEMCALL_ARGS);
void *pgb_calloc(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS);
void *pgb_realloc(void *ptr, size_t size, allocator_t *a  MEMCALL_ARGS);
void  pgb_free(void *ptr, allocator_t *a  MEMCALL_ARGS);

typedef struct pg_watermark
{
	u8 **page;
	u8 *ptr;
} pgb_watermark_t;

pgb_watermark_t pgb_save(const allocator_t *a);
void            pgb_restore(allocator_t *a, pgb_watermark_t watermark);
#define temp_memory_save()     const pgb_watermark_t memmark = pgb_save(g_temp_allocator)
#define temp_memory_restore()  pgb_restore(g_temp_allocator, memmark)

/* Tracking allocator */
typedef struct alloc_node
{
	struct alloc_node *prev, *next;
	size_t sz, generation;
	const char *location;
} alloc_node_t;

typedef struct alloc_tracker
{
	alloc_node_t *head, *tail;
	size_t generation;
	size_t current_bytes, peak_bytes, total_bytes;
	size_t total_chunks;
} alloc_tracker_t;

void alloc_tracker_advance_gen(alloc_tracker_t *tracker);
void alloc_tracker_log_usage(alloc_tracker_t *tracker);
void alloc_tracker_log_current_gen(alloc_tracker_t *tracker, size_t gen);

void *tracked_malloc(size_t size, allocator_t *a  MEMCALL_ARGS);
void *tracked_calloc(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS);
void *tracked_realloc(void *ptr, size_t size, allocator_t *a  MEMCALL_ARGS);
void  tracked_free(void *ptr, allocator_t *a  MEMCALL_ARGS);

void vlt_mem_advance_gen(void);
void vlt_mem_log_usage(void);

#ifdef VLT_TRACK_MEMORY

/* ensure access to stdlib functions before overriding them */
static inline void *std_malloc(size_t sz) { return malloc(sz); }
static inline void *std_calloc(size_t nmemb, size_t sz) { return calloc(nmemb, sz); }
static inline void *std_realloc(void *ptr, size_t sz) { return realloc(ptr, sz); }
static inline void  std_free(void *ptr) { return free(ptr); }

#define malloc(sz)         amalloc(sz, g_allocator)
#define calloc(nmemb, sz)  acalloc(nmemb, sz, g_allocator)
#define realloc(ptr, sz)   arealloc(ptr, sz, g_allocator)
#define free(ptr)          afree(ptr, g_allocator)

#else

#define std_malloc(sz) malloc(sz)
#define std_calloc(nmemb, sz) calloc(nmemb, sz)
#define std_realloc(ptr, sz) realloc(ptr, sz)
#define std_free(ptr) free(ptr)

#endif // VLT_TRACK_MEMORY

/* Hash */

u32 hash(const char *str);
u32 hashn(const char *str, u32 n);

/* Time */

timepoint_t time_current();
u32         time_diff_milli(timepoint_t start, timepoint_t end);
u32         time_diff_micro(timepoint_t start, timepoint_t end);
void        time_sleep_milli(u32 milli);

/* Log */

typedef enum log_level
{
	LOG_DEBUG   = 0x1,
	LOG_INFO    = 0x2,
	LOG_WARNING = 0x4,
	LOG_ERROR   = 0x8,
	LOG_FATAL   = 0x10,
	LOG_ALL     = (LOG_DEBUG | LOG_INFO | LOG_WARNING | LOG_ERROR | LOG_FATAL),
	LOG_STDOUT  = (LOG_DEBUG | LOG_INFO),
	LOG_STDERR  = (LOG_WARNING | LOG_ERROR | LOG_FATAL),
} log_level_t;

typedef void(*logger_t)(void *udata, log_level_t level,
                        const char *format, va_list ap);

#define LOG_STREAM_CAP 3

void log_add_stream(log_level_t level, logger_t logger, void *udata);
void log_remove_stream(logger_t logger, void *udata);
#define log_add_file(level, fp) log_add_stream(level, file_logger, fp)
#define log_remove_file(fp) log_remove_stream(file_logger, fp)
#define log_add_std(stdout_level) do { \
		log_add_file(stdout_level, stdout); \
		log_add_file(LOG_STDERR, stderr); \
	} while (0);
#define log_remove_std() do { \
		log_remove_file(stdout); \
		log_remove_file(stderr); \
	} while (0);
void log_level_write(log_level_t level, const char *format, ...);
#define log_debug(fmt, ...) log_level_write(LOG_DEBUG,   fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_level_write(LOG_INFO,    fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_level_write(LOG_WARNING, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_level_write(LOG_ERROR,   fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) log_level_write(LOG_FATAL,   fmt, ##__VA_ARGS__)
#define log_write(fmt, ...) log_level_write(LOG_INFO,    fmt, ##__VA_ARGS__)

void file_logger(void *udata, log_level_t level, const char *format, va_list ap);

#endif // VIOLET_CORE_H


/* Implementation */

#ifdef CORE_IMPLEMENTATION

#include "list.h"

/* Error handling */

static
void error_fatal(const char *msg, void *udata)
{
	log_fatal(msg);
#ifdef DEBUG
	assert(false);
#else
	exit(1);
#endif
}

thread_local struct error_handler g_error_handler_stack[ERROR_HANDLER_STACK_SIZE];

thread_local const struct error_handler *g_error_handler = &(struct error_handler) {
	.func = error_fatal,
	.udata = NULL,
	.prev = NULL,
};

void error_handler_push(error_f func, void *udata)
{
	u32 idx = g_error_handler->prev ? g_error_handler - g_error_handler_stack + 1 : 0;
	assert(idx < ERROR_HANDLER_STACK_SIZE);
	g_error_handler_stack[idx].func = func;
	g_error_handler_stack[idx].udata = udata;
	g_error_handler_stack[idx].prev = g_error_handler;
	g_error_handler = &g_error_handler_stack[idx];
}

void error_handler_pop(error_f func, void *udata)
{
	assert(g_error_handler->func == func);
	assert(g_error_handler->udata == udata);
	assert(g_error_handler->prev);
	g_error_handler = g_error_handler->prev;
}

void error_catch(const char *msg, void *udata)
{
	log_fatal("%s", msg);
	longjmp(*(jmp_buf*)udata, 0);
}

/* Memory allocation */

#ifdef VLT_TRACK_MEMORY
thread_local allocator_t *g_allocator =
	&allocator_create(tracked, &(alloc_tracker_t){0});
#else
thread_local allocator_t *g_allocator = &allocator_create(default, NULL);
#endif

static
void log_alloc(const char *prefix, size_t sz  MEMCALL_ARGS)
{
#ifdef VLT_TRACK_MEMORY_VERBOSE
	log_debug("%s_alloc: %lu @ %s", prefix, sz, loc);
#endif
}


/* TODO(rgrieg): make this public so consumer can create pgb allocators */
typedef struct pgb_data
{
	list(u8*) pages;
	u8 **current_page;
	u8 *current_ptr;
} pgb_data_t;

thread_local allocator_t *g_temp_allocator = &allocator_create(pgb, &(pgb_data_t){0});


/* Default allocator */

void *default_malloc(size_t size, allocator_t *a  MEMCALL_ARGS)
{
	return std_malloc(size);
}

void *default_calloc(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS)
{
	return std_calloc(nmemb, size);
}

void *default_realloc(void *ptr, size_t size, allocator_t *a  MEMCALL_ARGS)
{
	return std_realloc(ptr, size);
}

void default_free(void *ptr, allocator_t *a  MEMCALL_ARGS)
{
	std_free(ptr);
}

/* Paged bump memory allocator
 *
 * For 4096 byte pages, allocating on 16 byte boundaries allows 256 possible
 * pointer locations.  We can store the size of an allocation using 1 byte since
 * the lower 4 bits of the address should be 0 due to alignment. This provides
 * an 88% efficient use of memory to track the pointer sizes, which is required
 * for a correct realloc implementation, optimizing subsequent realloc calls,
 * and allowing a scope that only allocates a single array to replace saving
 * and restoring a watermark with a simple array_destroy(). */

#define PGB_PAGE_SIZE 4096
#define PGB_ALIGNMENT 16
#define PGB_PAGE_HEADER_SIZE (PGB_PAGE_SIZE / PGB_ALIGNMENT)
#define PGB_PAGE_USABLE_SIZE (PGB_PAGE_SIZE - PGB_PAGE_HEADER_SIZE)

typedef union max_align
{
	long l;
	unsigned long ul;
	double d;
	long double ld;
	void *p;
	void (*f)(void);
} max_align_t;

static_assert(sizeof(max_align_t) <= PGB_ALIGNMENT,
              invalid_word_size_for_pgb_allocator);

static inline
u8 *pgb__alloc_sz_loc(const u8 *ptr, u8 *page)
{
	return &page[(ptr - page) / PGB_ALIGNMENT];
}
static inline
u32 pgb__alloc_get_sz(const u8* ptr, u8* page)
{
	return *pgb__alloc_sz_loc(ptr, page) * PGB_ALIGNMENT;
}
static inline
void pgb__alloc_set_sz(const u8 *ptr, u8 *page, size_t sz)
{
	*pgb__alloc_sz_loc(ptr, page) = (u8)(sz / PGB_ALIGNMENT);
}

static
b32 pgb__ptr_is_last_alloc(const u8 *ptr, const pgb_data_t *data)
{
	return   ptr > *data->current_page
	      && ptr < *data->current_page + PGB_PAGE_SIZE
	      && ptr + pgb__alloc_get_sz(ptr, *data->current_page) == data->current_ptr;
}

static
void pgb_data__add_page(pgb_data_t *data)
{
	u8 *first_page = malloc(PGB_PAGE_SIZE);
	memset(first_page, 0, PGB_PAGE_HEADER_SIZE);
	data->current_page = list_append(data->pages, first_page);
	data->current_ptr = *data->current_page + PGB_PAGE_HEADER_SIZE;
}

static
void pgb_data__init(pgb_data_t *data)
{
	data->pages = list_create();
	pgb_data__add_page(data);
}

static
void pgb_data__destroy(pgb_data_t *data)
{
	list_foreach(data->pages, u8*, page)
		free(*page);
	list_destroy(data->pages);
}

void *pgb_malloc(size_t size, allocator_t *a  MEMCALL_ARGS)
{
	pgb_data_t *data = a->udata;
	u8 *ptr;
	size += (PGB_ALIGNMENT - (size % PGB_ALIGNMENT)) % PGB_ALIGNMENT;
	error_if(size > PGB_PAGE_USABLE_SIZE, "size too large for pgb allocator");
	if (data->current_ptr + size > *data->current_page + PGB_PAGE_SIZE) {
		data->current_page = list_next(data->current_page);
		if (!data->current_page)
			pgb_data__add_page(data);
		else
			data->current_ptr = *data->current_page + PGB_PAGE_HEADER_SIZE;
	}
	ptr = data->current_ptr;
	pgb__alloc_set_sz(ptr, *data->current_page, size);
	data->current_ptr += size;
	log_alloc("pgb", size  MEMCALL_VARS);
	return ptr;
}

void *pgb_calloc(size_t nmemb, size_t size, allocator_t *a  MEMCALL_ARGS)
{
	u8 *ptr = pgb_malloc(nmemb * size, a  MEMCALL_VARS);
	memset(ptr, 0, nmemb * size);
	return ptr;
}

void *pgb_realloc(void *ptr_, size_t size, allocator_t *a  MEMCALL_ARGS)
{
	u8 *ptr = ptr_;
	pgb_data_t *data = a->udata;
	if (ptr) {
		if (size) {
			if (   pgb__ptr_is_last_alloc(ptr, data)
			    && ptr + size <= *data->current_page + PGB_PAGE_SIZE) {
				pgb_free(ptr, a  MEMCALL_VARS);
				return pgb_malloc(size, a  MEMCALL_VARS);
			} else {
				u8 **page = data->current_page, *new_ptr;
				while (page && !(ptr > *page && ptr < *page + PGB_PAGE_SIZE))
					page = list_prev(page);
				error_if(!page, "could not find page for allocation");
				new_ptr = pgb_malloc(size, a  MEMCALL_VARS);
				memcpy(new_ptr, ptr, pgb__alloc_get_sz(ptr, *page));
				return new_ptr;
			}
		} else {
			pgb_free(ptr, a  MEMCALL_VARS);
			return NULL;
		}
	} else if (size) {
		return pgb_malloc(size, a  MEMCALL_VARS);
	} else {
		return NULL;
	}
}

void pgb_free(void *ptr, allocator_t *a  MEMCALL_ARGS)
{
	pgb_data_t *data = a->udata;
	if (pgb__ptr_is_last_alloc(ptr, data))
		data->current_ptr = ptr;
#ifdef VLT_ANALYZE_TEMP_MEMORY
	else
		log_warn("pgb_free: cannot recapture memory @ %s", loc);
#endif
}

pgb_watermark_t pgb_save(const allocator_t *a)
{
	pgb_data_t *data = a->udata;
	return (pgb_watermark_t) {
		.page = data->current_page,
		.ptr  = data->current_ptr,
	};
}

void pgb_restore(allocator_t *a, pgb_watermark_t watermark)
{
	pgb_data_t *data = a->udata;
	data->current_page   = watermark.page;
	data->current_ptr    = watermark.ptr;
}

/* Tracking allocator */

static
void alloc_tracker__record_alloc(alloc_tracker_t *tracker, size_t sz)
{
	tracker->current_bytes += sz;
	if (tracker->peak_bytes < tracker->current_bytes)
		tracker->peak_bytes = tracker->current_bytes;
	tracker->total_bytes += sz;
	++tracker->total_chunks;
}

static
void alloc_tracker__record_free(alloc_tracker_t *tracker, size_t sz)
{
	tracker->current_bytes -= sz;
}

static
void alloc_tracker__append_node(alloc_tracker_t *tracker,
                                alloc_node_t *node, size_t sz  MEMCALL_ARGS)
{
	node->sz = sz;
	node->generation = tracker->generation;
	node->next = NULL;
#ifdef VLT_TRACK_MEMORY
	node->location = loc;
#else
	node->location = "(unknown)";
#endif
	alloc_tracker__record_alloc(tracker, sz);
	if (tracker->tail) {
		tracker->tail->next = node;
		node->prev = tracker->tail;
		tracker->tail = node;
	} else {
		tracker->head = node;
		tracker->tail = node;
		node->prev = NULL;
	}
}

void alloc_tracker_advance_gen(alloc_tracker_t *tracker)
{
	++tracker->generation;
	error_if(tracker->generation == 0, "generation wrap-around");
}

void alloc_tracker_log_usage(alloc_tracker_t *tracker)
{
	alloc_node_t *node;

	log_info("memory diagnostic:");

	node = tracker->head;
 	while (node) {
 		log_warn("%p: %6lu bytes still active from %s @ gen %lu!",
		          node + 1, node->sz, node->location, node->generation);
 		node = node->next;
 	}

	if (tracker->current_bytes != 0)
		log_warn("exit:  %10lu bytes still allocated!", tracker->current_bytes);

	log_info("peak:  %10lu bytes", tracker->peak_bytes);
 	log_info("total: %10lu bytes in %lu chunks",
	         tracker->total_bytes, tracker->total_chunks);
}

void alloc_tracker_log_current_gen(alloc_tracker_t *tracker, size_t gen)
{
	alloc_node_t *node = tracker->head;
	log_info("memory generation %lu:", gen);
 	while (node) {
		if (node->generation == gen)
			log_info("%p: %10lu bytes from %s", node + 1, node->sz, node->location);
 		node = node->next;
 	}
}

void *tracked_malloc(size_t sz, allocator_t *a  MEMCALL_ARGS)
{
	alloc_node_t *node = std_malloc(sizeof(alloc_node_t) + sz);
	alloc_tracker__append_node(a->udata, node, sz  MEMCALL_VARS);
	log_alloc("std", sz  MEMCALL_VARS);
	return node + 1;
}

void *tracked_calloc(size_t nmemb, size_t sz, allocator_t *a  MEMCALL_ARGS)
{
	alloc_node_t *node = std_calloc(1, sizeof(alloc_node_t) + nmemb * sz);
	alloc_tracker__append_node(a->udata, node, nmemb * sz  MEMCALL_VARS);
	log_alloc("std", nmemb * sz  MEMCALL_VARS);
	return node + 1;
}

void *tracked_realloc(void *ptr, size_t sz, allocator_t *a  MEMCALL_ARGS)
{
	alloc_tracker_t *tracker = a->udata;
	if (ptr) {
		alloc_node_t *old_node = (alloc_node_t*)ptr - 1;
		alloc_tracker__record_free(tracker, old_node->sz);
		if (sz) {
			alloc_node_t *node = std_realloc(old_node, sizeof(alloc_node_t) + sz);
			alloc_tracker__record_alloc(tracker, sz);
			log_alloc("std", sz  MEMCALL_VARS);
			node->sz = sz;
			node->generation = tracker->generation;
#ifdef VLT_TRACK_MEMORY
			node->location = loc;
#else
			node->location = "(unknown)";
#endif
			if (node != old_node) {
				if (node->prev)
					node->prev->next = node;
				if (node->next)
					node->next->prev = node;
				if (tracker->head == old_node)
					tracker->head = node;
				if (tracker->tail == old_node)
					tracker->tail = node;
			}
			return node + 1;
		} else {
			tracked_free(ptr, a  MEMCALL_VARS);
			return NULL;
		}
	} else if (sz) {
		return tracked_malloc(sz, a  MEMCALL_VARS);
	} else {
		return NULL;
	}
}

void tracked_free(void *ptr, allocator_t *a  MEMCALL_ARGS)
{
	if (ptr) {
		alloc_tracker_t *tracker = a->udata;
		alloc_node_t *node = (alloc_node_t*)ptr - 1;
		if (node->prev)
			node->prev->next = node->next;
		if (node->next)
			node->next->prev = node->prev;
		if (tracker->head == node)
			tracker->head = node->next;
		if (tracker->tail == node)
			tracker->tail = node->prev;
		alloc_tracker__record_free(a->udata, node->sz);
		std_free(node);
	}
}

void vlt_mem_advance_gen(void)
{
#ifdef VLT_TRACK_MEMORY
	alloc_tracker_advance_gen(g_allocator->udata);
#endif
}

static
void vlt_mem_log_usage_(u32 temp_page_cnt)
{
#ifdef VLT_TRACK_MEMORY
	alloc_tracker_log_usage(g_allocator->udata);
#endif
	log_info("temp:  %10lu bytes in %u pages", temp_page_cnt * PGB_PAGE_SIZE,
	         temp_page_cnt);
}

void vlt_mem_log_usage(void)
{
	const pgb_data_t *data = g_temp_allocator->udata;
	vlt_mem_log_usage_(list_sz(data->pages));
}


/* Hash (djb2 by dan bernstein) */

u32 hash(const char *str)
{
	u32 hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

u32 hashn(const char *str, u32 n)
{
	u32 hash = 5381;
	for (u32 i = 0; i < n; ++i)
		hash = ((hash << 5) + hash) + (int)str[i];
	return hash;
}

/* Time */

#ifndef _WIN32

timepoint_t time_current()
{
	timepoint_t t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t;
}

static
timepoint_t time__diff(timepoint_t start, timepoint_t end)
{
	timepoint_t mod_start, res;

	mod_start.tv_nsec = start.tv_nsec;
	mod_start.tv_sec = start.tv_sec;
	if (end.tv_nsec < mod_start.tv_nsec) {
		mod_start.tv_nsec -= 1000000000;
		++mod_start.tv_sec;
	}

	/* C standard states that tv_nsec should be [0, 999,999,999] */
	error_if(end.tv_nsec - mod_start.tv_nsec >= 1000000000, "invalid nanosec diff");
	error_if(end.tv_sec < mod_start.tv_sec, "end is before start");
	res.tv_sec = end.tv_sec - mod_start.tv_sec;
	res.tv_nsec = end.tv_nsec - mod_start.tv_nsec;
	return res;
}

u32 time_diff_milli(timepoint_t start, timepoint_t end)
{
	timepoint_t res = time__diff(start, end);
	return res.tv_sec * 1000 + res.tv_nsec / 1000000;
}

u32 time_diff_micro(timepoint_t start, timepoint_t end)
{
	timepoint_t res = time__diff(start, end);
	return res.tv_sec * 1000000 + res.tv_nsec / 1000;
}

void time_sleep_milli(u32 milli)
{
	timepoint_t t = { .tv_nsec = milli * 1000000 };
	nanosleep(&t, NULL);
}

#else

timepoint_t time_current()
{
	timepoint_t t;
	QueryPerformanceCounter(&t);
	return t;
}

u32 time_diff_milli(timepoint_t start, timepoint_t end)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (u32)((end.QuadPart - start.QuadPart) * 1000 / frequency.QuadPart);
}

u32 time_diff_micro(timepoint_t start, timepoint_t end)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return (u32)((end.QuadPart - start.QuadPart) * 1000000 / frequency.QuadPart);
}

void time_sleep_milli(u32 milli)
{
	Sleep(milli);
}

#endif

/* Log */

typedef struct log_stream
{
	logger_t logger;
	log_level_t level;
	void *udata;
} log_stream_t;

log_stream_t g_log_streams[LOG_STREAM_CAP];
u32 g_log_stream_cnt = 0;

void log_add_stream(log_level_t level, logger_t logger, void *udata)
{
	assert(g_log_stream_cnt < LOG_STREAM_CAP);
	g_log_streams[g_log_stream_cnt].logger = logger;
	g_log_streams[g_log_stream_cnt].level = level;
	g_log_streams[g_log_stream_cnt].udata = udata;
	++g_log_stream_cnt;
}

void log_remove_stream(logger_t logger, void *udata)
{
	for (u32 i = 0; i < g_log_stream_cnt; ++i) {
		if (logger == g_log_streams[i].logger && udata == g_log_streams[i].udata) {
			g_log_streams[i] = g_log_streams[g_log_stream_cnt-1];
			--g_log_stream_cnt;
			return;
		}
	}
	assert(false);
}

void log_level_write(log_level_t level, const char *format, ...)
{
	for (u32 i = 0; i < g_log_stream_cnt; ++i) {
		if (level & g_log_streams[i].level) {
			va_list ap;
			va_start(ap, format);
			g_log_streams[i].logger(g_log_streams[i].udata, level, format, ap);
			va_end(ap);
		}
	}
}

void file_logger(void *udata, log_level_t level, const char *format, va_list ap)
{
	FILE *fp = udata;
	switch (level) {
	case LOG_DEBUG:
		fputs("[DEBUG] ", fp);
	break;
	case LOG_INFO:
		fputs("[INFO ] ", fp);
	break;
	case LOG_WARNING:
		fputs("[WARN ] ", fp);
	break;
	case LOG_ERROR:
		fputs("[ERROR] ", fp);
	break;
	case LOG_FATAL:
		fputs("[FATAL] ", fp);
	break;
	default:
		fputs("[XXXXX] ", fp);
	break;
	}
	vfprintf(fp, format, ap);
	fputc('\n', fp);
	fflush(fp);
}

/* Runtime */

static
void vlt_init(void)
{
	pgb_data__init(g_temp_allocator->udata);
}

static
void vlt_destroy(void)
{
	pgb_data_t *data = g_temp_allocator->udata;
	const u32 temp_page_cnt = list_sz(data->pages);
	pgb_data__destroy(data);
	vlt_mem_log_usage_(temp_page_cnt);
}

int app_entry(int argc, char *const argv[]);
int main(int argc, char *const argv[])
{
	vlt_init();
	app_entry(argc, argv);
	vlt_destroy();
}

#define main app_entry

#undef CORE_IMPLEMENTATION
#endif // CORE_IMPLEMENTATION
