/* Copyright (c) 2015, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <dev/flash-ubi.h>
#include <dev/flash.h>
#include <qpic_nand.h>
#include <rand.h>

#define UBI_VOL_UPDATE_IMG_MEM_OFFSET (80*1024*1024)

static
const uint32_t crc32_table[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

static int ubi_erase_peb(int peb_num, struct ubi_scan_info *si,
		int ptn_start);

static uint32_t mtd_crc32(uint32_t crc, const void *buf, size_t size)
{
	const uint8_t *p = buf;

	while (size--)
		crc = crc32_table[(crc ^ *p++) & 0xff] ^ (crc >> 8);
	return crc;
}

/**
 * check_pattern - check if buffer contains only a certain byte pattern.
 * @buf: buffer to check
 * @patt: the pattern to check
 * @size: buffer size in bytes
 *
 * This function returns %1 if there are only @patt bytes in @buf, and %0 if
 * something else was also found.
 */
int check_pattern(const void *buf, uint8_t patt, int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (((const uint8_t *)buf)[i] != patt)
			return 0;
	return 1;
}

/**
 * calc_data_len - calculate how much real data is stored in the buffer
 * @page_size: min I/O of the device
 * @buf: a buffer with the contents of the physical eraseblock
 * @len: the buffer length
 *
 * This function calculates how much "real data" is stored in @buf and
 * returns the length (in number of pages). Continuous 0xFF bytes at the end
 * of the buffer are not considered as "real data".
 */
static int calc_data_len(int page_size, const void *buf, int len)
{
	int i;

	for (i = len - 1; i >= 0; i--)
		if (((const uint8_t *)buf)[i] != 0xFF)
			break;

	/* The resulting length must be aligned to the minimum flash I/O size */
	len = i + 1;
	len = (len + page_size - 1) / page_size;
	return len;
}

/**
 * read_ec_hdr - read and check an erase counter header.
 * @peb: number of the physical erase block to read the header for
 * @ec_hdr: a &struct ubi_ec_hdr object where to store the read erase counter
 * 			header
 *
 * This function reads erase counter header from physical eraseblock @peb and
 * stores it in @ec_hdr. This function also checks the validity of the read
 * header.
 *
 * Return codes:
 * -1 - in case of error
 *  0 - if PEB was found valid
 *  1 - if PEB is empty
 */
static int read_ec_hdr(uint32_t peb, struct ubi_ec_hdr *ec_hdr)
{
	unsigned char *spare, *tmp_buf;
	int ret = -1;
	uint32_t crc;
	int page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;

	spare = (unsigned char *)malloc(flash_spare_size());
	if (!spare)
	{
		dprintf(CRITICAL, "read_ec_hdr: Mem allocation failed\n");
		return ret;
	}

	tmp_buf = (unsigned char *)malloc(page_size);
	if (!tmp_buf)
	{
		dprintf(CRITICAL, "read_ec_hdr: Mem allocation failed\n");
		goto out_tmp_buf;
	}

	if (qpic_nand_block_isbad(peb * num_pages_per_blk)) {
		dprintf(CRITICAL, "read_ec_hdr: Bad block @ %d\n", peb);
		goto out;
	}

	if (qpic_nand_read(peb * num_pages_per_blk, 1, tmp_buf, spare)) {
		dprintf(CRITICAL, "read_ec_hdr: Read %d failed \n", peb);
		goto out;
	}
	memcpy(ec_hdr, tmp_buf, UBI_EC_HDR_SIZE);

	if (check_pattern((void *)ec_hdr, 0xFF, UBI_EC_HDR_SIZE)) {
		ret = 1;
		goto out;
	}

	/* Make sure we read a valid UBI EC_HEADER */
	if (BE32(ec_hdr->magic) != (uint32_t)UBI_EC_HDR_MAGIC) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong magic at peb-%d Expected: %d, received %d\n",
			peb, UBI_EC_HDR_MAGIC, BE32(ec_hdr->magic));
		goto out;
	}

	if (ec_hdr->version != UBI_VERSION) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong version at peb-%d Expected: %d, received %d\n",
			peb, UBI_VERSION, ec_hdr->version);
		goto out;
	}

	if (BE64(ec_hdr->ec) > UBI_MAX_ERASECOUNTER) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong ec at peb-%d: %lld \n",
			peb, BE64(ec_hdr->ec));
		goto out;
	}

	crc = mtd_crc32(UBI_CRC32_INIT, ec_hdr, UBI_EC_HDR_SIZE_CRC);
	if (BE32(ec_hdr->hdr_crc) != crc) {
		dprintf(CRITICAL,
			"read_ec_hdr: Wrong crc at peb-%d: calculated %d, recived %d\n",
			peb,crc,  BE32(ec_hdr->hdr_crc));
		goto out;
	}

	ret = 0;
out:
	free(tmp_buf);
out_tmp_buf:
	free(spare);
	return ret;
}

/**
 * read_vid_hdr - read and check an Volume identifier header.
 * @peb: number of the physical erase block to read the header for
 * @vid_hdr: a &struct ubi_vid_hdr object where to store the read header
 * @vid_hdr_offset: offset of the VID header from the beginning of the PEB
 * 			 (in bytes)
 *
 * This function reads the volume identifier header from physical
 * eraseblock @peb and stores it in @vid_hdr. This function also checks the
 * validity of the read header.
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 *  1 - if the PEB is free (no VID hdr)
 */
static int read_vid_hdr(uint32_t peb, struct ubi_vid_hdr *vid_hdr,
		int vid_hdr_offset)
{
	unsigned char *spare, *tmp_buf;
	int ret = -1;
	uint32_t crc, magic;
	int page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;

	spare = (unsigned char *)malloc(flash_spare_size());
	if (!spare)
	{
		dprintf(CRITICAL, "read_vid_hdr: Mem allocation failed\n");
		return ret;
	}

	tmp_buf = (unsigned char *)malloc(page_size);
	if (!tmp_buf)
	{
		dprintf(CRITICAL, "read_vid_hdr: Mem allocation failed\n");
		goto out_tmp_buf;
	}

	if (qpic_nand_block_isbad(peb * num_pages_per_blk)) {
		dprintf(CRITICAL, "read_vid_hdr: Bad block @ %d\n", peb);
		goto out;
	}

	if (qpic_nand_read(peb * num_pages_per_blk + vid_hdr_offset/page_size,
			1, tmp_buf, spare)) {
		dprintf(CRITICAL, "read_vid_hdr: Read %d failed \n", peb);
		goto out;
	}
	memcpy(vid_hdr, tmp_buf, UBI_VID_HDR_SIZE);

	if (check_pattern((void *)vid_hdr, 0xFF, UBI_VID_HDR_SIZE)) {
		ret = 1;
		goto out;
	}

	magic = BE32(vid_hdr->magic);
	if (magic != UBI_VID_HDR_MAGIC) {
		dprintf(CRITICAL,
				"read_vid_hdr: Wrong magic at peb-%d Expected: %d, received %d\n",
				peb, UBI_VID_HDR_MAGIC, BE32(vid_hdr->magic));
		goto out;
	}

	crc = mtd_crc32(UBI_CRC32_INIT, vid_hdr, UBI_EC_HDR_SIZE_CRC);
	if (BE32(vid_hdr->hdr_crc) != crc) {
		dprintf(CRITICAL,
			"read_vid_hdr: Wrong crc at peb-%d: calculated %d, received %d\n",
			peb,crc,  BE32(vid_hdr->hdr_crc));
		goto out;
	}

	ret = 0;
out:
	free(tmp_buf);
out_tmp_buf:
	free(spare);
	return ret;
}

/**
 * read_leb_data - read data section of the PEB (LEB).
 * @peb: number of the physical erase block to read the data for
 * @leb_data: a buffer where to store the read data at
 * @leb_size: LEB size
 * @data_offset: offset of the data from the beginning of the PEB
 * 			 (in bytes)
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int read_leb_data(uint32_t peb, void *leb_data,
		int leb_size, int data_offset)
{
	unsigned char *spare, *tmp_buf;
	int ret = -1;
	int page_size = flash_page_size();
	int block_size = flash_block_size();
	int num_pages_per_blk = block_size/page_size;

	spare = (unsigned char *)malloc(flash_spare_size());
	if (!spare)
	{
		dprintf(CRITICAL, "read_leb_data: Mem allocation failed\n");
		return ret;
	}
	tmp_buf = (unsigned char *)malloc(leb_size);
	if (!tmp_buf)
	{
		dprintf(CRITICAL, "read_leb_data: Mem allocation failed\n");
		goto out_tmp_buf;
	}
	if (qpic_nand_block_isbad(peb * num_pages_per_blk)) {
		dprintf(CRITICAL, "read_leb_data: Bad block @ %d\n", peb);
		goto out;
	}

	if (qpic_nand_read(peb * num_pages_per_blk + data_offset/page_size,
			leb_size/page_size, tmp_buf, spare)) {
		dprintf(CRITICAL, "read_leb_data: Read %d failed \n", peb);
		goto out;
	}
	memcpy(leb_data, tmp_buf, leb_size);

	ret = 0;
out:
	free(tmp_buf);
out_tmp_buf:
	free(spare);
	return ret;
}

/**
 * write_ec_header() - Write provided ec_header for given PEB
 * @peb: number of the physical erase block to write the header to
 * @new_ech: the ec_header to write
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int write_ec_header(uint32_t peb, struct ubi_ec_hdr *new_ech)
{
	unsigned page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;
	unsigned char *buf;
	int ret = 0;

	buf = malloc(sizeof(uint8_t) * page_size);
	if (!buf) {
		dprintf(CRITICAL, "write_ec_header: Mem allocation failed\n");
		return -1;
	}

	memset(buf, 0xFF, page_size);
	ASSERT(page_size > sizeof(*new_ech));
	memcpy(buf, new_ech, UBI_EC_HDR_SIZE);
	ret = qpic_nand_write(peb * num_pages_per_blk, 1, buf, 0);
	if (ret) {
		dprintf(CRITICAL,
			"write_ec_header: qpic_nand_write failed with %d\n", ret);
		ret = -1;
		goto out;
	}

out:
	free(buf);
	return ret;
}

/**
 * write_vid_header() - Write provided vid_header for given PEB
 * @peb: number of the physical erase block to write the header to
 * @new_vidh: the vid_header to write
 * @offset: vid_hdr offset in bytes from the beginning of the PEB
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int write_vid_header(uint32_t peb,
		struct ubi_vid_hdr *new_vidh, int offset)
{
	unsigned page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size()/page_size;
	unsigned char *buf;
	int ret = 0;

	buf = malloc(sizeof(uint8_t) * page_size);
	if (!buf) {
		dprintf(CRITICAL, "write_vid_header: Mem allocation failed\n");
		return -1;
	}

	memset(buf, 0xFF, page_size);
	ASSERT(page_size > sizeof(*new_vidh));
	memcpy(buf, new_vidh, UBI_VID_HDR_SIZE);
	ret = qpic_nand_write(peb * num_pages_per_blk + offset/page_size,
			1, buf, 0);
	if (ret) {
		dprintf(CRITICAL,
			"write_vid_header: qpic_nand_write failed with %d\n", ret);
		ret = -1;
		goto out;
	}

out:
	free(buf);
	return ret;
}

/**
 * write_leb_data - write data section of the PEB (LEB).
 * @peb: number of the physical erase block to write the data for
 * @leb_data: a data buffer to write
 * @size: data size
 * @data_offset: offset of the data from the beginning of the PEB
 * 			 (in bytes)
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int write_leb_data(uint32_t peb, void *data,
		int size, int data_offset)
{
	unsigned char *tmp_buf;
	int ret = -1;
	int num_pages;
	int page_size = flash_page_size();
	int block_size = flash_block_size();
	int num_pages_per_blk = block_size/page_size;

	if(size > block_size - data_offset ){
		dprintf(CRITICAL,
			"write_leb_data: data size %d is too big!\n",size);
		return ret;
	}

	tmp_buf = (unsigned char *)malloc(block_size - data_offset);
	if (!tmp_buf)
	{
		dprintf(CRITICAL, "write_leb_data: Mem allocation failed\n");
		return -1;
	}
	memset(tmp_buf, 0xFF, block_size - data_offset);

	if (size < page_size)
		num_pages = 1;
	else
		num_pages = calc_data_len(page_size, data,
				block_size - data_offset);

	memcpy(tmp_buf, data, size);
	ret = qpic_nand_write(peb * num_pages_per_blk + data_offset/page_size,
			num_pages, tmp_buf, 0);
	if (ret) {
		dprintf(CRITICAL,
			"write_vid_header: qpic_nand_write failed with %d\n", ret);
		ret = -1;
		goto out;
	}

	ret = 0;
out:
	free(tmp_buf);
	return ret;
}

/**
 * scan_partition() - Collect the ec_headers info of a given partition
 * @ptn: partition to read the headers of
 *
 * Returns allocated and filled struct ubi_scan_info (si).
 * Note: si should be released by caller.
 */
static struct ubi_scan_info *scan_partition(struct ptentry *ptn)
{
	struct ubi_scan_info *si;
	struct ubi_ec_hdr *ec_hdr;
	struct ubi_vid_hdr vid_hdr;
	unsigned i;
	unsigned long long sum = 0;
	int page_size = flash_page_size();
	int ret;

	si = malloc(sizeof(*si));
	if (!si) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		return NULL;
	}

	memset((void *)si, 0, sizeof(*si));
	si->pebs_data = malloc(ptn->length * sizeof(struct peb_info));
	if (!si->pebs_data) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		goto out_failed_pebs;
	}
	memset((void *)si->pebs_data, 0, ptn->length * sizeof(struct peb_info));

	ec_hdr = malloc(UBI_EC_HDR_SIZE);
	if (!ec_hdr) {
		dprintf(CRITICAL,"scan_partition: (%s) Memory allocation failed\n",
				ptn->name);
		goto out_failed;
	}

	si->vid_hdr_offs = 0;
	si->image_seq = rand() & UBI_IMAGE_SEQ_BASE;
	si->vtbl_peb1 = -1;
	si->vtbl_peb2 = -1;
	si->fastmap_sb = -1;
	for (i = 0; i < ptn->length; i++){
		ret = read_ec_hdr(ptn->start + i, ec_hdr);
		switch (ret) {
		case 1:
			si->empty_cnt++;
			si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
			si->pebs_data[i].status = UBI_EMPTY_PEB;
			break;
		case 0:
			if (!si->vid_hdr_offs) {
				si->vid_hdr_offs = BE32(ec_hdr->vid_hdr_offset);
				si->data_offs = BE32(ec_hdr->data_offset);
				if (!si->vid_hdr_offs || !si->data_offs ||
					si->vid_hdr_offs % page_size ||
					si->data_offs % page_size) {
					si->bad_cnt++;
					si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
					si->vid_hdr_offs = 0;
					continue;
				}
				if (BE32(ec_hdr->vid_hdr_offset) != si->vid_hdr_offs) {
					si->bad_cnt++;
					si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
					continue;
				}
				if (BE32(ec_hdr->data_offset) != si->data_offs) {
					si->bad_cnt++;
					si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
					continue;
				}
			}
			si->read_image_seq = BE32(ec_hdr->image_seq);
			si->pebs_data[i].ec = BE64(ec_hdr->ec);
			/* Now read the VID header to find if the peb is free */
			ret = read_vid_hdr(ptn->start + i, &vid_hdr,
					BE32(ec_hdr->vid_hdr_offset));
			switch (ret) {
			case 1:
				si->pebs_data[i].status = UBI_FREE_PEB;
				si->free_cnt++;
				break;
			case 0:
				si->pebs_data[i].status = UBI_USED_PEB;
				si->pebs_data[i].volume = BE32(vid_hdr.vol_id);
				if (BE32(vid_hdr.vol_id) == UBI_LAYOUT_VOLUME_ID) {
					if (si->vtbl_peb1 == -1)
						si->vtbl_peb1 = i;
					else if (si->vtbl_peb2 == -1)
						si->vtbl_peb2 = i;
					else
						dprintf(CRITICAL,
							"scan_partition: Found > 2 copies of vtbl");
				}
				if (BE32(vid_hdr.vol_id) == UBI_FM_SB_VOLUME_ID)
					si->fastmap_sb = i;
				si->used_cnt++;
				break;
			case -1:
			default:
				si->bad_cnt++;
				si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
				si->pebs_data[i].status = UBI_BAD_PEB;
				break;
			}
			break;
		case -1:
		default:
			si->bad_cnt++;
			si->pebs_data[i].ec = UBI_MAX_ERASECOUNTER;
			si->pebs_data[i].status = UBI_BAD_PEB;
			break;
		}
	}

	/* Sanity check */
	if (si->bad_cnt + si->empty_cnt + si->free_cnt + si->used_cnt != (int)ptn->length) {
		dprintf(CRITICAL,"scan_partition: peb count doesn't sum up \n");
		goto out_failed;
	}

	/*
	 * If less then 95% of the PEBs were "bad" (didn't have valid
	 * ec header), then set mean_ec = UBI_DEF_ERACE_COUNTER.
	 */
	sum = 0;
	if ((si->free_cnt + si->used_cnt) &&
		(double)((si->free_cnt + si->used_cnt) / ptn->length) * 100 > 95) {
		for (i = 0; i < ptn->length; i++) {
			if (si->pebs_data[i].ec == UBI_MAX_ERASECOUNTER)
				continue;
			sum += si->pebs_data[i].ec;
		}
		si->mean_ec = sum / (si->free_cnt + si->used_cnt);
	} else {
		si->mean_ec = UBI_DEF_ERACE_COUNTER;
	}
	free(ec_hdr);
	return si;

out_failed:
	free(si->pebs_data);
out_failed_pebs:
	free(si);
	return NULL;
}

/**
 * update_ec_header() - Update provided ec_header
 * @si: pointer to struct ubi_scan_info, holding the collected
 * 		ec_headers information of the partition
 * @index: index in si->ec[] of this peb. Relative to ptn->start
 * @new_header: False if this is an update of an existing header.
 * 		True if new header needs to be filled in
 */
static void update_ec_header(struct ubi_ec_hdr *old_ech,
		const struct ubi_scan_info *si,
		int index, bool new_header)
{
	uint32_t crc;

	if (si->pebs_data[index].ec < UBI_MAX_ERASECOUNTER)
		old_ech->ec = BE64(si->pebs_data[index].ec + 1);
	else
		old_ech->ec = BE64(si->mean_ec);

	if (new_header) {
		old_ech->vid_hdr_offset = BE32(si->vid_hdr_offs);
		old_ech->data_offset = BE32(si->data_offs);
		old_ech->magic = BE32(UBI_EC_HDR_MAGIC);
		old_ech->version = UBI_VERSION;
	}
	old_ech->image_seq = BE32(si->image_seq);
	crc = mtd_crc32(UBI_CRC32_INIT,
			(const void *)old_ech, UBI_EC_HDR_SIZE_CRC);
	old_ech->hdr_crc = BE32(crc);
}


/**
 * update_vid_header() - Update provided vid_header
 * @si: pointer to struct ubi_scan_info, holding the collected
 *      vid_headers information of the partition
 * @vid_hdr: a &struct ubi_vid_hdr object where to store the new vid header
 * @old_vidh: a &struct ubi_vid_hdr object where to store the old vid header
 * @data_pad: how many bytes are not used at the end of physical eraseblocks to
 *            satisfy the requested alignment
 */
static void update_vid_header(struct ubi_vid_hdr *vid_hdr,
		struct ubi_vid_hdr *old_vidh,
		const struct ubi_scan_info *si, uint32_t data_pad)
{
	uint32_t crc;

	vid_hdr->vol_type = old_vidh->vol_type;
	vid_hdr->compat = 0;
	vid_hdr->vol_id = old_vidh->vol_id;
	vid_hdr->lnum = old_vidh->lnum;
	vid_hdr->data_size = old_vidh->data_size;
	vid_hdr->used_ebs = old_vidh->used_ebs;
	vid_hdr->sqnum = BE64(si->image_seq);
	vid_hdr->data_pad = BE32(data_pad);
	vid_hdr->data_crc = old_vidh->data_crc;

	vid_hdr->magic = BE32(UBI_VID_HDR_MAGIC);
	vid_hdr->version = UBI_VERSION;
	crc = mtd_crc32(UBI_CRC32_INIT,
			(const void *)vid_hdr, UBI_VID_HDR_SIZE_CRC);
	vid_hdr->hdr_crc = BE32(crc);
}

/**
 * get_layout_vol_vidh() - Read the second layout vid header
 * @si: pointer to struct ubi_scan_info, holding the collected
 *      ec_headers information of the partition
 * @ptn_start: Partition offset address
 * @vidh: a &struct ubi_vid_hdr object where to store the read header
 *
 * Returns: -1 on error
 *           0 on success
 */
static int get_layout_vol_vidh(struct ubi_scan_info *si,
			int ptn_start, struct ubi_vid_hdr *vidh)
{
	struct ubi_vid_hdr vid_hdr1, vid_hdr2;
	int ret = -1;

	if(read_vid_hdr(ptn_start + si->vtbl_peb1, &vid_hdr1,
			si->vid_hdr_offs) ){
		dprintf(CRITICAL,"update_layout_vol: read_vid_hdr failed\n");
		return ret;
	}

	if(read_vid_hdr(ptn_start + si->vtbl_peb2, &vid_hdr2,
			si->vid_hdr_offs) ){
		dprintf(CRITICAL,"update_layout_vol: read_vid_hdr failed\n");
		return ret;
	}

	if( BE64(vid_hdr1.sqnum) > BE64(vid_hdr2.sqnum))
		memcpy(vidh, &vid_hdr1, UBI_VID_HDR_SIZE);
	else
		memcpy(vidh, &vid_hdr2, UBI_VID_HDR_SIZE);

	return 0;
}

/**
 * update_layout_vol() - Read the second layout vid header
 * @si: pointer to struct ubi_scan_info, holding the collected
 *      ec_headers information of the partition
 * @data: vtbl LEBs buffer
 * @ptn: partition holding the required volume
 * @curr_peb: current PEB for new layout volume to write to
 * @new_vidh: a &struct ubi_vid_hdr object where to store the read header
 *
 * Returns: -1 on error
 *           0 on success
 */
static int update_layout_vol(struct ubi_scan_info *si,
		void *data, struct ptentry *ptn,
		int curr_peb, struct ubi_vid_hdr *new_vidh)
{
	struct ubi_vid_hdr vidh;
	unsigned page_size = flash_page_size();
	uint64_t sqnum;
	uint32_t crc, data_size;
	int lnum;

	memset(new_vidh, 0, UBI_VID_HDR_SIZE);
	if(get_layout_vol_vidh(si, ptn->start, new_vidh)){
		dprintf(CRITICAL,
			"update_layout_vol: Get ubi layout volume fail!\n");
		return -1;
	}
	sqnum = BE64(new_vidh->sqnum);

	memset(&vidh, 0, UBI_VID_HDR_SIZE);
	vidh.magic = new_vidh->magic;
	vidh.version = new_vidh->version;
	vidh.vol_type = UBI_VID_DYNAMIC;
	vidh.copy_flag = 1;
	vidh.compat = UBI_COMPAT_REJECT;
	vidh.vol_id = BE32(UBI_LAYOUT_VOLUME_ID);
	vidh.data_pad = new_vidh->data_pad;
	data_size = UBI_VTBL_RECORD_SIZE*UBI_MAX_VOLUMES;

	data_size = (data_size + page_size - 1) / page_size*page_size;
	vidh.data_size = BE32(data_size);

	for (lnum=0; curr_peb < (int)ptn->length && lnum < 2;
							curr_peb++) {
		if (si->pebs_data[curr_peb].status != UBI_FREE_PEB &&
			si->pebs_data[curr_peb].status != UBI_EMPTY_PEB)
			continue;

		sqnum++;
		vidh.lnum =  BE32(lnum);
		vidh.sqnum=  BE64(sqnum);
		crc = mtd_crc32(UBI_CRC32_INIT,(const void *)data, data_size);
		vidh.data_crc = BE32(crc);

		crc = mtd_crc32(UBI_CRC32_INIT,
				(const void *)&vidh, UBI_VID_HDR_SIZE_CRC);
		vidh.hdr_crc = BE32(crc);

		if (write_vid_header(curr_peb + ptn->start, &vidh, si->vid_hdr_offs)) {
			dprintf(CRITICAL,
					"update_layout_vol: write_vid_header for peb %d failed \n",
					curr_peb);
			return -1;
		}

		/* now write the data */
		if (write_leb_data(curr_peb + ptn->start, data, data_size, si->data_offs))
			dprintf(CRITICAL, "update_layout_vol: writing data to peb-%d failed\n",
					curr_peb);
		else
			si->pebs_data[curr_peb].status = UBI_USED_PEB;

		lnum++;
	}
	if(lnum != 2){
		dprintf(CRITICAL,
				"update_layout_vol: write fail, lnum %d\n",lnum);
		return -1;
	}

	if(si->vtbl_peb1 != -1 ){
		if (ubi_erase_peb(ptn->start + si->vtbl_peb1, si, ptn->start)) {
			dprintf(CRITICAL,
				"update_layout_vol: Erase old vtbl fail,vtbl_peb1 %d\n", si->vtbl_peb1);
		}
	}
	if(si->vtbl_peb2 != -1 ){
		if (ubi_erase_peb(ptn->start + si->vtbl_peb2, si, ptn->start)) {
			dprintf(CRITICAL,
				"update_layout_vol: Erase old vtbl fail,vtbl_peb2 %d\n", si->vtbl_peb2);
		}
	}
	return 0;
}
/**
 * fastmap_present - returns true if Fastmap superblock is found
 * @data: raw data to test
 *
 * This function returns 1 if the provided PEB data contains
 * Fastmap superblock, 0 otherwise
 */
static int fastmap_present(const void *data){
	struct ubi_ec_hdr *ec_hdr = (struct ubi_ec_hdr *)(data);
	struct ubi_vid_hdr *vid_hdr;

	vid_hdr = (struct ubi_vid_hdr *)(data + BE32(ec_hdr->vid_hdr_offset));
	if (BE32(vid_hdr->vol_id) == UBI_FM_SB_VOLUME_ID)
		return 1;
	else
		return 0;
}

/**
 * ubi_erase_peb - Erase PEB and update EC header
 * @peb_num: number of the PEB to erase
 * @si: UBI scan information
 * @ptn_start: first PEB of the flashed partition
 *
 * This function erases the given PEB (if required) and writes a new EC
 * header for it.
 *
 * Returns: -1 on error
 *           0 on success
 */
static int ubi_erase_peb(int peb_num, struct ubi_scan_info *si,
		int ptn_start)
{
	struct ubi_ec_hdr new_ech;
	int page_size = flash_page_size();
	int num_pages_per_blk = flash_block_size() / page_size;
	int ret;

	if (qpic_nand_blk_erase(peb_num * num_pages_per_blk)) {
		dprintf(INFO, "ubi_erase_peb: erase of %d failed\n", peb_num);
		return -1;
	}
	memset(&new_ech, 0, sizeof(new_ech));
	update_ec_header(&new_ech, si, peb_num - ptn_start, true);

	/* Write new ec_header */
	ret = write_ec_header(peb_num, &new_ech);
	if (ret) {
		dprintf(CRITICAL, "ubi_erase_peb: write ec_header to %d failed\n",
				peb_num);
		return -1;
	}
	si->pebs_data[peb_num - ptn_start].status = UBI_FREE_PEB;
	return 0;
}

/**
 * remove_F_flag() - Turn off space-fixup flag in the ubifs superblock
 * @data: pointer to the peb to check in the flashed image
 *
 * The UBIFS Superblock will be located at LEB 0 of the image. LEB 0 will be
 * mapped as follows:
 * If the image contains Fastmap superblock:
 * - LEB 0 will be at PEB3
 * else:
 * - LEB 0 will be at PEB2
 */
static void remove_F_flag(const void *leb_data)
{
	struct ubifs_ch *ch;
	struct ubifs_sb_node *ubifs_sb;
	struct ubi_ec_hdr *ech;
	struct ubi_vid_hdr *vidh;
	int vol_id;

	ech = (struct ubi_ec_hdr *)leb_data;
	vidh = (struct ubi_vid_hdr *)(leb_data + BE32(ech->vid_hdr_offset));
	vol_id = BE32(vidh->vol_id);

	if (vol_id > UBI_MAX_VOLUMES &&
			vol_id != UBI_LAYOUT_VOLUME_ID &&
			vol_id != UBI_FM_SB_VOLUME_ID)
		return;

	ubifs_sb = (struct ubifs_sb_node *)(leb_data + BE32(ech->data_offset));
	ch = (struct ubifs_ch *)ubifs_sb;
/* SWISTART */
#ifndef SIERRA
	if (ch->node_type != UBIFS_SB_NODE)
#else
    if (ch->node_type != UBIFS_SB_NODE || BE32(ch->magic) != UBIFS_MAGIC)
#endif
/* SWISTOP */
		return;
	if (ubifs_sb->flags & UBIFS_FLG_SPACE_FIXUP) {
		ubifs_sb->flags &= (~UBIFS_FLG_SPACE_FIXUP);
		ch->crc = mtd_crc32(UBIFS_CRC32_INIT, (void *)ubifs_sb + 8,
				sizeof(struct ubifs_sb_node) - 8);
	}
}

/* SWISTART */
#ifdef SIERRA
/*
* get the volume numbers of UBI in this image
*/
int get_ubi_vol_num(void *buf, unsigned block_size, unsigned data_offs)
{
	int i = 0,volume_num = 0;
	struct ubi_vtbl_record *curr_vol;

	volume_num = (block_size - data_offs) / UBI_VTBL_RECORD_SIZE;
	if (volume_num > UBI_MAX_VOLUMES)
		volume_num = UBI_MAX_VOLUMES;

	/* Now search for total volume numbers */
	for (i = 0; i < volume_num; i++) {
		curr_vol = (struct ubi_vtbl_record *)
				(buf + UBI_VTBL_RECORD_SIZE*i);
		if (!curr_vol->vol_type)
			break;
	}
	volume_num = i;
	return volume_num;
}

/*
* Function scan_input_image will take the input "data" as an input image try to
* collect some info: vol_id commpat, last_leb_data_size and max_leb_block.
* all the result will store in struct input_image_info.
*/
int scan_input_image(void *data, struct ubi_image_scan_info *input_image_info, unsigned size,
							unsigned block_size, unsigned vid_hdr_offset,
							unsigned page_size, unsigned leb_size )
{
	unsigned pre_lnum =0, data_size =0;
	int blocks = 0, volume = 0;
	char * image_data = data;
	struct ubi_vid_hdr *my_vid_hdr=NULL;

	my_vid_hdr = malloc(UBI_VID_HDR_SIZE);
	if (!my_vid_hdr){
		dprintf(CRITICAL, "cannot allocate memory for my_vid_hdr!\n");
		return -1;
	}
	do{
		memset(my_vid_hdr, 0, UBI_VID_HDR_SIZE);
		memcpy(my_vid_hdr,data + ( blocks*block_size + vid_hdr_offset ),UBI_VID_HDR_SIZE);

		if(my_vid_hdr->lnum == 0)
		{
			if( blocks == 0 ){
				input_image_info[volume].compat_vid= my_vid_hdr->compat;
				input_image_info[volume].vol_id_vid= my_vid_hdr->vol_id;
			}else if( blocks == 2 ){
				input_image_info[volume].compat_leb= my_vid_hdr->compat;
				input_image_info[volume].vol_id_leb= my_vid_hdr->vol_id;
			}else if(blocks > 2){
				data_size= (calc_data_len(page_size, image_data + (blocks-1)*block_size + 2*page_size, leb_size))*page_size;
				input_image_info[volume].last_leb_data_size= data_size;
				input_image_info[volume].max_leb_block= pre_lnum;

				volume++;
				input_image_info[volume].compat_leb= my_vid_hdr->compat;
				input_image_info[volume].vol_id_leb= my_vid_hdr->vol_id;
			}
		}
		blocks++;
		pre_lnum = BE32(my_vid_hdr->lnum);
	}while(size > block_size*blocks);
	data_size= (calc_data_len(page_size, image_data + (blocks-1)*block_size + 2*page_size, leb_size))*page_size;
	input_image_info[volume].last_leb_data_size= data_size;
	input_image_info[volume].max_leb_block= pre_lnum;

	free(my_vid_hdr);
	return 0;
}

int switch_4k_to_2k_ubi_img(void *data, unsigned * size)
{
	struct ubi_ec_hdr *ec_hdr = (struct ubi_ec_hdr *)data;
	struct ubi_vid_hdr *vid_hdr;

	struct ubi_ec_hdr *my_ec_hdr=NULL;
	struct ubi_vid_hdr *my_vid_hdr=NULL;
	struct ubi_vtbl_record *my_vtbl_recode=NULL;
	struct ubi_image_scan_info *input_image_info=NULL;
	char * final_image_data;

	uint32_t crc;
	unsigned data_size = LEB_SIZE_2K;
	unsigned  blocks_4k=0, tmp_lnum =0;
	int blocks_2k=0,blocks_num = 0,tmp =0, tmp2 =0;
	int tmp_volume =0,volume_numbers = 0;
	int i;

//Check UBI hdr
	dprintf(CRITICAL, "\nec_hdr->magic=%x,Original data size=0x%x\n",BE32(ec_hdr->magic),*size);

	if( BE32(ec_hdr->magic) != UBI_EC_HDR_MAGIC )
	{
		dprintf(CRITICAL, "This binary format is not right!\n");
		return -1;
	}

//Check if it is 2k or 4k flash, if 2k do nothing!
	if( BE32(ec_hdr->vid_hdr_offset) == FLASH_PAGE_SIZE_2K )
	{
		dprintf(CRITICAL, "This is 2k pages binary, no need to change!\n");
		return 0;
	}

	my_ec_hdr = malloc(UBI_EC_HDR_SIZE);
	if (!my_ec_hdr){
		dprintf(CRITICAL, "cannot allocate memory for my_ec_hdr!\n");
		return -1;
	}
	my_vid_hdr = malloc(UBI_VID_HDR_SIZE);
	if (!my_vid_hdr){
		dprintf(CRITICAL, "cannot allocate memory for my_vid_hdr!\n");
		goto my_ec_out;
	}

//get orignal ec hdr data then we can get some info from it
	memset(my_ec_hdr, 0, UBI_EC_HDR_SIZE);
	memcpy(my_ec_hdr,ec_hdr,UBI_EC_HDR_SIZE);

//get orignal VID data
	vid_hdr = (struct ubi_vid_hdr *)(data + BE32(my_ec_hdr->vid_hdr_offset));
	memset(my_vid_hdr, 0, UBI_VID_HDR_SIZE);
	memcpy(my_vid_hdr,vid_hdr,UBI_VID_HDR_SIZE);

//get orignal vtbl data
	volume_numbers = get_ubi_vol_num(data + BE32(my_ec_hdr->vid_hdr_offset)*2,BLOCK_SIZE_4K,BE32(ec_hdr->data_offset));
	dprintf(CRITICAL, "volume_numbers=%d\n",volume_numbers);

	my_vtbl_recode = calloc(UBI_VTBL_RECORD_SIZE, volume_numbers);
	if (!my_vtbl_recode){
		dprintf(CRITICAL, "cannot calloc memory for my_vtbl_recode!\n");
		goto my_vid_out;
	}

//backup all orignal vtbl data
	memset(my_vtbl_recode, 0, UBI_VTBL_RECORD_SIZE*volume_numbers);
	memcpy(my_vtbl_recode,data + BE32(ec_hdr->data_offset),UBI_VTBL_RECORD_SIZE*volume_numbers);

	input_image_info= calloc(sizeof(struct ubi_image_scan_info), volume_numbers);
	if (!input_image_info){
		dprintf(CRITICAL, "cannot calloc memory for input_image_info!\n");
		goto my_vtbl_recode;
	}

//Scan the input image to get the special info and store in "input_image_info"
	memset(input_image_info, 0, (sizeof(struct ubi_image_scan_info))*volume_numbers);
	scan_input_image(data,input_image_info,*size,
					BLOCK_SIZE_4K, BE32(ec_hdr->vid_hdr_offset),
					FLASH_PAGE_SIZE_4K, LEB_SIZE_4K);

// update the vtabl that change from 4k to 2k.
	for(i=0;i< volume_numbers;i++){

		if(BE32(my_vtbl_recode[i].reserved_pebs) == input_image_info[i].max_leb_block + 1){
			my_vtbl_recode[i].reserved_pebs = SWAP_32( (input_image_info[i].max_leb_block)*2 + (input_image_info[i].last_leb_data_size/LEB_SIZE_2K +1) );
		}
		else
		{
			if(i == 0)
				//The first ubi volume will reserve one more reserved_pebs.
				my_vtbl_recode[i].reserved_pebs = BE32(SWAP_32(my_vtbl_recode[i].reserved_pebs) * 2 );
			else
				my_vtbl_recode[i].reserved_pebs = BE32(SWAP_32(my_vtbl_recode[i].reserved_pebs) * 2 - 1 );

		}

		//Some of the ubi volume just reserve an empty partition has no data, so not use any blocks_num.
		if( SWAP_32(my_vtbl_recode[i].reserved_pebs) !=0 )
			input_image_info[i].max_leb_block= (input_image_info[i].max_leb_block)*2 + (input_image_info[i].last_leb_data_size/LEB_SIZE_2K);

		crc = mtd_crc32(UBI_CRC32_INIT, my_vtbl_recode + i, UBI_VTBL_RECORD_SIZE_CRC);
		my_vtbl_recode[i].crc= SWAP_32(crc);

		//Some of the ubi volume just reserve an empty partition has no data, so not use any blocks_num.
		if( input_image_info[i].last_leb_data_size !=0 )
			blocks_num += input_image_info[i].max_leb_block +1;
	}
	blocks_num = blocks_num + 2;

//change 4k page info data to 2k info for 2k page flash
	my_ec_hdr->vid_hdr_offset=SWAP_32(FLASH_PAGE_SIZE_2K);
	my_ec_hdr->data_offset=SWAP_32(FLASH_PAGE_SIZE_2K*2);
	crc = mtd_crc32(UBI_CRC32_INIT, my_ec_hdr, UBI_EC_HDR_SIZE_CRC);
	my_ec_hdr->hdr_crc = SWAP_32(crc);

//use this address for final image data (80M affer the orignal data) .
	final_image_data = (char *)target_get_scratch_address() + CONVERTED_IMG_MEM_OFFSET;
	memset(final_image_data, 0xFF, *size);
	dprintf(CRITICAL,"Need blocks_num=%u\n",blocks_num);

//Set data for 2k page flash and write it to the buffer, use "tmp" as the counter of the loop
	tmp = blocks_num;

//Get the total block numbers that used of the first UBI volume in the image, the second UBI volume may start from this position.
	tmp2 = input_image_info[0].max_leb_block + 2 /* UBI header */ + 1 /* start from 0 */;

	do {
		if(blocks_2k < 2){

			my_vid_hdr->lnum = SWAP_32(blocks_2k);
			crc = mtd_crc32(UBI_CRC32_INIT, my_vid_hdr, UBI_VID_HDR_SIZE_CRC);
			my_vid_hdr->hdr_crc = SWAP_32(crc);

			memcpy(final_image_data + 0*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, my_ec_hdr, UBI_EC_HDR_SIZE);
			memcpy(final_image_data + 1*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, my_vid_hdr, UBI_VID_HDR_SIZE);
			memcpy(final_image_data + 2*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, data + 2*FLASH_PAGE_SIZE_4K , LEB_SIZE_2K);
			//update all the vtable volumes.
			memcpy(final_image_data + 2*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, my_vtbl_recode , UBI_VTBL_RECORD_SIZE*volume_numbers);
			blocks_2k++;
		}
		else
		{
			// Data in one 4K page need two 2k page to store
			for(i=0;i<2 && blocks_2k < blocks_num ;i++){

				//When blocks_2k reach to tmp2 mean that the different volume block position is comming
				if( blocks_2k == tmp2 && tmp_volume < volume_numbers-1 ){
					tmp_volume++;
					//get the current block position is in which volume of UBI in the input image.
					tmp2 += input_image_info[tmp_volume].max_leb_block + 1;
					tmp_lnum = 0;
				}
				//update the special data info
				my_vid_hdr->vol_type= my_vtbl_recode[tmp_volume].vol_type;
				my_vid_hdr->vol_id = input_image_info[tmp_volume].vol_id_leb;
				my_vid_hdr->compat = input_image_info[tmp_volume].compat_leb;
				my_vid_hdr->lnum = SWAP_32(tmp_lnum);

				//check the data not "FF" and return the real data size
				data_size= (calc_data_len(FLASH_PAGE_SIZE_2K, data + blocks_4k*BLOCK_SIZE_4K + 2*FLASH_PAGE_SIZE_4K + LEB_SIZE_2K*i, LEB_SIZE_2K))*FLASH_PAGE_SIZE_2K;
				if( data_size < LEB_SIZE_2K)
					dprintf(CRITICAL,"data_size=%u, blocks_2k=%d, blocks_num=%d\n",data_size,blocks_2k,blocks_num);
				if( data_size > LEB_SIZE_2K)
					data_size = LEB_SIZE_2K;

				if( data_size <= 0 || SWAP_32(my_vtbl_recode[tmp_volume].reserved_pebs) <= 0 ){
					continue;
				}

				//static type of UBI volume need to store data size and crc in header.
				if(my_vtbl_recode[tmp_volume].vol_type == 0x2){

					// used_ebs is always equal "lnum + 1"
					if(tmp_lnum >= input_image_info[tmp_volume].max_leb_block + 1)
						break;

					my_vid_hdr->used_ebs= SWAP_32(input_image_info[tmp_volume].max_leb_block + 1);
					my_vid_hdr->data_size= SWAP_32(data_size);
					crc = mtd_crc32(UBI_CRC32_INIT, data + blocks_4k*BLOCK_SIZE_4K + 2*FLASH_PAGE_SIZE_4K + LEB_SIZE_2K*i, data_size);
					my_vid_hdr->data_crc= SWAP_32(crc);
				}
				else{
					//my_vtbl_recode[tmp_volume].vol_type = 0x1;
					my_vid_hdr->used_ebs= 0;
					my_vid_hdr->data_size= 0;
					my_vid_hdr->data_crc= 0;
				}

				crc = mtd_crc32(UBI_CRC32_INIT, my_vid_hdr, UBI_VID_HDR_SIZE_CRC);
				my_vid_hdr->hdr_crc = SWAP_32(crc);

				memcpy(final_image_data + 0*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, my_ec_hdr, UBI_EC_HDR_SIZE);
				memcpy(final_image_data + 1*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, my_vid_hdr, UBI_VID_HDR_SIZE);
				memcpy(final_image_data + 2*FLASH_PAGE_SIZE_2K + blocks_2k*BLOCK_SIZE_2K, data + blocks_4k*BLOCK_SIZE_4K + 2*FLASH_PAGE_SIZE_4K + LEB_SIZE_2K*i, LEB_SIZE_2K);
				blocks_2k++;
				tmp_lnum++;

			}
		}
		blocks_4k++;
	}while( (--tmp) && (*size > blocks_4k*BLOCK_SIZE_4K - 1) );

// Blocks align
	*size = blocks_2k*BLOCK_SIZE_2K;
	memset(data, 0xFF, *size);
	memcpy(data,final_image_data,*size);
	dprintf(CRITICAL, "Converted data size=0x%x,end data_size=0x%x,blocks numbers: %d\n",*size,data_size,blocks_2k);

	free(my_ec_hdr);
	free(my_vid_hdr);
	free(my_vtbl_recode);
	free(input_image_info);
	return 0;

my_vtbl_recode:
	free(my_vtbl_recode);
my_vid_out:
	free(my_vid_hdr);
my_ec_out:
	free(my_ec_hdr);
	return -1;
}
#endif
/* SWISTOP */

/**
 * flash_ubi_img() - Write the provided (UBI) image to given partition
 * @ptn: partition to write the image to
 * @data: the image to write
 * @size: size of the image to write
 *
 *  Return codes:
 * -1 - in case of error
 *  0 - on success
 */
int flash_ubi_img(struct ptentry *ptn, void *data, unsigned size)
{
	struct ubi_scan_info *si;
	struct ubi_ec_hdr *old_ech;
	uint32_t curr_peb = ptn->start;
	void *img_peb;
	unsigned page_size = flash_page_size();
	unsigned block_size = flash_block_size();
	int num_pages_per_blk = block_size / page_size;
	int num_pages;
	int ret;
	int bad_blocks_cnt = 0;
	uint32_t fmsb_peb = UINT_MAX;
	int is_fmsb_peb_valid = 0;
	unsigned peb_valid_sz= 0;

	si = scan_partition(ptn);
	if (!si) {
		dprintf(CRITICAL, "flash_ubi_img: scan_partition failed\n");
		return -1;
	}

/* SWISTART */
#ifdef SIERRA
	if( page_size == 2048 ){
		if(switch_4k_to_2k_ubi_img(data,&size))
		{
			dprintf(CRITICAL, "flash_ubi_img: switch_4k_to_2k_ubi_img failed\n");
			return -1;
		}
	}
#endif
/* SWISTOP */

	/*
	 * In case si->vid_hdr_offs is still -1 (non UBI image was
	 * flashed on device, get the value from the image to flush
	 */
	if (!si->vid_hdr_offs){
		struct ubi_ec_hdr *echd = (struct ubi_ec_hdr *)data;
		si->vid_hdr_offs = BE32(echd->vid_hdr_offset);
		si->data_offs = BE32(echd->data_offset);
	}

	/* Update the "to be" flashed image and flash it */
	img_peb = data;
	while (size && curr_peb < ptn->start + ptn->length) {
		if (qpic_nand_blk_erase(curr_peb * num_pages_per_blk)) {
			dprintf(CRITICAL, "flash_ubi_img: erase of %d failed\n",
				curr_peb);
			bad_blocks_cnt++;
			curr_peb++;
			continue;
		}

		if (size < block_size)
			num_pages = size / page_size;
		else
			num_pages = calc_data_len(page_size, img_peb, block_size);

		/* Total size of valid data in peb */
		peb_valid_sz = num_pages * page_size;

		if (size < UBI_MAGIC_SIZE)
		{
			dprintf(CRITICAL, "flash_ubi_img: invalid size provided.\n");
			return -1;
		}

		/*
		* Check for oob access if any in img_peb.
		*/
		if (memcmp(img_peb, UBI_MAGIC, UBI_MAGIC_SIZE) ||
			BE32(((struct ubi_ec_hdr *)img_peb)->vid_hdr_offset) > peb_valid_sz ||
			BE32(((struct ubi_ec_hdr *)img_peb)->data_offset) > peb_valid_sz)
		{
			dprintf(CRITICAL, "flash_ubi_img: invalid image peb found\n");
			return -1;
		}

		remove_F_flag(img_peb);
		/* Update the ec_header in the image */
		old_ech = (struct ubi_ec_hdr *)img_peb;
		update_ec_header(old_ech, si, curr_peb - ptn->start, false);
		/* Write one block from image */
		ret = qpic_nand_write(curr_peb * num_pages_per_blk,
				num_pages, img_peb, 0);
		if (ret) {
			dprintf(CRITICAL, "flash_ubi_img: writing to peb-%d failed\n",
					curr_peb);
			bad_blocks_cnt++;
			curr_peb++;
			continue;
		}
		if (size < block_size)
			size = 0;
		else
			size -= block_size;

		if (fastmap_present(img_peb)) {
			fmsb_peb = curr_peb;
			is_fmsb_peb_valid = 1;
		}
		img_peb += flash_block_size();
		curr_peb++;
	}

	if (size) {
		dprintf(CRITICAL,
				"flash_ubi_img: Not enough good blocks to flash image!");
		ret = -1;
		goto out;
	}

	/* Erase and write ec_header for the rest of the blocks */
	for (; curr_peb < ptn->start + ptn->length; curr_peb++)
		if (ubi_erase_peb(curr_peb, si, ptn->start))
			bad_blocks_cnt++;

	ret = 0;
	/*
	 * If flashed image contains fastmap data and bad blocks were found
	 * we need to invalidate the flashed fastmap since it isn't accurate
	 * anymore.
	 */
	if (bad_blocks_cnt && (is_fmsb_peb_valid == 1)) {
		dprintf(CRITICAL, "flash_ubi_img: invalidate fmsb (fmsb_peb = %u)\n",
			fmsb_peb);
		ret = ubi_erase_peb(fmsb_peb, si, ptn->start);
	}

out:
	free(si->pebs_data);
	free(si);
	return ret;
}

/**
 * find_volume() - Find given volume in a partition by it's name
 * @si: pointer to struct ubi_scan_info
 * @ptn_start: PEB number the partition begins at
 * @vol_name: name of the volume to search for
 * @vol_info: info obout the found volume
 *
 * This functions reads the volume table, then iterates over all its records
 * and searches for a volume with a given name. If found, the volume table
 * record describing this volume is returned at @vol_info. The volume
 * id returned as a return code of the function.
 *
 * Returns:
 * -1 - if the volume was not found
 * volume in dex when found
 */
static int find_volume(struct ubi_scan_info *si, int ptn_start,
		const char *vol_name, struct ubi_vtbl_record *vol_info,
		void *leb_mem_buf)
{
	int i, vtbl_records, vtbl_peb, ret = -1;
	int block_size = flash_block_size();
	void *leb_data;
	struct ubi_vtbl_record *curr_vol;

	if (si->vtbl_peb1 < 0) {
		dprintf(CRITICAL,"find_volume: vtbl not found \n");
		return -1;
	}
	vtbl_peb = si->vtbl_peb1;

	vtbl_records = (block_size - si->data_offs) / UBI_VTBL_RECORD_SIZE;
	if (vtbl_records > UBI_MAX_VOLUMES)
		vtbl_records = UBI_MAX_VOLUMES;

#ifdef SIERRA
	leb_data = leb_mem_buf;
	memset(leb_data, 0, block_size - si->data_offs);
#else
	leb_data = malloc(block_size - si->data_offs);
	if (!leb_data) {
		dprintf(CRITICAL,"find_volume: Memory allocation failed\n");
		goto out_free_leb;
	}
#endif
retry:
	/* First read the volume table */
	if (read_leb_data(vtbl_peb + ptn_start, leb_data,
			block_size - si->data_offs, si->data_offs)) {
		dprintf(CRITICAL,"find_volume: read_leb_data failed\n");
		if (vtbl_peb == si->vtbl_peb1 && si->vtbl_peb2 != -1) {
			vtbl_peb = si->vtbl_peb2;
			goto retry;
		}
		goto out_free_leb;
	}

	/* Now search for the required volume ID */
	for (i = 0; i < vtbl_records; i++) {
		curr_vol = (struct ubi_vtbl_record *)
				(leb_data + UBI_VTBL_RECORD_SIZE*i);
		if (!curr_vol->vol_type)
			continue;
		if (!strcmp((char *)curr_vol->name, vol_name)) {
			ret = i;
			memcpy((void*)vol_info, curr_vol, sizeof(struct ubi_vtbl_record));
			break;
		}
	}

out_free_leb:
#ifndef SIERRA
	free(leb_data);
#endif
	return ret;
}

/**
 * write_one_peb() - writes data to a PEB, including VID header
 * @curr_peb - PEB to write to
 * @ptn_start: number of first PEB of the partition
 * @si: pointer to struct ubi_scan_info
 * @new_vidh: a struct ubi_vid_hdr object containing VID header
 * @data: data to write
 * @size: size of the data
 *
 * Assumption: EC header correctly written and PEB erased
 *
 * Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int write_one_peb(int curr_peb, int ptn_start,
		struct ubi_scan_info *si,
		struct ubi_vid_hdr *new_vidh, void* data, int size)
{
	int ret;
	struct ubi_vid_hdr vidh;

	memset((void *)&vidh, 0, UBI_VID_HDR_SIZE);
	update_vid_header(&vidh, new_vidh, si, 0);
	if (write_vid_header(curr_peb + ptn_start, &vidh, si->vid_hdr_offs)) {
		dprintf(CRITICAL,
				"update_ubi_vol: write_vid_header for peb %d failed \n",
				curr_peb);
		ret = -1;
		goto out;
	}

	/* now write the data */
	ret = write_leb_data(curr_peb + ptn_start, data, size, si->data_offs);
	if (ret)
		dprintf(CRITICAL, "update_ubi_vol: writing data to peb-%d failed\n",
				curr_peb);
	else
		si->pebs_data[curr_peb].status = UBI_USED_PEB;
out:
	return ret;
}

/**
 * get_used_lebs() - Get PEBs used by all the ubi volumes
 * @leb_size: size of LEB
 * @leb_mem_buf: vtbl buffer containing volume informations
 *
 *  Return codes:
 * -1 - in case of error
 *  0 - on success
 */
static int get_used_lebs(unsigned leb_size, void * leb_mem_buf)
{
	int vtbl_records, i, used_lebs = 0, vol_num = 0;
	struct ubi_vtbl_record *vtbl;

	vtbl_records = leb_size / UBI_VTBL_RECORD_SIZE;
	if (vtbl_records > UBI_MAX_VOLUMES)
		vtbl_records = UBI_MAX_VOLUMES;

	for (i = 0; i < vtbl_records; i++) {
		vtbl = (struct ubi_vtbl_record *)
				(leb_mem_buf + UBI_VTBL_RECORD_SIZE*i);
		if (!vtbl->vol_type)
			continue;
		used_lebs += BE32(vtbl->reserved_pebs);
		vol_num++;
	}
	return used_lebs;
}

/**
 * update_ubi_vol() - Write the provided (UBI) image to given volume
 * @ptn: partition holding the required volume
 * @data: the image to write
 * @size: size of the image to write
 *
 *  Return codes:
 * -1 - in case of error
 *  0 - on success
 */
int update_ubi_vol(struct ptentry *ptn, const char* vol_name,
				void *data, unsigned size)
{
	struct ubi_scan_info *si;
	int vol_id, vol_pebs, curr_peb = 0, ret = -1;
	unsigned block_size = flash_block_size();
	unsigned page_size = flash_page_size();
	unsigned leb_size;
	void *img_peb, *leb_mem_buf;
	struct ubi_vtbl_record curr_vol;
	struct ubi_vtbl_record *vtbl, *vtbl_bak;
	struct ubi_vid_hdr *new_vidh;
	uint32_t data_size, crc;
	int img_pebs, tmp_pebs = 0, lnum = 0, reserve_pebs = 0;

	si = scan_partition(ptn);
	if (!si) {
		dprintf(CRITICAL, "update_ubi_vol: scan_partition failed\n");
		return -1;
	}
	leb_size = block_size - si->data_offs;

	leb_mem_buf = (unsigned char *)target_get_scratch_address() + UBI_VOL_UPDATE_IMG_MEM_OFFSET;
	vtbl_bak = leb_mem_buf + leb_size;
	memset(vtbl_bak, 0, leb_size);

	new_vidh = malloc(UBI_VID_HDR_SIZE);
	if (!new_vidh) {
		dprintf(CRITICAL,
			"update_ubi_vol: new_vidh memory allocation failed\n");
		goto out_vid;
	}
	memset(new_vidh, 0, UBI_VID_HDR_SIZE);

	if (si->read_image_seq)
		si->image_seq = si->read_image_seq;

	vol_id = find_volume(si, ptn->start, vol_name, &curr_vol, leb_mem_buf);
	if (vol_id == -1) {
		dprintf(CRITICAL, "update_ubi_vol: dint find volume\n");
		goto out;
	}

	if (si->fastmap_sb > -1 &&
			ubi_erase_peb(ptn->start + si->fastmap_sb, si, ptn->start)) {
		dprintf(CRITICAL, "update_ubi_vol: fastmap invalidation failed\n");
		goto out;
	}

	vol_pebs = BE32(curr_vol.reserved_pebs);
	tmp_pebs = get_used_lebs(leb_size,leb_mem_buf) - vol_pebs;
	reserve_pebs = (int)ptn->length - si->bad_cnt - tmp_pebs - 2;

	img_pebs = size / leb_size;
	if (size % leb_size)
		img_pebs++;

	if ( (reserve_pebs - img_pebs) < (reserve_pebs*4/100 + 2) ) {
		dprintf(CRITICAL,
			"update_ubi_vol: Provided image is too big. Requires %d PEBs, avail. only %d\n",
				reserve_pebs*4/100 + 2 + img_pebs, reserve_pebs);
		goto out;
	}

	/* First erase all volume used PEBs */
	curr_peb = 0;
	while (curr_peb < (int)ptn->length) {
		if (si->pebs_data[curr_peb].status != UBI_USED_PEB ||
				si->pebs_data[curr_peb].volume != vol_id) {
			curr_peb++;
			continue;
		}
		if (ubi_erase_peb(ptn->start + curr_peb, si, ptn->start))
			goto out;
		curr_peb++;
	}

	/* Get free PEBs */
	curr_peb = 0;
	tmp_pebs = 0;
	while (curr_peb < (int)ptn->length) {
		if (si->pebs_data[curr_peb].status == UBI_FREE_PEB ||
			si->pebs_data[curr_peb].status == UBI_EMPTY_PEB) {
			tmp_pebs++;
		}
		curr_peb++;
	}
	reserve_pebs = tmp_pebs - 2;/* leave 2 for vtble header */

	/* Keep at least 4% empty blocks for UBI wear-leveling and bad block handling */
	if( (reserve_pebs - img_pebs) < (reserve_pebs*4/100 + 2) ){
		dprintf(CRITICAL,
			"update_ubi_vol: Provided img is too big. need at least 4 percent empty blocks!\n");
		goto out;
	}

	new_vidh->vol_type = curr_vol.vol_type;
	new_vidh->vol_id = BE32(vol_id);
	if(curr_vol.vol_type != UBI_VID_DYNAMIC){
		new_vidh->used_ebs = BE32(img_pebs);
	}

	/* Flash the image */
	img_peb = data;
	lnum = 0;
	for (curr_peb = 0;
			curr_peb < (int)ptn->length && size && reserve_pebs;
			curr_peb++) {
		if (si->pebs_data[curr_peb].status != UBI_FREE_PEB &&
			si->pebs_data[curr_peb].status != UBI_EMPTY_PEB){
			continue;
		}

		data_size = (size < leb_size ? size : leb_size);
		if( data_size > page_size)
			data_size = (calc_data_len(page_size, img_peb,
					data_size) * page_size);

		crc = mtd_crc32(UBI_CRC32_INIT, img_peb, data_size);
		new_vidh->lnum = BE32(lnum);
		lnum++;
		if(curr_vol.vol_type != UBI_VID_DYNAMIC){
			new_vidh->data_size = BE32(data_size);
			new_vidh->data_crc= BE32(crc);
		}

		if (write_one_peb(curr_peb, ptn->start, si,
				new_vidh, img_peb, data_size)) {
			dprintf(CRITICAL, "update_ubi_vol: write_one_peb failed\n");
			goto out;
		}

		if (size < leb_size)
			size = 0;
		else
			size -= leb_size;
		reserve_pebs--;
		img_peb += leb_size;
	}

	if (size) {
		dprintf(CRITICAL,
			"update_ubi_vol: Not enough available PEBs for writing the volume\n");
		goto out;
	}

	/* Update layout volume - VID header and vtble */
	if(vol_pebs < img_pebs){
		vtbl = (struct ubi_vtbl_record *)
				(leb_mem_buf + UBI_VTBL_RECORD_SIZE*vol_id);

		vtbl->reserved_pebs = BE32(img_pebs);
		crc = mtd_crc32(UBI_CRC32_INIT,
				(const void *)vtbl, UBI_VTBL_RECORD_SIZE_CRC);
		vtbl->crc = BE32(crc);

		if(update_layout_vol(si, leb_mem_buf, ptn, curr_peb, new_vidh)){
			dprintf(CRITICAL,
				"update_ubi_vol: Get ubi layout volume fail!\n");
			goto out;
		}
	}
	ret = 0;

out:
	free(new_vidh);
out_vid:
	free(si->pebs_data);
	free(si);
	return ret;
}

/**
 * get_ubi_vol_data() - Get the provided (UBI) volume data from UBI partition
 * @ptn: partition holding the required UBI volume
 * @data: the data of the UBI volume store in this pointer.
 * @size: size of the UBI volume get from UBI partition
 *
 *  Return codes:
 * -1 - in case of error
 *  0 - on success
 */
int get_ubi_vol_data(struct ptentry *ptn, const char* vol_name,
				void *data, unsigned *size)
{
	struct ubi_scan_info *si;
	int vol_id, curr_peb = 0, ret = -1;
	unsigned block_size = flash_block_size();

	struct ubi_vtbl_record curr_vol;
	struct ubi_vid_hdr vid_hdr;
	unsigned lnum = 0;
	void *leb_mem_buf;

	si = scan_partition(ptn);
	if (!si) {
		dprintf(CRITICAL, "get_ubi_vol_data: scan_partition failed\n");
		return -1;
	}
	leb_mem_buf = (unsigned char *)target_get_scratch_address() + UBI_VOL_UPDATE_IMG_MEM_OFFSET;

/* Get vol id */
	vol_id = find_volume(si, ptn->start, vol_name, &curr_vol, leb_mem_buf);
	if (vol_id == -1) {
		dprintf(CRITICAL, "get_ubi_vol_data: volume %s not found!\n",vol_name);
		goto out;
	}

/* Get volume data */
	curr_peb = 0;
	while (curr_peb < (int)ptn->length) {
		if (si->pebs_data[curr_peb].status == UBI_USED_PEB &&
			si->pebs_data[curr_peb].volume == vol_id) {

			if(read_vid_hdr(ptn->start + curr_peb, &vid_hdr,
					si->vid_hdr_offs) ){
				dprintf(CRITICAL,"get_ubi_vol_data: read_vid_hdr failed\n");
				goto out;
			}
			if (read_leb_data(ptn->start + curr_peb,
					data + (block_size - si->data_offs)*BE32(vid_hdr.lnum),
					block_size - si->data_offs, si->data_offs)) {
				dprintf(CRITICAL,"get_ubi_vol_data: read_leb_data failed\n");
				goto out;
			}

			if(BE32(vid_hdr.lnum) > lnum)
				lnum = BE32(vid_hdr.lnum);
		}
		curr_peb++;
	}

	*size = (lnum+1)*(block_size - si->data_offs);
	ret = 0;

out:
	free(si->pebs_data);
	free(si);
	return ret;
}
