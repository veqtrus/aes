/*
Copyright (c) 2020 Pavlos Georgiou

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
https://www.boost.org/LICENSE_1_0.txt
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h> 

#include "aes.h"

struct aes_testcase {
	enum vial_aes_mode mode;
	const char *key, *plain, *cipher, *iv;
};

/* SP 800-38A - Recommendation for Block Cipher Modes of Operation: Methods and Techniques */

static const struct aes_testcase aes_testcases[] = {
	{
		VIAL_AES_MODE_ECB,
		"2b7e151628aed2a6abf7158809cf4f3c",
		"6bc1bee22e409f96e93d7e117393172a"
		"ae2d8a571e03ac9c9eb76fac45af8e51"
		"30c81c46a35ce411e5fbc1191a0a52ef"
		"f69f2445df4f9b17ad2b417be66c3710",
		"3ad77bb40d7a3660a89ecaf32466ef97"
		"f5d3d58503b9699de785895a96fdbaaf"
		"43b1cd7f598ece23881b00e3ed030688"
		"7b0c785e27e8ad3f8223207104725dd4"
	}, {
		VIAL_AES_MODE_ECB,
		"8e73b0f7da0e6452c810f32b809079e5"
		"62f8ead2522c6b7b",
		"6bc1bee22e409f96e93d7e117393172a",
		"bd334f1d6e45f25ff712a214571fa5cc"
	}, {
		VIAL_AES_MODE_ECB,
		"603deb1015ca71be2b73aef0857d7781"
		"1f352c073b6108d72d9810a30914dff4",
		"6bc1bee22e409f96e93d7e117393172a",
		"f3eed1bdb5d2a03c064b5a7e3db181f8"
	}, {
		VIAL_AES_MODE_CBC,
		"2b7e151628aed2a6abf7158809cf4f3c",
		"6bc1bee22e409f96e93d7e117393172a"
		"ae2d8a571e03ac9c9eb76fac45af8e51"
		"30c81c46a35ce411e5fbc1191a0a52ef"
		"f69f2445df4f9b17ad2b417be66c3710",
		"7649abac8119b246cee98e9b12e9197d"
		"5086cb9b507219ee95db113a917678b2"
		"73bed6b8e3c1743b7116e69e22229516"
		"3ff1caa1681fac09120eca307586e1a7",
		"000102030405060708090a0b0c0d0e0f"
	}, {
		VIAL_AES_MODE_CTR,
		"2b7e151628aed2a6abf7158809cf4f3c",
		"6bc1bee22e409f96e93d7e117393172a"
		"ae2d8a571e03ac9c9eb76fac45af8e51"
		"30c81c46a35ce411e5fbc1191a0a52ef"
		"f69f2445df4f9b17ad2b417be66c3710",
		"874d6191b620e3261bef6864990db6ce"
		"9806f66b7970fdff8617187bb9fffdff"
		"5ae4df3edbd5d35e5b4f09020db03eab"
		"1e031dda2fbe03d1792170a0f3009cee",
		"f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
	}, { 0 }
};

struct cmac_testcase {
	const char *key, *msg, *tag;
};

/* SP 800-38B - Recommendation for Block Cipher Modes of Operation: The CMAC Mode for Authentication */

static const struct cmac_testcase cmac_testcases[] = {
	{
		"2B7E151628AED2A6ABF7158809CF4F3C",
		"",
		"BB1D6929E95937287FA37D129B756746"
	}, {
		"2B7E151628AED2A6ABF7158809CF4F3C",
		"6BC1BEE22E409F96E93D7E117393172A",
		"070A16B46B4D4144F79BDD9DD04A287C"
	}, {
		"2B7E151628AED2A6ABF7158809CF4F3C",
		"6BC1BEE22E409F96E93D7E117393172A"
		"AE2D8A57",
		"7D85449EA6EA19C823A7BF78837DFADE"
	}, {
		"2B7E151628AED2A6ABF7158809CF4F3C",
		"6BC1BEE22E409F96E93D7E117393172A"
		"AE2D8A571E03AC9C9EB76FAC45AF8E51"
		"30C81C46A35CE411E5FBC1191A0A52EF"
		"F69F2445DF4F9B17AD2B417BE66C3710",
		"51F0BEBF7E3B9D92FC49741779363CFE"
	}, { 0 }
};

static uint8_t *decode_hex(const char *src)
{
	if (src == NULL)
		return NULL;
	const size_t len = strlen(src);
	uint8_t *result = malloc(len / 2);
	unsigned digit;
	for (size_t i = 0; i < len; ++i) {
		digit = src[i];
		if (digit >= '0' && digit <= '9') {
			digit -= '0';
		} else if (digit >= 'A' && digit <= 'F') {
			digit -= 'A' - 10;
		} else if (digit >= 'a' && digit <= 'f') {
			digit -= 'a' - 10;
		} else {
			free(result);
			return NULL;
		}
		if (i % 2 == 0)
			result[i / 2] = digit * 16;
		else
			result[i / 2] += digit;
	}
	return result;
}

static void print_hex(const uint8_t *src, size_t len)
{
	const uint8_t *end = src + len;
	for (; src < end; ++src)
		printf("%02X", *src);
}

static int test_aes(const struct aes_testcase *test)
{	struct vial_aes_key aes_key;
	struct vial_aes aes;
	const char *mode;
	uint8_t *key, *plain, *cipher, *iv, *result;
	const size_t key_size = strlen(test->key) / 2,
		plain_size = strlen(test->plain) / 2,
		cipher_size = strlen(test->cipher) / 2;
	int code = 0;
	key = decode_hex(test->key);
	plain = decode_hex(test->plain);
	cipher = decode_hex(test->cipher);
	iv = decode_hex(test->iv);
	result = malloc(cipher_size > plain_size ? cipher_size : plain_size);
	switch (test->mode) {
	case VIAL_AES_MODE_ECB:
		mode = "ECB"; break;
	case VIAL_AES_MODE_CBC:
		mode = "CBC"; break;
	case VIAL_AES_MODE_CTR:
		mode = "CTR"; break;
	default:
		code = 31;
		goto exit;
	}
	if (key == NULL || plain == NULL || cipher == NULL) {
		puts("Failed decoding test case");
		code = 31;
		goto exit;
	}
	vial_aes_key_init(&aes_key, key_size * 8, key);
	vial_aes_init(&aes, test->mode, &aes_key, iv);
	code = vial_aes_encrypt(&aes, result, plain, plain_size);
	if (code) goto exit;
	if (memcmp(cipher, result, cipher_size)) {
		printf("AES %s failed encrypting %s\n", mode, test->plain);
		printf("  got ");
		print_hex(result, cipher_size);
		printf("\n  exp %s\n", test->cipher);
		code = 32;
		goto exit;
	}
	vial_aes_init(&aes, test->mode, &aes_key, iv);
	code = vial_aes_decrypt(&aes, result, cipher, plain_size);
	if (code) goto exit;
	if (memcmp(plain, result, plain_size)) {
		printf("AES %s failed decrypting %s\n", mode, test->cipher);
		printf("  got ");
		print_hex(result, cipher_size);
		printf("\n  exp %s\n", test->plain);
		code = 33;
		goto exit;
	}
exit:
	free(key);
	free(plain);
	free(cipher);
	free(iv);
	free(result);
	return code;
}

static int test_cmac(const struct cmac_testcase *test)
{
	struct vial_aes_key aes_key;
	struct vial_aes_cmac cmac;
	const size_t key_size = strlen(test->key) / 2,
		msg_size = strlen(test->msg) / 2,
		tag_size = strlen(test->tag) / 2;
	uint8_t *key, *msg, *tag, *result;
	int code = 0;
	key = decode_hex(test->key);
	msg = decode_hex(test->msg);
	tag = decode_hex(test->tag);
	result = malloc(tag_size);
	if (key == NULL || msg == NULL || tag == NULL) {
		puts("Failed decoding test case");
		code = 31;
		goto exit;
	}
	vial_aes_key_init(&aes_key, key_size * 8, key);
	vial_aes_cmac_init(&cmac, &aes_key);
	vial_aes_cmac_update(&cmac, msg, msg_size);
	vial_aes_cmac_finish(&cmac, result, tag_size);
	if (memcmp(tag, result, tag_size)) {
		printf("AES CMAC failed on message %s\n", test->msg);
		code = 32;
		goto exit;
	}
exit:
	free(key);
	free(msg);
	free(tag);
	free(result);
	return code;
}

int main()
{
	int err;
	for (const struct aes_testcase *test = aes_testcases; test->key; ++test) {
		err = test_aes(test);
		if (err) return err;
	}
	puts("AES encryption/decryption OK");
	for (const struct cmac_testcase *test = cmac_testcases; test->key; ++test) {
		err = test_cmac(test);
		if (err) return err;
	}
	puts("AES CMAC OK");
	return 0;
}
