#if defined(__linux__)

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define CPUINFO_LINE_LENGTH 128

typedef struct cpuid_flags_t {
	unsigned long processor_flags;
	unsigned long feature_flags;
	unsigned long arch_flags;
	unsigned long part_flags;
} cpuid_flags_t;

typedef struct cpuid_flag_table_t {
	const char *str;
	unsigned long flag;
} cpuid_flag_table_t;

static const cpuid_flag_table_t parts[] = {
	{"0xb02", CPUID_ARMv6}, /* "armv6k"   - "mpcore" */
	{"0xb36", CPUID_ARMv6}, /* "armv6j"   - "arm1136j-s" */
	{"0xb56", CPUID_ARMv6}, /* "armv6t2"  - "arm1156t2-s" */
	{"0xb76", CPUID_ARMv6}, /* "armv6zk"  - "arm1176jz-s" */
	{"0xc05", CPUID_ARMv7}, /* "armv7-a"  - "cortex-a5" */
	{"0xc07", CPUID_ARMv7}, /* "armv7ve"  - "cortex-a7" */
	{"0xc08", CPUID_ARMv7}, /* "armv7-a"  - "cortex-a8" */
	{"0xc09", CPUID_ARMv7}, /* "armv7-a"  - "cortex-a9" */
	{"0xc0d", CPUID_ARMv7}, /* "armv7ve"  - "cortex-a12" */
	{"0xc0f", CPUID_ARMv7}, /* "armv7ve"  - "cortex-a15" */
	{"0xc14", CPUID_ARMv7}, /* "armv7-r"  - "cortex-r4" */
	{"0xc15", CPUID_ARMv7}, /* "armv7-r"  - "cortex-r5" */
	{"0xc20", CPUID_ARMv6}, /* "armv6-m"  - "cortex-m0" */
	{"0xc21", CPUID_ARMv6}, /* "armv6-m"  - "cortex-m1" */
	{"0xc23", CPUID_ARMv7}, /* "armv7-m"  - "cortex-m3" */
	{"0xc24", CPUID_ARMv7}, /* "armv7e-m" - "cortex-m4" */
	{"0xc60", CPUID_ARMv6}, /* "armv6-m"  - "cortex-m0+" */
	{"0xd03", CPUID_ARMv8}, /* "armv8-a"  - "cortex-a53" */
	{"0xd07", CPUID_ARMv8}  /* "armv8-a"  - "cortex-a57" */
};

static const cpuid_flag_table_t features[] = {
	{"tls ",       CPUID_TLS},
	{"aes ",       CPUID_AES},
	{"pmull ",     CPUID_PMULL},
	{"sha1 ",      CPUID_SHA1},
	{"sha2 ",      CPUID_SHA2},
	{"crc32 ",     CPUID_CRC32},
	{"iwmmxt ",    CPUID_IWMMXT},
	{"idivt ",     CPUID_IDIVT},
	{"idiva ",     CPUID_IDIVA},
	{"vfpv3d16 ",  CPUID_VFP3D16},
	{"vfpv3 ",     CPUID_VFP3},
	{"vfpv4 ",     CPUID_VFP4},
	{"neon ",      CPUID_NEON},
	{"asimd ",     CPUID_ASIMD}
};

/* that's an L, not a 1?? */
static const cpuid_flag_table_t processors[] = {
	{"(v6l)",      CPUID_ARMv6},
	{"(v7l)",      CPUID_ARMv7},
	{"(v8l)",      CPUID_ARMv8}
};

static const cpuid_flag_table_t archs[] = {
	{"5",   CPUID_ARM  },
	{"6",   CPUID_ARMv6},
	{"7",   CPUID_ARMv7},
	{"8",   CPUID_ARMv8}
};

static unsigned long
cpuid_scan(const char *line, const cpuid_flag_table_t *table, size_t table_count) {
	unsigned long flags = 0;
	size_t i;

	for (i = 0; i < table_count; i++) {
		if (strstr(line, table[i].str) != NULL)
			flags |= table[i].flag;
	}

	return flags;
}

/* flags: [processor, feature, arch, part] */
static void
cpuid_line_parse(const char *line, cpuid_flags_t *flags) {
	if (strncmp(line, "Processor", 9) == 0)
		flags->processor_flags |= cpuid_scan(line, processors, sizeof(processors) / sizeof(cpuid_flag_table_t));
	else if (strncmp(line, "Features", 8) == 0)
		flags->feature_flags |= cpuid_scan(line, features, sizeof(features) / sizeof(cpuid_flag_table_t));
	else if (strncmp(line, "CPU architecture", 16) == 0)
		flags->arch_flags |= cpuid_scan(line, archs, sizeof(archs) / sizeof(cpuid_flag_table_t));
	else if (strncmp(line, "CPU part", 8) == 0)
		flags->part_flags |= cpuid_scan(line, parts, sizeof(parts) / sizeof(cpuid_flag_table_t));
}

static int
cpuid_line_length(char *line, int start, int end) {
	int i;
	for (i = start; i < end; i++) {
		if (line[i] == '\n') {
			line[i] = 0;
			return i - start;
		}
	}
	return -1;
}

static unsigned long
cpuid_specific_impl(void) {
	cpuid_flags_t flags = {0, 0, 0, 0};
	int skip_to_next_line = 0;
	int num_old_bytes = 0;

	char line[CPUINFO_LINE_LENGTH];
	int fd;

	fd = open("/proc/cpuinfo", O_RDONLY);
	if (fd < 0)
		goto cpuid_specific_impl_done;

	for (;;) {
		int cur_line_pos = 0;
		int bytes = read(fd, line + num_old_bytes, CPUINFO_LINE_LENGTH - num_old_bytes);
		if (bytes <= 0) {
			if ((bytes < 0) && (errno == EINTR))
				continue;
			goto cpuid_specific_impl_done;
		}

		bytes += num_old_bytes;
		num_old_bytes = 0;
		while (bytes) {
			int line_length = cpuid_line_length(line, cur_line_pos, bytes);

			/* if the line extends past the buffer.. */
			if (line_length < 0) {
				if (cur_line_pos == 0) {
					/* and it's larger than our buffer, skip it */
					skip_to_next_line = 1;
				} else {
					/* otherwise copy it to the front */
					memmove(line, line + cur_line_pos, CPUINFO_LINE_LENGTH - cur_line_pos);
					num_old_bytes = CPUINFO_LINE_LENGTH - cur_line_pos;
					line[num_old_bytes] = 0;
					cur_line_pos = 0;
				}

				/* break out and read more */
				break;
			}

			/* found the end of a line, are we skipping until a new line? */
			if (!skip_to_next_line)
				cpuid_line_parse(line + cur_line_pos, &flags);
			else
				skip_to_next_line = 0;

			cur_line_pos += line_length + 1;
			bytes -= line_length + 1;

			if (cur_line_pos == CPUINFO_LINE_LENGTH) {
				cur_line_pos = 0;
			}
		}
	}

cpuid_specific_impl_done:
	if (fd != -1)
		close(fd);

	/* only check processor & arch if we don't have an exact identification */
	if (!flags.part_flags) {
		/* work around https://code.google.com/p/android/issues/detail?id=10812 */
		if (flags.arch_flags) {
			flags.processor_flags = 
				((flags.arch_flags & CPUID_ARMv7) && (flags.processor_flags & CPUID_ARMv6)) ?
					flags.processor_flags :
					flags.arch_flags;
		}
	} else {
		flags.processor_flags = flags.part_flags;
	}

	return CPUID_ARM | flags.processor_flags | flags.feature_flags;
}

#endif
