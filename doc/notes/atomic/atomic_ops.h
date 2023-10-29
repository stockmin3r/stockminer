// https://github.com/ivmai/libatomic_ops

/* We define various atomic operations on memory in a           */
/* machine-specific way.  Unfortunately, this is complicated    */
/* by the fact that these may or may not be combined with       */
/* various memory barriers.  Thus the actual operations we      */
/* define have the form AO_<atomic-op>_<barrier>, for all       */
/* plausible combinations of <atomic-op> and <barrier>.         */
/* This of course results in a mild combinatorial explosion.    */
/* To deal with it, we try to generate derived                  */
/* definitions for as many of the combinations as we can, as    */
/* automatically as possible.                                   */
/*                                                              */
/* Our assumption throughout is that the programmer will        */
/* specify the least demanding operation and memory barrier     */
/* that will guarantee correctness for the implementation.      */
/* Our job is to find the least expensive way to implement it   */
/* on the applicable hardware.  In many cases that will         */
/* involve, for example, a stronger memory barrier, or a        */
/* combination of hardware primitives.                          */
/*                                                              */
/* Conventions:                                                 */
/* "plain" atomic operations are not guaranteed to include      */
/* a barrier.  The suffix in the name specifies the barrier     */
/* type.  Suffixes are:                                         */
/* _release: Earlier operations may not be delayed past it.     */
/* _acquire: Later operations may not move ahead of it.         */
/* _read: Subsequent reads must follow this operation and       */
/*        preceding reads.                                      */
/* _write: Earlier writes precede both this operation and       */
/*        later writes.                                         */
/* _full: Ordered with respect to both earlier and later memory */
/*        operations.                                           */
/* _release_write: Ordered with respect to earlier writes.      */
/* _acquire_read: Ordered with respect to later reads.          */
/*                                                              */
/* Currently we try to define the following atomic memory       */
/* operations, in combination with the above barriers:          */
/* AO_nop                                                       */
/* AO_load                                                      */
/* AO_store                                                     */
/* AO_test_and_set (binary)                                     */
/* AO_fetch_and_add                                             */
/* AO_fetch_and_add1                                            */
/* AO_fetch_and_sub1                                            */
/* AO_and                                                       */
/* AO_or                                                        */
/* AO_xor                                                       */
/* AO_compare_and_swap                                          */
/* AO_fetch_compare_and_swap                                    */
/*                                                              */
/* Note that atomicity guarantees are valid only if both        */
/* readers and writers use AO_ operations to access the         */
/* shared value, while ordering constraints are intended to     */
/* apply all memory operations.  If a location can potentially  */
/* be accessed simultaneously from multiple threads, and one of */
/* those accesses may be a write access, then all such          */
/* accesses to that location should be through AO_ primitives.  */
/* However if AO_ operations enforce sufficient ordering to     */
/* ensure that a location x cannot be accessed concurrently,    */
/* or can only be read concurrently, then x can be accessed     */
/* via ordinary references and assignments.                     */
/*                                                              */
/* AO_compare_and_swap takes an address and an expected old     */
/* value and a new value, and returns an int.  Non-zero result  */
/* indicates that it succeeded.                                 */
/* AO_fetch_compare_and_swap takes an address and an expected   */
/* old value and a new value, and returns the real old value.   */
/* The operation succeeded if and only if the expected old      */
/* value matches the old value returned.                        */
/*                                                              */
/* Test_and_set takes an address, atomically replaces it by     */
/* AO_TS_SET, and returns the prior value.                      */
/* An AO_TS_t location can be reset with the                    */
/* AO_CLEAR macro, which normally uses AO_store_release.        */
/* AO_fetch_and_add takes an address and an AO_t increment      */
/* value.  The AO_fetch_and_add1 and AO_fetch_and_sub1 variants */
/* are provided, since they allow faster implementations on     */
/* some hardware. AO_and, AO_or, AO_xor do atomically and, or,  */
/* xor (respectively) an AO_t value into a memory location,     */
/* but do not provide access to the original.                   */
/*                                                              */
/* We expect this list to grow slowly over time.                */
/*                                                              */
/* Note that AO_nop_full is a full memory barrier.              */
/*                                                              */
/* Note that if some data is initialized with                   */
/*      data.x = ...; data.y = ...; ...                         */
/*      AO_store_release_write(&data_is_initialized, 1)         */
/* then data is guaranteed to be initialized after the test     */
/*      if (AO_load_acquire_read(&data_is_initialized)) ...     */
/* succeeds.  Furthermore, this should generate near-optimal    */
/* code on all common platforms.                                */
/*                                                              */
/* All operations operate on unsigned AO_t, which               */
/* is the natural word size, and usually unsigned long.         */
/* It is possible to check whether a particular operation op    */
/* is available on a particular platform by checking whether    */
/* AO_HAVE_op is defined.  We make heavy use of these macros    */
/* internally.                                                  */

/* The rest of this file basically has three sections:          */
/*                                                              */
/* Some utility and default definitions.                        */
/*                                                              */
/* The architecture dependent section:                          */
/* This defines atomic operations that have direct hardware     */
/* support on a particular platform, mostly by including the    */
/* appropriate compiler- and hardware-dependent file.           */
/*                                                              */
/* The synthesis section:                                       */
/* This tries to define other atomic operations in terms of     */
/* those that are explicitly available on the platform.         */
/* This section is hardware independent.                        */
/* We make no attempt to synthesize operations in ways that     */
/* effectively introduce locks, except for the debugging/demo   */
/* pthread-based implementation at the beginning.  A more       */
/* realistic implementation that falls back to locks could be   */
/* added as a higher layer.  But that would sacrifice           */
/* usability from signal handlers.                              */
/* The synthesis section is implemented almost entirely in      */
/* atomic_ops/generalize.h.                                     */
