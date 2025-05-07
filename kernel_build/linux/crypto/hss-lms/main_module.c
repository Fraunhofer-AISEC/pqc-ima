/*
* Copyright(c) 2025 Fraunhofer AISEC
* Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2 (GPL 2), as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GPL 2 license for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <crypto/internal/akcipher.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/printk.h>
#include "sha256linux.h"

#include "hss_verify.h"

#define SELFTEST 0

#define STRSTR(a) #a
#define STR(a) STRSTR(a)
#define JOIN(a,b) a ## b
#define JOIN3(a,b,c) a ## b ## c

#define CRYPTO_CTX(cipher) JOIN(cipher,_ctx)
#define CRYPTO_VERIFY(cipher) JOIN(cipher,_verify)
#define CRYPTO_SET_PUB_KEY(cipher) JOIN(cipher,_set_pub_key)
#define CRYPTO_MAX_SIZE(cipher) JOIN(cipher,_max_size)
#define CRYPTO_ALG_INIT(cipher) JOIN(cipher,_alg_init)
#define CRYPTO_ALG_EXIT(cipher) JOIN(cipher,_alg_exit)
#define CRYPTO_AKCIPHER_ALG(cipher) JOIN(cipher,_alg)

#define PQCLEAN_crypto_sign_verify(cipher) JOIN3(PQCLEAN_, cipher, _crypto_sign_verify)
#define PQCLEAN_crypto_sign_keypair(cipher) JOIN3(PQCLEAN_, cipher, _crypto_sign_keypair)
#define PQCLEAN_crypto_sign_signature(cipher) JOIN3(PQCLEAN_, cipher, _crypto_sign_signature)
#define PQCLEAN_CRYPTO_PUBLICKEYBYTES(cipher) JOIN3(PQCLEAN_, cipher, _CRYPTO_PUBLICKEYBYTES)
#define PQCLEAN_CRYPTO_SECRETKEYBYTES(cipher) JOIN3(PQCLEAN_, cipher, _CRYPTO_SECRETKEYBYTES)
#define PQCLEAN_CRYPTO_BYTES(cipher) JOIN3(PQCLEAN_, cipher, _CRYPTO_BYTES)

struct hsslms_ctx {
	uint8_t pk[60];
};

static int hsslms_verify(struct akcipher_request *req)
{
	struct crypto_akcipher *tfm = crypto_akcipher_reqtfm(req);
	struct hsslms_ctx *ctx = akcipher_tfm_ctx(tfm);
	uint8_t *sig;
	size_t siglen = req->src_len;
	uint8_t *m;
	size_t mlen = req->src->length - siglen;
	int ret;

	sig = kmalloc(siglen + mlen, GFP_KERNEL);
	m = sig + siglen;
	if (!sig || !m) {
		pr_warn("kmalloc fail\n");
		return -1;
	}

	sg_copy_to_buffer(req->src, sg_nents(req->src), sig, siglen + mlen);

	//pr_info("req->src_len: %d\n", req->src_len);
	//print_hex_dump(KERN_INFO, "msg: ", DUMP_PREFIX_OFFSET, 16, 1, m, mlen, true);
	//print_hex_dump(KERN_INFO, "sig: ", DUMP_PREFIX_OFFSET, 16, 1, sig, siglen, true);
	//print_hex_dump(KERN_INFO, "pk: ", DUMP_PREFIX_OFFSET, 16, 1, ctx->pk, sizeof(ctx->pk), true);

	ret = !hss_validate_signature(ctx->pk, m, mlen, sig, siglen, NULL);
	if (ret) {
		//pr_warn("PQCLEAN_crypto_sign_verify failed!\n");
		kfree(sig);
		return -1;
	}

	kfree(sig);

	return ret;
}

static int hsslms_set_pub_key(struct crypto_akcipher *tfm,
				       const void *key, unsigned int keylen)
{
	struct hsslms_ctx *ctx = akcipher_tfm_ctx(tfm);

	memset(ctx->pk, 0, sizeof(ctx->pk));

	if (keylen != 60)
		return -EINVAL;

	memcpy(ctx->pk, key, keylen);

	return 0;
}

static unsigned int hsslms_max_size(struct crypto_akcipher *tfm)
{
        struct hsslms_ctx *ctx = akcipher_tfm_ctx(tfm);

        return memchr_inv(ctx->pk, 0, sizeof(ctx->pk)) ? 2964 : 0;
}

static int hsslms_alg_init(struct crypto_akcipher *tfm)
{
        return 0;
}

static void hsslms_alg_exit(struct crypto_akcipher *tfm)
{
        struct hsslms_ctx *ctx = akcipher_tfm_ctx(tfm);

	memset(ctx->pk, 0, sizeof(ctx->pk));
}

static struct akcipher_alg hsslms_alg = {
        .verify = hsslms_verify,
        .set_pub_key = hsslms_set_pub_key,
        .max_size = hsslms_max_size,
        .init = hsslms_alg_init,
        .exit = hsslms_alg_exit,
        .base.cra_name = "hsslms",
        .base.cra_driver_name = "aisec-hsslms",
        .base.cra_ctxsize = sizeof(struct hsslms_ctx),
        .base.cra_module = THIS_MODULE,
        .base.cra_priority = 5000,
};

#if SELFTEST
static int verify_signature(const char *message, size_t message_len, const char *signature, size_t sig_len, const char *public_key) {
	struct crypto_akcipher *tfm;
	struct akcipher_request *req;
	struct scatterlist src;
	int ret;

	tfm = crypto_alloc_akcipher(STR(PQCLEAN_CIPHER_LOWER), 0, 0);
	if (IS_ERR(tfm)) {
		pr_err("Failed to allocate transform\n");
		return PTR_ERR(tfm);
	}

	req = akcipher_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		pr_err("Failed to allocate akcipher request\n");
		ret = -ENOMEM;
		goto free_tfm;
	}

	crypto_akcipher_set_pub_key(tfm, public_key, PQCLEAN_CRYPTO_PUBLICKEYBYTES(PQCLEAN_CIPHER_UPPER));

	uint8_t *data = kmalloc(sig_len + message_len, GFP_KERNEL);

	memcpy(data, signature, sig_len);
	memcpy(data + sig_len, message, message_len);

	sg_init_one(&src, data, sig_len + message_len);
	akcipher_request_set_crypt(req, &src, NULL, sig_len, 0);

	ret = crypto_akcipher_verify(req);
	if (ret) {
		//pr_err("Decryption failed\n");
		goto free_output;
	}

free_output:
	kfree(data);
	akcipher_request_free(req);
free_tfm:
	crypto_free_akcipher(tfm);
	return ret;
}

#define MLEN 32

static int selftest(void)
{
	size_t siglen;
	int ret;

	uint8_t *pk = kmalloc(PQCLEAN_CRYPTO_PUBLICKEYBYTES(PQCLEAN_CIPHER_UPPER), GFP_KERNEL);
    	uint8_t *sk = kmalloc(PQCLEAN_CRYPTO_SECRETKEYBYTES(PQCLEAN_CIPHER_UPPER), GFP_KERNEL);
	uint8_t *sig = kmalloc(PQCLEAN_CRYPTO_BYTES(PQCLEAN_CIPHER_UPPER), GFP_KERNEL);
    	uint8_t *m = kmalloc(MLEN, GFP_KERNEL);

	memset(m, 10, MLEN);

	ret = PQCLEAN_crypto_sign_keypair(PQCLEAN_CIPHER_UPPER)(pk, sk);
	if (ret) {
		pr_warn("PQCLEAN_crypto_sign_keypair failed!\n");
		return -1;
	}

	ret = PQCLEAN_crypto_sign_signature(PQCLEAN_CIPHER_UPPER)(sig, &siglen, m, MLEN, sk);
	if (ret) {
		pr_warn("PQCLEAN_crypto_sign_signature failed!\n");
		return -1;
	}

	ret = verify_signature(m, MLEN, sig, siglen, pk);
	if (ret) {
		pr_warn("Verify failed!\n");
		return -1;
	}

	m[10] += 1;
	ret = verify_signature(m, MLEN, sig, siglen, pk);
	if (!ret) {
		pr_warn("Verify ok. Should not!\n");
		return -1;
	}

	kfree(pk);
	kfree(sk);
	kfree(sig);
	kfree(m);

	return 0;
}
#endif

static int __init crypto_module_init(void)
{
	crypto_register_akcipher(&hsslms_alg);

#if SELFTEST
	if (selftest()) {
		pr_warn("%s: selftest failed\n", THIS_MODULE->name);
	} else {
		pr_warn("%s: selftest ok\n", THIS_MODULE->name);
	}
#endif

	return 0;
}

static void __exit crypto_module_exit(void)
{
	crypto_unregister_akcipher(&hsslms_alg);
}

module_init(crypto_module_init);
module_exit(crypto_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Wiesboeck");
MODULE_DESCRIPTION("hsslms kernel module");
