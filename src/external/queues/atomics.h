#ifndef ATOMICS_H
#define ATOMICS_H

/* Atomic fetch-and-add. */
#define ATOMIC_FAA(ptr, val) __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED)

#define ATOMIC_INC(ptr)     ATOMIC_FAA(ptr, 1)

/* An atomic fetch-and-add that also ensures sequential consistency. */
#define ATOMIC_FAA_CST(ptr, val) __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST)

/* Atomic compare-and-swap. */
#define CAS(ptr, cmp, val)     __atomic_compare_exchange_n(ptr, cmp, val, 0,  __ATOMIC_RELAXED, __ATOMIC_RELAXED)

/* atomic compare-and-swap that also ensures sequential consistency. */
#define CAS_CST(ptr, cmp, val) __atomic_compare_exchange_n(ptr, cmp, val, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
/* atomic compare-and-swap that ensures release semantic when succeed or acquire semantic when failed. */
#define CASra(ptr, cmp, val)  __atomic_compare_exchange_n(ptr, cmp, val, 0,  __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)
/* atomic compare-and-swap that ensures acquire semantic when succeed or relaxed semantic when failed. */
#define CASa(ptr, cmp, val)   __atomic_compare_exchange_n(ptr, cmp, val, 0,  __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)

/* Atomic Store val to *ptr */
#define ATOMIC_EXCHANGE(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_RELAXED)

/**
 * An atomic swap that ensures acquire release semantics.
 */
#define SWAPra(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_ACQ_REL)

/**
 * A memory fence to ensure sequential consistency.
 */
#define ATOMIC_FENCE() __atomic_thread_fence(__ATOMIC_SEQ_CST)

/**
 * An atomic store.
 */
#define ATOMIC_STORE(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_RELAXED)

/**
 * A store with a preceding release fence to ensure all previous load
 * and stores completes before the current store is visiable.
 */
#define RELEASE(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_RELEASE)

/**
 * A load with a following acquire fence to ensure no following load and
 * stores can start before the current load completes.
 */
#define ACQUIRE(ptr) __atomic_load_n(ptr, __ATOMIC_ACQUIRE)

#define PAUSE() __asm__ ("pause")

static inline
int _CAS2(volatile long * ptr, long * cmp1, long * cmp2, long val1, long val2)
{
  char success;
  long tmp1 = *cmp1;
  long tmp2 = *cmp2;

  __asm__ __volatile__(
      "lock cmpxchg16b %1\n"
      "setz %0"
      : "=q" (success), "+m" (*ptr), "+a" (tmp1), "+d" (tmp2)
      : "b" (val1), "c" (val2)
      : "cc" );

  *cmp1 = tmp1;
  *cmp2 = tmp2;
  return success;
}
#define CAS128(p, o1, o2, n1, n2) \
  _CAS2((volatile long *) p, (long *) o1, (long *) o2, (long) n1, (long) n2)

#define BTAS(ptr, bit) ({ \
  char __ret; \
  __asm__ __volatile__( \
      "lock btsq %2, %0; setnc %1" \
      : "+m" (*ptr), "=r" (__ret) : "ri" (bit) : "cc" ); \
  __ret; \
})

#endif
