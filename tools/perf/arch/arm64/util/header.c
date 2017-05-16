#include <stdio.h>
#include <stdlib.h>
#include <api/fs/fs.h>
#include "header.h"

#define MIDR "/regs/identification/midr_el1"
#define MIDR_SIZE 128

char *get_cpuid_str(struct perf_pmu *pmu)
{
	char *buf = malloc(MIDR_SIZE);
	char *temp = NULL;
	char path[PATH_MAX];
	const char *sysfs = sysfs__mountpoint();
	int cpu, ret;
	unsigned long long midr;
	struct cpu_map *cpus;
	FILE *file;

	if (!pmu->cpus)
		return NULL;

	if (!sysfs)
		return NULL;

	/* read midr from list of cpus mapped to this pmu */
	cpus = cpu_map__get(pmu->cpus);
	for (cpu = 0; cpu < cpus->nr; cpu++) {
		ret = snprintf(path, PATH_MAX,
				"%s/devices/system/cpu/cpu%d"MIDR,
				sysfs, cpus->map[cpu]);
		if (ret == PATH_MAX) {
			pr_err("sysfs path crossed PATH_MAX(%d) size\n", PATH_MAX);
			goto err;
		}

		file = fopen(path, "r");
		if (!file)
			continue;

		temp = fgets(buf, MIDR_SIZE, file);
		fclose(file);
		if (!temp)
			continue;

		/* Ignore/clear Variant[23:20] and
		 * Revision[3:0] of MIDR
		 */
		midr = strtoll(buf, NULL, 16);
		midr &= (~(0xf << 20 | 0xf));
		snprintf(buf, MIDR_SIZE, "0x%016llx", midr);
		cpu_map__put(cpus);
		return buf;
	}

err:	cpu_map__put(cpus);
	free(buf);
	return NULL;
}
