#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x89e24b9c, "struct_module" },
	{ 0x317541f0, "mem_map" },
	{ 0x89b301d4, "param_get_int" },
	{ 0x20000329, "simple_strtoul" },
	{ 0x98bd6f46, "param_set_int" },
	{ 0x87cddf59, "_spin_lock_irqsave" },
	{ 0x1b7d4074, "printk" },
	{ 0xb7046d9a, "pci_find_device" },
	{ 0xa20fdde, "_spin_unlock_irqrestore" },
	{ 0x107d6ba3, "__get_free_pages" },
	{ 0x683a3221, "param_set_copystring" },
	{ 0x9941ccb8, "free_pages" },
	{ 0x25da070, "snprintf" },
	{ 0x93304684, "param_get_string" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9182915914D7D816D9F93B1");
