#include <jni.h>
#include <limits.h>

#include "agram_wc.h"
#include "is_within.h"
#include "lcwc.h"

int words_from(jchar const * const str, size_t const len, int const max, int (* const cb)(jchar const *, size_t, void *), void * const cba)
{
  jsize i;
  struct wc target;
  if (wc_init(&target, str, len))
    return 1;
  if (max)
    for (i = 0; i < target.nchars; i++)
      target.counts[i] = UINT_MAX;
  for (i = 0; i < NWORDS; i++)
    if (is_within_lw(words_counts+i, &target))
      if (cb(words_counts[i].str + strbase, words_counts[i].len, cba))
        return 2;
  return 0;
}
