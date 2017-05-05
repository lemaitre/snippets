/**
 * @file rdstc.h
 *
 * read the time stamp counter register (or equivalent)
 *
 * @author Manuel Schiller
 * @date 2012-01-27
 */
#ifndef RDTSC_H
#define RDTSC_H

// KNC doesn't support RDTSCP instruction: fallback on CPUID + RDTSC
#ifndef __KNC__
#define RDTSCP
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline __attribute((always_inline)) void reset_tsc(void) {
#ifdef __arm__
  __asm__ volatile (
      "MCR p15, 0, %0, c9, c12, 0\n\t"
      "MCR p15, 0, %1, c9, c12, 1\n\t"
      "MCR p15, 0, %2, c9, c12, 3\n\t"
      :: "r"(1|2|4|16), "r"(0x8000000ful), "r"(0x8000000ful)
  );
#endif
}

/// return the time stamp counter register (or equivalent)
static inline __attribute((always_inline))  unsigned long long rdtsc(void)
{
    unsigned long long int x;
#if defined(__i386__)
#ifdef RDTSCP
    __asm__ volatile (
	    "xor %%eax, %%eax\n"
	    "rdtscp" : "=A" (x) :: "ebx", "ecx", "memory"); // TSC in edx:eax
#else
    __asm__ volatile (
	    "xor %%eax, %%eax\n"
	    "cpuid\n"
	    "rdtsc" : "=A" (x) :: "ebx", "ecx", "memory"); // TSC in edx:eax
#endif
#elif defined(__x86_64__)
#ifdef RDTSCP
    register unsigned long lo, hi;
    __asm__ volatile (
	    "xor %%eax, %%eax\n"
	    "rdtscp\n"             // TSC in edx:eax
	    : "=a" (lo), "=d" (hi) :: "ebx", "ecx", "memory");
    x = (unsigned long long) lo | ((unsigned long long) hi << 32);
#else
    register unsigned long lo, hi;
    __asm__ volatile (
	    "xor %%eax, %%eax\n"
	    "cpuid\n"
	    "rdtsc\n"             // TSC in edx:eax
	    : "=a" (lo), "=d" (hi) :: "ebx", "ecx", "memory");
    x = (unsigned long long) lo | ((unsigned long long) hi << 32);
#endif
#elif defined(__powerpc__)
    register unsigned long hi, lo, tmp;
    __asm__ volatile(
	    "0:\n"
	    "mftbu %[hi]\n"        // read high word time base register
	    "mftb  %[lo]\n"        // read low word time base register
	    "mftbu %[tmp]\n"       // read high word time base register again
	    "cmpw  %[tmp],%[hi]\n" // make sure that we did not overflow
	    "bne   0b\n"           // if we did, try again...
	    : [hi]"=r"(hi), [lo]"=r"(lo), [tmp]"=r"(tmp) :: "memory");
    x = (((unsigned long long) hi) << 32) | ((unsigned long long) lo);
#elif defined(__sparc__) || defined(__sparc64__)
#ifdef __arch64__
    __asm__ volatile ("rd %%tick,%0" : "=r" (x) :: "memory");
#else
    __asm__ volatile (
	    "rd %%tick,%g1\n"
	    "stx %%g1,%0"
	    : "=m" (x) :: "g1", "memory");
#endif
#elif defined(__arm__)
    unsigned xx;
  __asm__ volatile (
      "MRC p15, 0, %0, c9, c13, 0"
      : "=r" (xx)
  );
  x = xx;
#else
#error "Unknown architecture!"
#endif
    return x;
}

#ifdef __cplusplus
}
#endif

#endif // RDTSC_H

// vim: sw=4:tw=78:ft=c
