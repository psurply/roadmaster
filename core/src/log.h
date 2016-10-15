#ifndef LOG_H
#define LOG_H

#ifdef DEBUG
#   define _DEBUG(fmt, ...) \
    fprintf(stderr, "%s:%d: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define _DEBUG(fmt, args...)
#endif

#endif /* LOG_H */
