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
	{ 0x4c3af445, "__request_region" },
	{ 0x70ecc9b2, "cdev_del" },
	{ 0x4ac7b024, "pci_bus_read_config_byte" },
	{ 0x90f98600, "nac_mem_free" },
	{ 0xec7bc0d, "__mod_timer" },
	{ 0xd2248aac, "cdev_init" },
	{ 0xdc3eaf70, "iomem_resource" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xab978df6, "malloc_sizes" },
	{ 0x472279b0, "kobject_set_name" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x7d11c268, "jiffies" },
	{ 0xb2a606bf, "pci_set_master" },
	{ 0xc659d5a, "del_timer_sync" },
	{ 0x1b7d4074, "printk" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x56134363, "pci_bus_write_config_dword" },
	{ 0xf3b39202, "mod_timer" },
	{ 0x6ca08048, "pci_enable_msi" },
	{ 0x5e22fdec, "cdev_add" },
	{ 0xca8e5223, "pci_set_mwi" },
	{ 0xda73a1d0, "__check_region" },
	{ 0x19070091, "kmem_cache_alloc" },
	{ 0x3762cb6e, "ioremap_nocache" },
	{ 0x7561ed, "pci_bus_read_config_dword" },
	{ 0x37c3a5d7, "__udivdi3" },
	{ 0x26e96637, "request_irq" },
	{ 0x4292364c, "schedule" },
	{ 0x8a7d1c31, "high_memory" },
	{ 0x8bb33e7d, "__release_region" },
	{ 0x7423476b, "pci_unregister_driver" },
	{ 0x19cacd0, "init_waitqueue_head" },
	{ 0xd0b91f9b, "init_timer" },
	{ 0x59968f3c, "__wake_up" },
	{ 0x72270e35, "do_gettimeofday" },
	{ 0xb5f46a8b, "pci_bus_write_config_byte" },
	{ 0x37a0cba, "kfree" },
	{ 0x2b6754b3, "remap_pfn_range" },
	{ 0x1a2e7d93, "nac_mem_malloc" },
	{ 0xc8f02aeb, "prepare_to_wait" },
	{ 0xedc03953, "iounmap" },
	{ 0x13f3405e, "__pci_register_driver" },
	{ 0x865edc9b, "finish_wait" },
	{ 0x25da070, "snprintf" },
	{ 0xdd994dbd, "pci_enable_device" },
	{ 0xf2a644fb, "copy_from_user" },
	{ 0x22a992bf, "__divdi3" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=nacmem,divdi3";


MODULE_INFO(srcversion, "82AC749390C1CDA8B97FCB0");
