/* linux/drivers/video/samsung/mdnie_tunning.c
 *
 * Register interface file for Samsung mDNIe driver
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>

#include "s3cfb.h"
#include "mdnie.h"

static unsigned short mdnie_data[100];

static int parse_text(char *src, int len)
{
	int i, count, ret;
	int index = 0;
	char *str_line[100];
	char *sstart;
	char *c;
	unsigned int data1, data2;
	unsigned int reg_mcm_mask_index, reg_mcm_cb_index, reg_mcm_cr_index;
	unsigned int reg_de_sharpness_index, reg_de_threshold_index, reg_cs_gain_index;

	c = src;
	count = 0;
	sstart = c;
	reg_mcm_mask_index = reg_mcm_cb_index = reg_mcm_cr_index = 0;
	reg_de_sharpness_index = reg_de_threshold_index = reg_cs_gain_index = 0;

	for (i = 0; i < len; i++, c++) {
		char a = *c;
		if (a == '\r' || a == '\n') {
			if (c > sstart) {
				str_line[count] = sstart;
				count++;
			}
			*c = '\0';
			sstart = c+1;
		}
	}

	if (c > sstart) {
		str_line[count] = sstart;
		count++;
	}

	/* printk(KERN_INFO "----------------------------- Total number of lines:%d\n", count); */

	for (i = 0; i < count; i++) {
		/* printk(KERN_INFO "line:%d, [start]%s[end]\n", i, str_line[i]); */
		ret = sscanf(str_line[i], "0x%x,0x%x\n", &data1, &data2);
		/* printk(KERN_INFO "Result => [0x%2x 0x%4x] %s\n", data1, data2, (ret == 2) ? "Ok" : "Not available"); */
		if (ret == 2) {
			if (mdnie_user_mode != 0x0000) {
				switch (data1) {
					case 0x0001:
						data2 = mdnie_user_mode;
						break;
					case 0x0028: // register mask
						if (mdnie_user_mode == 0x0045 || mdnie_user_mode == 0x0006) {
							printk(KERN_INFO "mdnie_user_mcm_cb=0x%x, mdnie_user_mcm_cr=0x%x", mdnie_user_mcm_cb, mdnie_user_mcm_cr);
							if (reg_mcm_mask_index) {
								mdnie_data[reg_mcm_mask_index+1]  = 0x0064;
							} else {
								mdnie_data[index++] = 0x005b;
								mdnie_data[index++] = 0x0064;
							}
							if (reg_mcm_cb_index) {
								mdnie_data[reg_mcm_cb_index+1]  = mdnie_user_mcm_cb;
							} else {
								mdnie_data[index++] = 0x0063;
								mdnie_data[index++] = mdnie_user_mcm_cb;
							}
							if (reg_mcm_cr_index) {
								mdnie_data[reg_mcm_cr_index+1]  = mdnie_user_mcm_cr;
							} else {
								mdnie_data[index++] = 0x0065;
								mdnie_data[index++] = mdnie_user_mcm_cr;
							}
						}
						if (mdnie_user_de_control_enabled) {
							if (reg_de_sharpness_index) {
								// noop :mdnie_data[reg_de_sharpness_index+1]  = mdnie_user_de_sharpness;
							} else {
								mdnie_data[index++] = 0x003b;
								mdnie_data[index++] = mdnie_user_de_sharpness;
							}
							if (reg_cs_gain_index) {
								// noop : mdnie_data[reg_cs_gain_index+1]  = mdnie_user_cs_gain;
							} else {
								mdnie_data[index++] = 0x003f;
								mdnie_data[index++] = mdnie_user_cs_gain;
							}
							if (reg_de_threshold_index) {
								// noop : mDNIe_data[reg_de_threshold_index+1]  = mdnie_user_de_threshold;
							} else {
								mdnie_data[index++] = 0x0042;
								mdnie_data[index++] = mdnie_user_de_threshold;
							}
						}
						break;
					case 0x005b: // MCM type
						reg_mcm_mask_index = index;
						break;
					case 0x0063: // MCM cb
						reg_mcm_cb_index = index;
						break;
					case 0x0065: // MCM cr
						reg_mcm_cr_index = index;
						break;
					case 0x003b: // DE SHARPNESS
						reg_de_sharpness_index = index;
						break;
					case 0x003f: // CS Gain
						reg_cs_gain_index = index;
						break;
					case 0x0042: // DE TH
						reg_de_threshold_index = index;
						break;
				}
			}
			mdnie_data[index++] = (u16)data1;
			mdnie_data[index++] = (u16)data2;
		}
	}
	return index;
}

int mdnie_txtbuf_to_parsing(char const *pFilepath)
{
	struct file *filp;
	char	*dp;
	long	l;
	loff_t  pos;
	int     ret, num;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(get_ds());

	printk(KERN_INFO "%s:", pFilepath);

	if (!pFilepath) {
		printk(KERN_ERR "Error : mdnie_txtbuf_to_parsing has invalid filepath.\n");
		goto parse_err;
	}

	filp = filp_open(pFilepath, O_RDONLY, 0);

	if (IS_ERR(filp)) {
		printk(KERN_ERR "file open error:%d\n", (s32)filp);
		goto parse_err;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	dp = kmalloc(l+10, GFP_KERNEL);		/* add cushion : origianl code is 'dp = kmalloc(l, GFP_KERNEL);' */
	if (dp == NULL) {
		printk(KERN_INFO "Out of Memory!\n");
		filp_close(filp, current->files);
		goto parse_err;
	}
	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		printk(KERN_INFO "<CMC623> Failed to read file (ret = %d)\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		goto parse_err;
	}

	filp_close(filp, current->files);
	set_fs(fs);
	num = parse_text(dp, l);
	if (!num) {
		printk(KERN_ERR "Nothing to parse!\n");
		kfree(dp);
		goto parse_err;
	}

	mdnie_data[num] = END_SEQ;

	mdnie_send_sequence(g_mdnie, mdnie_data);

	kfree(dp);

	num = num / 2;
	return num;

parse_err:
	return -EPERM;
}

