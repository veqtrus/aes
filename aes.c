/*
Copyright (c) 2020 Pavlos Georgiou

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
https://www.boost.org/LICENSE_1_0.txt
*/

#include "aes.h"

#include <string.h>

static const uint8_t sbox[256] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t rsbox[256] = {
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const uint8_t rcon[11] = { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

#define GDBL(n) (((n) << 1) ^ (0x1B & -(const uint8_t)(((n) >> 7) & 1)))

#define GMUL(arr, row, col, n) ( \
	(arr[0].bytes[4 * col + row] & -(const uint8_t)((n) & 1)) ^ \
	(arr[1].bytes[4 * col + row] & -(const uint8_t)((n >> 1) & 1)) ^ \
	(arr[2].bytes[4 * col + row] & -(const uint8_t)((n >> 2) & 1)) ^ \
	(arr[3].bytes[4 * col + row] & -(const uint8_t)((n >> 3) & 1)) )

#define GDOT(arr, col, a0, a1, a2, a3) ( \
	GMUL(arr, 0, col, a0) ^ \
	GMUL(arr, 1, col, a1) ^ \
	GMUL(arr, 2, col, a2) ^ \
	GMUL(arr, 3, col, a3) )

#define BXOR(dst, src) do { \
		(dst).words[0] ^= (src).words[0]; \
		(dst).words[1] ^= (src).words[1]; \
		(dst).words[2] ^= (src).words[2]; \
		(dst).words[3] ^= (src).words[3]; \
	} while (0)

static void incr_counter(union vial_aes_block *counter)
{
	unsigned i = sizeof(counter->bytes);
	do {
		--i;
	} while (++(counter->bytes[i]) == 0 && i != 0);
}

static void expand_keys(uint32_t *w, unsigned n, unsigned r)
{
	uint8_t *p;
	unsigned i;
	for (i = n; i < 4 * r; ++i) {
		if (i % n == 0) {
			p = (uint8_t *) (w + i - 1);
			p[4] = sbox[p[1]] ^ rcon[i / n];
			p[5] = sbox[p[2]];
			p[6] = sbox[p[3]];
			p[7] = sbox[p[0]];
			w[i] ^= w[i - n];
		} else if (n > 6 && i % n == 4) {
			p = (uint8_t *) (w + i - 1);
			p[4] = sbox[p[0]];
			p[5] = sbox[p[1]];
			p[6] = sbox[p[2]];
			p[7] = sbox[p[3]];
			w[i] ^= w[i - n];
		} else {
			w[i] = w[i - n] ^ w[i - 1];
		}
	}
}

static void aes_encrypt(union vial_aes_block *restrict blk, const union vial_aes_block *restrict keys, unsigned rounds)
{
	union vial_aes_block tblk[4];
	unsigned i, j, r;
	for (r = 0; ; ++r) {
		/* AddRoundKey */
		BXOR(*blk, keys[r]);
		/* SubBytes */
		/* ShiftRows */
		for (i = 0; i < 4; ++i)
			for (j = 0; j < 4; ++j)
				tblk[0].bytes[4 * i + j] = sbox[blk->bytes[4 * ((i + j) & 3) + j]];
		if (r == rounds - 1)
			break;
		/* MixColumns */
		for (i = 0; i < VIAL_AES_BLOCK_SIZE; ++i)
			tblk[1].bytes[i] = GDBL(tblk[0].bytes[i]);
		for (i = 0; i < 4; ++i) {
			blk->bytes[4 * i + 0] = GDOT(tblk, i, 2, 3, 1, 1);
			blk->bytes[4 * i + 1] = GDOT(tblk, i, 1, 2, 3, 1);
			blk->bytes[4 * i + 2] = GDOT(tblk, i, 1, 1, 2, 3);
			blk->bytes[4 * i + 3] = GDOT(tblk, i, 3, 1, 1, 2);
		}
	}
	/* AddRoundKey */
	blk->words[0] = tblk[0].words[0] ^ keys[rounds].words[0];
	blk->words[1] = tblk[0].words[1] ^ keys[rounds].words[1];
	blk->words[2] = tblk[0].words[2] ^ keys[rounds].words[2];
	blk->words[3] = tblk[0].words[3] ^ keys[rounds].words[3];
}

static void aes_decrypt(union vial_aes_block *restrict blk, const union vial_aes_block *restrict keys, unsigned rounds)
{
	union vial_aes_block tblk[4];
	unsigned i, j, r;
	/* AddRoundKey */
	BXOR(*blk, keys[rounds]);
	for (r = rounds - 1; ; --r) {
		/* ShiftRows */
		/* SubBytes */
		for (i = 0; i < 4; ++i)
			for (j = 0; j < 4; ++j)
				tblk[0].bytes[4 * i + j] = rsbox[blk->bytes[4 * ((i - j) & 3) + j]];
		/* AddRoundKey */
		BXOR(tblk[0], keys[r]);
		if (r == 0)
			break;
		/* MixColumns */
		for (j = 0; j < 3; ++j)
			for (i = 0; i < VIAL_AES_BLOCK_SIZE; ++i)
				tblk[j + 1].bytes[i] = GDBL(tblk[j].bytes[i]);
		for (i = 0; i < 4; ++i) {
			blk->bytes[4 * i + 0] = GDOT(tblk, i, 14, 11, 13,  9);
			blk->bytes[4 * i + 1] = GDOT(tblk, i,  9, 14, 11, 13);
			blk->bytes[4 * i + 2] = GDOT(tblk, i, 13,  9, 14, 11);
			blk->bytes[4 * i + 3] = GDOT(tblk, i, 11, 13,  9, 14);
		}
	}
	*blk = tblk[0];
}

static void aes_ctr_pad(struct vial_aes *self, uint8_t *dst, const uint8_t *src, size_t len)
{
	while (len > 0) {
		if (self->pad_rem == 0) {
			self->pad = self->iv;
			aes_encrypt(&self->pad, self->keys, self->rounds);
			incr_counter(&self->iv);
			self->pad_rem = VIAL_AES_BLOCK_SIZE;
		}
		*dst = *src ^ self->pad.bytes[VIAL_AES_BLOCK_SIZE - self->pad_rem];
		self->pad_rem--;
		len--;
		src++;
		dst++;
	}
}

static void cbcmac(const struct vial_aes *self, union vial_aes_block *mac, const uint8_t *data, size_t len)
{
	union vial_aes_block blk;
	while (len >= VIAL_AES_BLOCK_SIZE) {
		memcpy(&blk, data, VIAL_AES_BLOCK_SIZE);
		BXOR(*mac, blk);
		aes_encrypt(mac, self->keys, self->rounds);
		len -= VIAL_AES_BLOCK_SIZE;
		data += VIAL_AES_BLOCK_SIZE;
	}
	if (len > 0) {
		memset(&blk, 0, VIAL_AES_BLOCK_SIZE);
		memcpy(&blk, data, len);
		BXOR(*mac, blk);
		aes_encrypt(mac, self->keys, self->rounds);
	}
}

static enum vial_aes_error cbcmac_tag(const struct vial_aes *self, uint8_t *dst, const uint8_t *src, size_t len)
{
	size_t i, tmp_len;
	union vial_aes_block tag = {{0}}, blk = {{0}};
	/* set flags */
	tag.bytes[0] = (self->auth_data_len > 0 ? 64 : 0) +
		8 * (self->tag_len / 2 - 1) + (self->counter_len - 1);
	/* set counter */
	tmp_len = len;
	for (i = 0; i < self->counter_len; ++i) {
		tag.bytes[VIAL_AES_BLOCK_SIZE - 1 - i] = tmp_len & 255;
		tmp_len >>= 8;
	}
	if (tmp_len > 0)
		return VIAL_AES_ERROR_LENGTH;
	memcpy(tag.bytes + 1, self->iv.bytes + self->counter_len + 1, VIAL_AES_BLOCK_SIZE - 1 - self->counter_len);
	aes_encrypt(&tag, self->keys, self->rounds);
	if (self->auth_data_len < 0xFF00) {
		blk.bytes[0] = self->auth_data_len >> 8;
		blk.bytes[1] = self->auth_data_len & 255;
		i = 2;
	} else if (self->auth_data_len <= 0xFFFFFFFF) {
		blk.bytes[0] = 0xFF;
		blk.bytes[1] = 0xFE;
		tmp_len = self->auth_data_len;
		blk.bytes[5] = tmp_len & 255;
		tmp_len >>= 8;
		blk.bytes[4] = tmp_len & 255;
		tmp_len >>= 8;
		blk.bytes[3] = tmp_len & 255;
		tmp_len >>= 8;
		blk.bytes[2] = tmp_len & 255;
		i = 6;
	} else {
		return VIAL_AES_ERROR_LENGTH;
	}
	tmp_len = VIAL_AES_BLOCK_SIZE - i;
	if (self->auth_data_len < tmp_len)
		tmp_len = self->auth_data_len;
	memcpy(blk.bytes + i, self->auth_data, tmp_len);
	BXOR(tag, blk);
	aes_encrypt(&tag, self->keys, self->rounds);
	cbcmac(self, &tag, self->auth_data + tmp_len, self->auth_data_len - tmp_len);
	cbcmac(self, &tag, src, len);
	memcpy(dst, &tag, self->tag_len);
	return VIAL_AES_ERROR_NONE;
}

static void aes_ccm_set_counter(struct vial_aes *self)
{
	const union vial_aes_block old_iv = self->iv;
	self->pad_rem = 0;
	memset(&self->iv, 0, sizeof(self->iv));
	self->iv.bytes[0] = self->counter_len - 1;
	memcpy(self->iv.bytes + 1, old_iv.bytes + self->counter_len + 1, VIAL_AES_BLOCK_SIZE - 1 - self->counter_len);
}

enum vial_aes_error vial_aes_init(struct vial_aes *self, enum vial_aes_mode mode,
	unsigned keybits, const uint8_t *key, const uint8_t *iv)
{
	self->mode = mode;
	self->rounds = keybits / 32 + 6;
	self->pad_rem = 0;
	self->counter_len = 4;
	self->tag_len = 16;
	self->auth_data_len = 0;
	self->auth_data = NULL;
	memcpy(&self->keys[0], key, keybits / 8);
	if (iv == NULL) {
		if (mode != VIAL_AES_MODE_ECB)
			return VIAL_AES_ERROR_IV;
		memset(&self->iv, 0, VIAL_AES_BLOCK_SIZE);
	} else {
		memcpy(&self->iv, iv, VIAL_AES_BLOCK_SIZE);
	}
	expand_keys(self->keys[0].words, keybits / 32, self->rounds + 1);
	return VIAL_AES_ERROR_NONE;
}

enum vial_aes_error vial_aes_encrypt(struct vial_aes *self, uint8_t *dst, const uint8_t *src, size_t len)
{
	union vial_aes_block blk;
	enum vial_aes_error err;
	if (self->mode == VIAL_AES_MODE_CCM) {
		err = cbcmac_tag(self, dst + len, src, len);
		if (err != VIAL_AES_ERROR_NONE)
			return err;
		blk = self->iv; /* save iv */
		aes_ccm_set_counter(self);
		aes_ctr_pad(self, dst + len, dst + len, self->tag_len); /* first encrypt tag */
		self->pad_rem = 0;
		aes_ctr_pad(self, dst, src, len);
		self->iv = blk; /* restore iv */
		self->pad_rem = 0;
		incr_counter(&self->iv);
	} else if (self->mode == VIAL_AES_MODE_CTR) {
		aes_ctr_pad(self, dst, src, len);
	} else {
		if (len % VIAL_AES_BLOCK_SIZE != 0)
			return VIAL_AES_ERROR_LENGTH;
		while (len > 0) {
			memcpy(&blk, src, VIAL_AES_BLOCK_SIZE);
			if (self->mode == VIAL_AES_MODE_ECB) {
				aes_encrypt(&blk, self->keys, self->rounds);
			} else {
				BXOR(blk, self->iv);
				aes_encrypt(&blk, self->keys, self->rounds);
				self->iv = blk;
			}
			memcpy(dst, &blk, VIAL_AES_BLOCK_SIZE);
			len -= VIAL_AES_BLOCK_SIZE;
			src += VIAL_AES_BLOCK_SIZE;
			dst += VIAL_AES_BLOCK_SIZE;
		}
	}
	return VIAL_AES_ERROR_NONE;
}

enum vial_aes_error vial_aes_decrypt(struct vial_aes *self, uint8_t *dst, const uint8_t *src, size_t len)
{
	union vial_aes_block blk, tag_a, tag_b;
	enum vial_aes_error err;
	if (self->mode == VIAL_AES_MODE_CCM) {
		blk = self->iv; /* save iv */
		aes_ccm_set_counter(self);
		aes_ctr_pad(self, tag_a.bytes, dst + len, self->tag_len); /* first decrypt tag */
		self->pad_rem = 0;
		aes_ctr_pad(self, dst, src, len);
		self->iv = blk; /* restore iv */
		self->pad_rem = 0;
		err = cbcmac_tag(self, tag_b.bytes, dst, len);
		if (err != VIAL_AES_ERROR_NONE)
			return err;
		if (memcmp(&tag_a, &tag_b, self->tag_len))
			return VIAL_AES_ERROR_MAC;
		incr_counter(&self->iv);
	} else if (self->mode == VIAL_AES_MODE_CTR) {
		aes_ctr_pad(self, dst, src, len);
	} else {
		if (len % VIAL_AES_BLOCK_SIZE != 0)
			return VIAL_AES_ERROR_LENGTH;
		while (len > 0) {
			memcpy(&blk, src, VIAL_AES_BLOCK_SIZE);
			if (self->mode == VIAL_AES_MODE_ECB) {
				aes_decrypt(&blk, self->keys, self->rounds);
			} else {
				aes_decrypt(&blk, self->keys, self->rounds);
				BXOR(blk, self->iv);
				memcpy(&self->iv, src, VIAL_AES_BLOCK_SIZE);
			}
			memcpy(dst, &blk, VIAL_AES_BLOCK_SIZE);
			len -= VIAL_AES_BLOCK_SIZE;
			src += VIAL_AES_BLOCK_SIZE;
			dst += VIAL_AES_BLOCK_SIZE;
		}
	}
	return VIAL_AES_ERROR_NONE;
}
