/*
 * cpufreq-hipv660.c - CPUfreq for PhosphorV660
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#define DEBUG

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>

#include <linux/io.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#define SC_SIZE			0x10000

#define SC_CLK_SEL		0x3048
#define SC_PLLFCTRL0		0x3014
#define SC_PLLFCTRL1		0x3018
#define SC_TSMC_PLLCTRL1	0x3204

#define SC_SYSMODE_CTRL		0x3000
#define SC_SYSMODE_STATUS	0xC000
#define SC_PLL_LOCK_STATUS	0xC004

#define FBDIV_SBIT		6
#define PDIV1_SBIT		18
#define PDIV2_SBIT		21
#define DSMPD_SBIT		26

#define NF_SBIT			5
#define NO_SBIT			15

#define SYSMODE_SLOW		0x2
#define SYSMODE_NORMAL		0x4

#define REFDIV_FLAG		0x3F
#define FBDIV_FLAG		0x3FFC0
#define PDIV1_FLAG		0x1C0000
#define PDIV2_FLAG		0xE00000
#define PD_FLAG			0x2000000
#define DSMPD_FLAG		0x4000000

#define NR_FLAG			0x1F
#define NF_FLAG			0x1FE0
#define NO_FLAG			0x38000

#define FCTRL1_OFLAG		0x1000000
#define FCTRL1_SFLAG		0x2000000
#define FCTRL1_EFLAG		0x3000000

#define TA_CPU_FLAG		0x10000
#define SC_C_BASE		0x80000000

#define CLEAN_FBDIV		0xFFFC003F
#define CLEAN_DSMPD		0xFBFFFFFF

#define SYS_CLKIN	50

/* sysctrl reg base address */
struct hipv660_cpufreq_priv {
	void __iomem *a_base;
	void __iomem *c_base;
	void __iomem *ta_base;
	unsigned int power_off;
};

static struct hipv660_cpufreq_priv *hip_freq_priv;

static int pll_flag;
static int freq_type_400;
static int freq_type_100;

static struct cpufreq_frequency_table *hipv660_freq_table;

static struct cpufreq_frequency_table hipv660_freq_table0[] = {
	{0, 0, 2100 * 1000},
	{0, 1, 1600 * 1000},
	{0, 2, 1050 * 1000},
	{0, 3,	600 * 1000},
	{0, 4,	400 * 1000},
	{0, 5,	100 * 1000},
	{0, 6,  CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table hipv660_freq_table1[] = {
	{0, 0, 2100 * 1000},
	{0, 1,	400 * 1000},
	{0, 2,	100 * 1000},
	{0, 3, CPUFREQ_TABLE_END},
};

static DEFINE_PER_CPU(unsigned int, cur_index);
static DEFINE_PER_CPU(unsigned int, master_cpu);


static struct cpufreq_driver hipv660_cpufreq_driver;

void hipv660_cpufreq_set_power_off(unsigned int val)
{
	hip_freq_priv->power_off = val;
}
EXPORT_SYMBOL(hipv660_cpufreq_set_power_off);

static bool check_master_pll(void __iomem *base)
{
	u32 sc_clk_sel = readl_relaxed(base + SC_CLK_SEL);

	return !(sc_clk_sel & 0x1);
}

static u32 read_pll_fctrl0(void __iomem *base)
{
	return readl_relaxed(base + SC_PLLFCTRL0);
}

static u32 read_tsmc_pll_fctrl1(void __iomem *base)
{
	return readl_relaxed(base + SC_TSMC_PLLCTRL1);
}

static bool check_pll_lock_status(void)
{
	u32 lock_status = readl_relaxed(hip_freq_priv->ta_base + SC_PLL_LOCK_STATUS);

	return (lock_status & 0x1d) != 0x1d;
}

static bool check_pll1_lock_status(void)
{
	u32 lock_status = readl_relaxed(hip_freq_priv->ta_base + SC_PLL_LOCK_STATUS);

	return (lock_status & 0x2) != 0x2;
}

static void sys_mode_change(u32 val, void __iomem *base)
{
	writel_relaxed(val, base + SC_SYSMODE_CTRL);
}

static bool check_mode_change(u32 val, void __iomem *base)
{
	u32 change_status = readl_relaxed(base + SC_SYSMODE_STATUS);

	return (change_status & 0xF) != val;
}

static void write_pll_fctrl0(u32 val, void __iomem *base)
{
	writel_relaxed(val, base + SC_PLLFCTRL0);
}

static void write_tsmc_pll_fctrl1(u32 val, void __iomem *base)
{
	writel_relaxed(val, base + SC_TSMC_PLLCTRL1);
}

static void write_pll_fctrl1(u32 val, void __iomem *base)
{
	writel_relaxed(val, base + SC_PLLFCTRL1);
}

static void do_write_pll(u32 val, void __iomem *base)
{
	/* step 1 set ctrl1 to 0x2000000 */
	write_pll_fctrl1(FCTRL1_SFLAG, base);

	/* step 2 set fbdiv to ctrl0 */
	write_pll_fctrl0(val, base);

	/* step 3 set ctrl2 to 0x3000000 */
	write_pll_fctrl1(FCTRL1_EFLAG, base);

	/* step 4 make sure the pll0 lock */
	while (check_pll_lock_status())
		;

	udelay(1);
}

static bool check_ta_cpu(unsigned int cpu)
{
	struct device *cpu_dev;
	struct device_node *cpu_np;
	u32 val;

	cpu_dev = get_cpu_device(cpu);
	cpu_np = cpu_dev->of_node;
	of_property_read_u32(cpu_np, "reg", &val);

	return val & TA_CPU_FLAG;
}

static bool check_fctrl0_dsmpd(u32 val)
{
	return val & DSMPD_FLAG;
}

static u32 set_target_fbdiv(unsigned int freq, u32 val)
{
	u32 refdiv = val & REFDIV_FLAG;
	u32 pdiv1 = (val & PDIV1_FLAG) >> PDIV1_SBIT;
	u32 fbdiv = (freq / SYS_CLKIN * refdiv * pdiv1) << FBDIV_SBIT;

	val = (val & CLEAN_FBDIV) | fbdiv;
	return val;
}

static bool check_target_fbdiv(u32 val, unsigned int freq)
{
	u32 fbdiv = (val & FBDIV_FLAG) >> FBDIV_SBIT;
	u32 refdiv = val & REFDIV_FLAG;
	u32 pdiv1 = (val & PDIV1_FLAG) >> PDIV1_SBIT;
	u32 pdiv2 = (val & PDIV2_FLAG) >> PDIV2_SBIT;

	return fbdiv == freq / SYS_CLKIN * refdiv * pdiv1 * pdiv2;
}

static unsigned int comput_pll0(unsigned int cpu)
{
	u32 val, fbdiv, refdiv, pdiv1, pdiv2;

	if (check_ta_cpu(cpu))
		val = read_pll_fctrl0(hip_freq_priv->a_base);
	else
		val = read_pll_fctrl0(hip_freq_priv->c_base);

	fbdiv = (val & FBDIV_FLAG) >> FBDIV_SBIT;
	refdiv = val & REFDIV_FLAG;
	pdiv1 = (val & PDIV1_FLAG) >> PDIV1_SBIT;
	pdiv2 = (val & PDIV2_FLAG) >> PDIV2_SBIT;

	return SYS_CLKIN * fbdiv * 1000 / refdiv / pdiv1 / pdiv2;
}

static u32 calc_no(u32 no)
{
	int i = 0;
	u32 val = 1;
	for (; i < no; i++) {
		val *= 2;
		if (i >= 4)
			break;
	}
	return val;
}

static unsigned int comput_pll1(unsigned int cpu)
{
	u32 val, no, nf, nr;

	if (check_ta_cpu(cpu))
		val = read_tsmc_pll_fctrl1(hip_freq_priv->a_base);
	else
		val = read_tsmc_pll_fctrl1(hip_freq_priv->c_base);

	nr = val & NR_FLAG;
	nr++;
	nf = (val & NF_FLAG) >> NF_SBIT;
	nf = 2 * (nf + 1);
	no = (val & NO_FLAG) >> NO_SBIT;
	no = calc_no(no);

	return SYS_CLKIN * 1000 * nf / nr / no;
}

static unsigned int extract_freq(unsigned int cpu)
{
	if (pll_flag == 0)
		return comput_pll0(cpu);
	return comput_pll1(cpu);
}

static unsigned int get_cur_freq_on_cpu(unsigned int cpu)
{
	unsigned int master = per_cpu(master_cpu, cpu);
	unsigned int index = per_cpu(cur_index, master);
	unsigned int freq;
	unsigned int cached_freq;

	pr_debug("get_cpu_freq_on_cpu (%d)\n", cpu);

	cached_freq = hipv660_freq_table[index].frequency;

	/* check the cur_freq with extract_freq */
	freq = extract_freq(master);
	if (freq != cached_freq) {
		/* TODO */
	}

	pr_debug("extract freq %d cur freq %d\n", freq, cached_freq);
	return freq;
}

static int hipv660_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int index)
{ 
	unsigned int cur_state = per_cpu(cur_index, policy->cpu);
	unsigned int next_state = 0; /* Index into freq table */
	unsigned int freq_old = 0;
	unsigned int freq_new = 0;
	unsigned int span = 0;
	bool dsmpd_flag = 0;
	u32 val;
	void __iomem *base;
	int result = 0;

	if (policy->cpu != per_cpu(master_cpu, policy->cpu))
		return -EINVAL;

	if (check_ta_cpu(policy->cpu))
		base = hip_freq_priv->a_base;
	else
		base = hip_freq_priv->c_base;

	next_state = hipv660_freq_table[index].driver_data;

	if (hip_freq_priv->power_off && (index == freq_type_400 || index == freq_type_100)) {
		printk("Normal to Slow\n");
		sys_mode_change(SYSMODE_SLOW, hip_freq_priv->ta_base);
		printk("Waiting for changing\n");
		while(check_mode_change(SYSMODE_SLOW, hip_freq_priv->ta_base))
			;
		if (check_master_pll(base)) {
			if (index == freq_type_400) {
				printk("Set 400M to pll0\n");
				write_pll_fctrl0(0x563C1C02, base);
				udelay(5);
				write_pll_fctrl0(0x543C1C02, base);
					;
			} else {
				printk("Set 100M to pll0\n");
				write_pll_fctrl0(0x569C1C02, base);
				udelay(5);
				write_pll_fctrl0(0x549C1C02, base);
			}
			printk("Waiting for pll0 lock\n");
			while (check_pll_lock_status())
				;
		} else {
			if (index == freq_type_400) {
				printk("Set 400M to pll1\n");
				write_tsmc_pll_fctrl1(0x103E1, base);
			} else {
				printk("Set 100M to pll1\n");
				write_tsmc_pll_fctrl1(0x203E1, base);
			}
			printk("Waiting for pll1 lock\n");
			while (check_pll1_lock_status())
				;
		}

		printk("SLOW to Normal\n");
		sys_mode_change(SYSMODE_NORMAL, hip_freq_priv->ta_base);
		printk("Waiting for changing\n");
		while(check_mode_change(SYSMODE_NORMAL, hip_freq_priv->ta_base))
			;
		goto set_state;
	} else if (hip_freq_priv->power_off || index == freq_type_400 || index == freq_type_100) {
		pr_err("Now we will power off or set 400M/100M, so we can't set the cpufreq\n");
		return -EINVAL;
	}

	if (!check_master_pll(base)) {
		goto set_state;
	}

	freq_old = hipv660_freq_table[cur_state].frequency / 1000;
	freq_new = hipv660_freq_table[index].frequency / 1000;

	val = read_pll_fctrl0(base);

	if (check_target_fbdiv(val, freq_new)) {
		pr_debug("The sc_fctrl0 is already target fbdiv %d\n", (val & FBDIV_FLAG) >> 6);
		goto set_state;
	}

	/*
	 * make sure the SC_PLLFCTRL0[26] to be zero,
	 * so that we can change freq under decimal.
	 */
	if (check_fctrl0_dsmpd(val)) {
		pr_debug("FCTRL0 Dsmpd is not in decimal\n");
		dsmpd_flag = 1;
		val &= CLEAN_DSMPD;
		write_pll_fctrl0(val, base);
		udelay(1);
	}

	while (freq_old > freq_new) {
		span = freq_old / 100 * 5;
		freq_old -= span;
		if (freq_old <= freq_new)
			goto set_new_freq;

		/* fbdiv = CPUFREQ/SYS_CLKIN * REFDIV * POSTDIV1 */
		val = set_target_fbdiv(freq_old, val);

		/* write pll ctrl0 and ctrl1 */
		do_write_pll(val, base);
	}

	while (freq_old < freq_new) {
		span = freq_old / 100 * 5;
		freq_old += span;
		if (freq_old >= freq_new)
			goto set_new_freq;

		val = set_target_fbdiv(freq_old, val);

		/* write pll ctrl0 and ctrl1 */
		do_write_pll(val, base);
	}

set_new_freq:
	val = set_target_fbdiv(freq_new, val);

	/* write pll ctrl0 and ctrl1 */
	do_write_pll(val, base);

	write_pll_fctrl1(FCTRL1_OFLAG, base);

	if (dsmpd_flag) {
		val |= DSMPD_FLAG;
		write_pll_fctrl0(val, base);
	}

set_state:
	if (!result)
		per_cpu(cur_index, policy->cpu) = next_state;

	return result;
}

static int hipv660_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	unsigned int cpu = policy->cpu;
	unsigned int cur_freq;
	int result = 0;

	pr_debug("hipv660_cpufreq_cpu_init\n");

	cur_freq = extract_freq(cpu) / 1000;

	if (cur_freq == 1050)
		per_cpu(cur_index, cpu) = 2;
	else
		per_cpu(cur_index, cpu) = 0;

	/* detect transition latency */
	policy->cpuinfo.transition_latency = 0;
	hipv660_cpufreq_driver.flags |= CPUFREQ_CONST_LOOPS;

	result = cpufreq_table_validate_and_show(policy, hipv660_freq_table);
	if (result) {
		pr_err("%s: validate freq table failed\n", __func__);
		return result;
	}

	return result;

}

static int hipv660_cpufreq_cpu_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static struct freq_attr *hipv660_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver hipv660_cpufreq_driver = {
	.verify		= cpufreq_generic_frequency_table_verify,
	.target_index	= hipv660_cpufreq_target,
	.init		= hipv660_cpufreq_cpu_init,
	.exit		= hipv660_cpufreq_cpu_exit,
	.get		= get_cur_freq_on_cpu,
	.name		= "cpufreq-hipv660",
	.attr		= hipv660_cpufreq_attr,
};

static void set_master_cpu(void)
{
	unsigned int i, f_a = 0, f_c = 0;
	unsigned int m_a, m_c;

	for (i = 0; i < num_possible_cpus(); i++)
		per_cpu(master_cpu, i) = 0xffff;
	for (i = 0; i < num_possible_cpus(); i++) {
		if (check_ta_cpu(i)) {
			if (!f_a) {
				m_a = i;
				f_a = 1;
			}
			per_cpu(master_cpu, i) = m_a;
		} else {
			if (!f_c) {
				m_c = i;
				f_c = 1;
			}
			per_cpu(master_cpu, i) = m_c;
		}
	}
}

static int hip_cpufreq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *ta_res, *a_res, *c_res;

	if (cpufreq_get_current_driver())
		return -EEXIST;

	hip_freq_priv = kzalloc(sizeof(*hip_freq_priv), GFP_KERNEL);
	if (!hip_freq_priv)
		return -ENOMEM;

	ta_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	a_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	c_res = platform_get_resource(pdev, IORESOURCE_MEM, 2);

	if (!ta_res || !(a_res || c_res)) {
		pr_err("Can not find this hip-sysctrl IO resource\n");
		return -ENOENT;
	}

	if (ta_res->flags & IORESOURCE_CACHEABLE)
		hip_freq_priv->ta_base = devm_ioremap(dev, ta_res->start, SC_SIZE);
	else
		hip_freq_priv->ta_base = devm_ioremap_nocache(dev, ta_res->start, SC_SIZE);
	if (a_res) {
		if (a_res->flags & IORESOURCE_CACHEABLE)
			hip_freq_priv->a_base = devm_ioremap(dev, a_res->start, SC_SIZE);
		else
			hip_freq_priv->a_base = devm_ioremap_nocache(dev, a_res->start, SC_SIZE);
	}
	if (c_res) {
		if (c_res->flags & IORESOURCE_CACHEABLE)
			hip_freq_priv->c_base = devm_ioremap(dev, c_res->start, SC_SIZE);
		else
			hip_freq_priv->c_base = devm_ioremap_nocache(dev, c_res->start, SC_SIZE);
	}
	 if (IS_ERR(hip_freq_priv->ta_base) ||
	     (a_res && IS_ERR(hip_freq_priv->a_base)) ||
	     (c_res && IS_ERR(hip_freq_priv->c_base)))
		return -EIO;

	if (a_res->start & SC_C_BASE) {
		void __iomem *tmp;

		tmp = hip_freq_priv->c_base;
		hip_freq_priv->c_base = hip_freq_priv->a_base;
		hip_freq_priv->a_base = tmp;
	}

	if (!check_master_pll(hip_freq_priv->c_base)) {
		pr_info("The cpu freq support PLL1\n");
		hipv660_freq_table = hipv660_freq_table1;
		pll_flag = 1;
		freq_type_400 = 1;
		freq_type_100 = 2;
	} else {
		pr_info("The cpu freq support PLL0\n");
		hipv660_freq_table = hipv660_freq_table0;
		pll_flag = 0;
		freq_type_400 = 4;
		freq_type_100 = 5;
	}

	set_master_cpu();
	hip_freq_priv->power_off = 0;

	return cpufreq_register_driver(&hipv660_cpufreq_driver);
}

static int hip_cpufreq_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	if (hip_freq_priv->ta_base)
		devm_iounmap(dev, hip_freq_priv->ta_base);
	if (hip_freq_priv->a_base)
		devm_iounmap(dev, hip_freq_priv->a_base);
	if (hip_freq_priv->c_base)
		devm_iounmap(dev, hip_freq_priv->c_base);

	kfree(hip_freq_priv);

	cpufreq_unregister_driver(&hipv660_cpufreq_driver);

	return 0;
}

static const struct of_device_id hip_sc_of_match[] = {
	{.compatible = "hisilicon,hip-cpufreq",},
	{},
};

MODULE_DEVICE_TABLE(of, hip_sc_of_match);

static struct platform_driver hip_cpufreq_driver = {
	.driver = {
		.name = "hip-cpufreq",
		.of_match_table = hip_sc_of_match,
	},
	.probe = hip_cpufreq_probe,
	.remove = hip_cpufreq_remove,
};

module_platform_driver(hip_cpufreq_driver);

MODULE_DESCRIPTION("CPUfreq for hisillicon platform");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:hip-cpufreq");
