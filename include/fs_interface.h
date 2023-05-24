/*
 * Copyright (C) 2021 Stefano Babic <sbabic@denx.de>
 *
 * SPDX-License-Identifier:     GPL-2.0-only
 */

#ifndef _FS_INTERFACE_H
#define _FS_INTERFACE_H

char *diskformat_fs_detect(char *device);
int diskformat_fs_exists(char *device, char *fstype);

int diskformat_mkfs(char *device, char *fstype);

#if defined(CONFIG_FAT_FILESYSTEM)
extern int fat_mkfs(const char *device_name, const char *fstype);
#endif

#if defined (CONFIG_EXT_FILESYSTEM) 
extern int ext_mkfs(const char *device_name, const char *fstype, unsigned long features,
		const char *volume_label);
#endif

#if defined (CONFIG_BTRFS_FILESYSTEM) 
extern int btrfs_mkfs(const char *device_name, const char *fstype);
#endif
#endif
