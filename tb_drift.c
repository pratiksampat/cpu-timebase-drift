// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Module-based API test to test for any TimeBase anomalies or drifts from
 * one CPU to another
 */

#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>

int active = 0;

static void measure_tb(void *remote_tb)
{
	*((u64 *) remote_tb) = mftb();
}

static void run_tb_test(int activate)
{
	int cpu;
	u64 local_tb = 0, remote_tb = 0;

	if (!activate)
		return;
	trace_printk("--TB TEST INITIATED--\n");
	active = activate;

	for (cpu = 0; cpu < NR_CPUS; cpu+=8) {
		if (!cpu_online(cpu) || cpu == smp_processor_id())
			continue;
		smp_call_function_single(cpu, measure_tb, &remote_tb, 1);
		smp_mb();
		local_tb = mftb();
		if (local_tb < remote_tb) {
			printk(KERN_ALERT "ERR : CPU [%d] TB = %lld, Remote CPU [%d] TB = %lld\n", smp_processor_id(), local_tb, cpu, remote_tb);
			trace_printk(KERN_ALERT "ERR : CPU [%d] TB = %lld, Remote CPU [%d] TB = %lld\n", smp_processor_id(), local_tb, cpu, remote_tb);
		} else {
			trace_printk("INFO : CPU [%d] TB = %lld, Remote CPU [%d] TB = %lld\n", smp_processor_id(), local_tb, cpu, remote_tb);
		}
	}

	trace_printk("CPU: %d tb: %ld\n", smp_processor_id(), mftb());
}

static struct dentry *dir;

static int tb_read_op(void *data, u64 *value)
{
	*value = active;
	return 0;
}

static int tb_write_op(void *data, u64 value)
{
	run_tb_test(value);
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(tb_ops, tb_read_op, tb_write_op, "%llu\n");

static int __init tb_init(void)
{
	struct dentry *temp;

	dir = debugfs_create_dir("tb_test", 0);
	if (!dir) {
		pr_alert("tb_test: failed to create /sys/kernel/debug/tb_test\n");
		return -1;
	}

	temp = debugfs_create_file("tb_test_activate",
				   0666,
				   dir,
				   NULL,
				   &tb_ops);
	if (!temp) {
		pr_alert("tb_test: failed to create /sys/kernel/debug/tb_test_activate\n");
		return -1;
	}

	pr_info("TB Test module loaded\n");
	return 0;
}

static void __exit tb_cleanup(void)
{
	pr_info("Cleaning up TB Test module.\n");
	debugfs_remove_recursive(dir);
}

module_init(tb_init);
module_exit(tb_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("IBM Corporation");
MODULE_DESCRIPTION("Measuring idle latency for IPIs and Timers");
