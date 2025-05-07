#include <linux/printk.h>
#include <linux/slab.h>
#include <crypto/hash.h>

typedef struct {
	struct shash_desc *desc;
} SHA256_CTX;

static inline void SHA256_Init(SHA256_CTX *ctx)
{
	struct crypto_shash *tfm;
	struct shash_desc *desc;
	size_t digest_size, desc_size;

	tfm = crypto_alloc_shash("sha256", 0, 0);
	desc_size = crypto_shash_descsize(tfm) + sizeof(*desc);
	digest_size = crypto_shash_digestsize(tfm);

	desc = kzalloc(desc_size + digest_size, GFP_KERNEL);
	desc->tfm = tfm;

	crypto_shash_init(desc);

	ctx->desc = desc;
}

static inline void SHA256_Update(SHA256_CTX *ctx, const void *data, unsigned int len)
{
	crypto_shash_update(ctx->desc, data, len);
}

static inline void SHA256_Final(unsigned char *digest, SHA256_CTX *ctx)
{
	struct crypto_shash *tfm = ctx->desc->tfm;

	crypto_shash_final(ctx->desc, digest);

	kfree_sensitive(ctx->desc);
	crypto_free_shash(tfm);
}
