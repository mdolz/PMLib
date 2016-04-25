/* Basic access functions.  */
# define __BYTE 8
# define __LINELT(line)  ((line) / __BYTE)
# define __LINEMASK(line) ((__line_mask) 1 << ((line) % __BYTE))

/* Access functions for LINE masks.  */ 
#  define __LINE_CLR_ALL_S(setsize, linesetp)                                    \
  do {                                                                        \
    size_t __i;                                                               \
    size_t __imax = (setsize) / sizeof (__line_mask);                         \
    __line_mask *__bits = (linesetp)->__bits;                                 \
    for (__i = 0; __i < __imax; ++__i)                                        \
      __bits[__i] = 0x00;                                                     \
  } while (0)

#  define __LINE_SET_ALL_S(setsize, linesetp)                                     \
  do {                                                                        \
    size_t __i;                                                               \
    size_t __imax = (setsize) / sizeof (__line_mask);                         \
    __line_mask *__bits = (linesetp)->__bits;                                 \
    for (__i = 0; __i < __imax; ++__i)                                        \
      __bits[__i] = 0xff;                                                     \
  } while (0)

# define __LINE_SET_S(line, setsize, linesetp)                                \
  (__extension__                                                              \
   ({ size_t __line = (line);                                                 \
      __line < __BYTE * (setsize)                                             \
      ? (((__line_mask *) ((linesetp)->__bits))[__LINELT (__line)]            \
         |= __LINEMASK (__line))                                              \
      : 0; }))

# define __LINE_CLR_S(line, setsize, linesetp)                                \
  (__extension__                                                              \
   ({ size_t __line = (line);                                                 \
      __line < __BYTE * (setsize)                                             \
      ? (((__line_mask *) ((linesetp)->__bits))[__LINELT (__line)]            \
         &= ~__LINEMASK (__line))                                             \
      : 0; }))

# define __LINE_ISSET_S(line, setsize, linesetp)                              \
  (__extension__                                                              \
   ({ size_t __line = (line);                                                 \
      __line < __BYTE * (setsize)                                             \
      ? ((((__const __line_mask *) ((linesetp)->__bits))[__LINELT (__line)]   \
          & __LINEMASK (__line))) != 0                                        \
      : 0; }))

# define __LINE_OP_S(setsize, destset, srcset1, srcset2, op)                  \
  (__extension__                                                              \
   ({ __const __line_mask *__arr1 = (srcset1).__bits;                         \
      __const __line_mask *__arr2 = (srcset2).__bits;                         \
      __line_mask *__dest = (destset)->__bits;                                \
      size_t __imax = (setsize) / sizeof (__line_mask);                       \
      size_t __i;                                                             \
      for (__i = 0; __i < __imax; ++__i)                                      \
        __dest[__i] = __arr1[__i] op __arr2[__i];                             \
      __dest; }))

/* Access macros for 'line_set'.  */
# define LINE_SETSIZE __LINE_SETSIZE
# define LINE_SET(line, linesetp)   __LINE_SET_S (line, sizeof (line_t), linesetp)
# define LINE_CLR(line, linesetp)   __LINE_CLR_S (line, sizeof (line_t), linesetp)
# define LINE_ISSET(line, linesetp) __LINE_ISSET_S (line, sizeof (line_t), \
                                                linesetp)
# define LINE_CLR_ALL(linesetp)       __LINE_CLR_ALL_S (sizeof (line_t), linesetp)
# define LINE_SET_ALL(linesetp)       __LINE_SET_ALL_S (sizeof (line_t), linesetp)

# define LINE_AND(destset, srcset1, srcset2) \
  __LINE_OP_S (sizeof (line_t), destset, srcset1, srcset2, &)

