#ifndef JO_INCLUDE_JPEG_H
#define JO_INCLUDE_JPEG_H

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Returns false on failure
extern bool jo_write_jpg(const char *filename, const void *data, int width, int height, int comp, int quality);

#ifdef __cplusplus
}
#endif

#endif // JO_INCLUDE_JPEG_H