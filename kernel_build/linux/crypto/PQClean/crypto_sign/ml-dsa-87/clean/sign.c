#include "fips202.h"
#include "packing.h"
#include "params.h"
#include "poly.h"
#include "polyvec.h"
#include "randombytes.h"
#include "sign.h"
#include "symmetric.h"
#include <linux/types.h>
#include <linux/slab.h>

/*************************************************
* Name:        PQCLEAN_MLDSA87_CLEAN_crypto_sign_keypair
*
* Description: Generates public and private key.
*
* Arguments:   - uint8_t *pk: pointer to output public key (allocated
*                             array of PQCLEAN_MLDSA87_CLEAN_CRYPTO_PUBLICKEYBYTES bytes)
*              - uint8_t *sk: pointer to output private key (allocated
*                             array of PQCLEAN_MLDSA87_CLEAN_CRYPTO_SECRETKEYBYTES bytes)
*
* Returns 0 (success)
**************************************************/
int PQCLEAN_MLDSA87_CLEAN_crypto_sign_keypair(uint8_t *pk, uint8_t *sk) {
    uint8_t seedbuf[2 * SEEDBYTES + CRHBYTES];
    uint8_t tr[TRBYTES];
    const uint8_t *rho, *rhoprime, *key;
    struct {
        polyvecl mat[K];
        polyvecl s1, s1hat;
        polyveck s2, t1, t0;
    } *tmp;

    tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);

    /* Get randomness for rho, rhoprime and key */
    randombytes(seedbuf, SEEDBYTES);
    seedbuf[SEEDBYTES + 0] = K;
    seedbuf[SEEDBYTES + 1] = L;
    shake256(seedbuf, 2 * SEEDBYTES + CRHBYTES, seedbuf, SEEDBYTES + 2);
    rho = seedbuf;
    rhoprime = rho + SEEDBYTES;
    key = rhoprime + CRHBYTES;

    /* Expand matrix */
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_expand(tmp->mat, rho);

    /* Sample short vectors s1 and s2 */
    PQCLEAN_MLDSA87_CLEAN_polyvecl_uniform_eta(&tmp->s1, rhoprime, 0);
    PQCLEAN_MLDSA87_CLEAN_polyveck_uniform_eta(&tmp->s2, rhoprime, L);

    /* Matrix-vector multiplication */
    tmp->s1hat = tmp->s1;
    PQCLEAN_MLDSA87_CLEAN_polyvecl_ntt(&tmp->s1hat);
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_pointwise_montgomery(&tmp->t1, tmp->mat, &tmp->s1hat);
    PQCLEAN_MLDSA87_CLEAN_polyveck_reduce(&tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_invntt_tomont(&tmp->t1);

    /* Add error vector s2 */
    PQCLEAN_MLDSA87_CLEAN_polyveck_add(&tmp->t1, &tmp->t1, &tmp->s2);

    /* Extract t1 and write public key */
    PQCLEAN_MLDSA87_CLEAN_polyveck_caddq(&tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_power2round(&tmp->t1, &tmp->t0, &tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_pack_pk(pk, rho, &tmp->t1);

    /* Compute H(rho, t1) and write secret key */
    shake256(tr, TRBYTES, pk, PQCLEAN_MLDSA87_CLEAN_CRYPTO_PUBLICKEYBYTES);
    PQCLEAN_MLDSA87_CLEAN_pack_sk(sk, rho, tr, key, &tmp->t0, &tmp->s1, &tmp->s2);

    kfree(tmp);

    return 0;
}

/*************************************************
* Name:        crypto_sign_signature
*
* Description: Computes signature.
*
* Arguments:   - uint8_t *sig:   pointer to output signature (of length PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES)
*              - size_t *siglen: pointer to output length of signature
*              - uint8_t *m:     pointer to message to be signed
*              - size_t mlen:    length of message
*              - uint8_t *ctx:   pointer to context string
*              - size_t ctxlen:  length of context string
*              - uint8_t *sk:    pointer to bit-packed secret key
*
* Returns 0 (success) or -1 (context string too long)
**************************************************/
int PQCLEAN_MLDSA87_CLEAN_crypto_sign_signature_ctx(uint8_t *sig,
        size_t *siglen,
        const uint8_t *m,
        size_t mlen,
        const uint8_t *ctx,
        size_t ctxlen,
        const uint8_t *sk) {
    unsigned int n;
    uint8_t seedbuf[2 * SEEDBYTES + TRBYTES + RNDBYTES + 2 * CRHBYTES];
    uint8_t *rho, *tr, *key, *mu, *rhoprime, *rnd;
    uint16_t nonce = 0;
    struct {
        polyvecl mat[K], s1, y, z;
        polyveck t0, s2, w1, w0, h;
        poly cp;
        shake256incctx state;
    } *tmp;

    if (ctxlen > 255) {
        return -1;
    }

    tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);

    rho = seedbuf;
    tr = rho + SEEDBYTES;
    key = tr + TRBYTES;
    rnd = key + SEEDBYTES;
    mu = rnd + RNDBYTES;
    rhoprime = mu + CRHBYTES;
    PQCLEAN_MLDSA87_CLEAN_unpack_sk(rho, tr, key, &tmp->t0, &tmp->s1, &tmp->s2, sk);

    /* Compute mu = CRH(tr, 0, ctxlen, ctx, msg) */
    mu[0] = 0;
    mu[1] = (uint8_t)ctxlen;
    shake256_inc_init(&tmp->state);
    shake256_inc_absorb(&tmp->state, tr, TRBYTES);
    shake256_inc_absorb(&tmp->state, mu, 2);
    shake256_inc_absorb(&tmp->state, ctx, ctxlen);
    shake256_inc_absorb(&tmp->state, m, mlen);
    shake256_inc_finalize(&tmp->state);
    shake256_inc_squeeze(mu, CRHBYTES, &tmp->state);
    shake256_inc_ctx_release(&tmp->state);

    randombytes(rnd, RNDBYTES);
    shake256(rhoprime, CRHBYTES, key, SEEDBYTES + RNDBYTES + CRHBYTES);

    /* Expand matrix and transform vectors */
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_expand(tmp->mat, rho);
    PQCLEAN_MLDSA87_CLEAN_polyvecl_ntt(&tmp->s1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_ntt(&tmp->s2);
    PQCLEAN_MLDSA87_CLEAN_polyveck_ntt(&tmp->t0);

rej:
    /* Sample intermediate vector y */
    PQCLEAN_MLDSA87_CLEAN_polyvecl_uniform_gamma1(&tmp->y, rhoprime, nonce++);

    /* Matrix-vector multiplication */
    tmp->z = tmp->y;
    PQCLEAN_MLDSA87_CLEAN_polyvecl_ntt(&tmp->z);
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_pointwise_montgomery(&tmp->w1, tmp->mat, &tmp->z);
    PQCLEAN_MLDSA87_CLEAN_polyveck_reduce(&tmp->w1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_invntt_tomont(&tmp->w1);

    /* Decompose w and call the random oracle */
    PQCLEAN_MLDSA87_CLEAN_polyveck_caddq(&tmp->w1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_decompose(&tmp->w1, &tmp->w0, &tmp->w1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_pack_w1(sig, &tmp->w1);

    shake256_inc_init(&tmp->state);
    shake256_inc_absorb(&tmp->state, mu, CRHBYTES);
    shake256_inc_absorb(&tmp->state, sig, K * POLYW1_PACKEDBYTES);
    shake256_inc_finalize(&tmp->state);
    shake256_inc_squeeze(sig, CTILDEBYTES, &tmp->state);
    shake256_inc_ctx_release(&tmp->state);
    PQCLEAN_MLDSA87_CLEAN_poly_challenge(&tmp->cp, sig);
    PQCLEAN_MLDSA87_CLEAN_poly_ntt(&tmp->cp);

    /* Compute z, reject if it reveals secret */
    PQCLEAN_MLDSA87_CLEAN_polyvecl_pointwise_poly_montgomery(&tmp->z, &tmp->cp, &tmp->s1);
    PQCLEAN_MLDSA87_CLEAN_polyvecl_invntt_tomont(&tmp->z);
    PQCLEAN_MLDSA87_CLEAN_polyvecl_add(&tmp->z, &tmp->z, &tmp->y);
    PQCLEAN_MLDSA87_CLEAN_polyvecl_reduce(&tmp->z);
    if (PQCLEAN_MLDSA87_CLEAN_polyvecl_chknorm(&tmp->z, GAMMA1 - BETA)) {
        goto rej;
    }

    /* Check that subtracting cs2 does not change high bits of w and low bits
     * do not reveal secret information */
    PQCLEAN_MLDSA87_CLEAN_polyveck_pointwise_poly_montgomery(&tmp->h, &tmp->cp, &tmp->s2);
    PQCLEAN_MLDSA87_CLEAN_polyveck_invntt_tomont(&tmp->h);
    PQCLEAN_MLDSA87_CLEAN_polyveck_sub(&tmp->w0, &tmp->w0, &tmp->h);
    PQCLEAN_MLDSA87_CLEAN_polyveck_reduce(&tmp->w0);
    if (PQCLEAN_MLDSA87_CLEAN_polyveck_chknorm(&tmp->w0, GAMMA2 - BETA)) {
        goto rej;
    }

    /* Compute hints for w1 */
    PQCLEAN_MLDSA87_CLEAN_polyveck_pointwise_poly_montgomery(&tmp->h, &tmp->cp, &tmp->t0);
    PQCLEAN_MLDSA87_CLEAN_polyveck_invntt_tomont(&tmp->h);
    PQCLEAN_MLDSA87_CLEAN_polyveck_reduce(&tmp->h);
    if (PQCLEAN_MLDSA87_CLEAN_polyveck_chknorm(&tmp->h, GAMMA2)) {
        goto rej;
    }

    PQCLEAN_MLDSA87_CLEAN_polyveck_add(&tmp->w0, &tmp->w0, &tmp->h);
    n = PQCLEAN_MLDSA87_CLEAN_polyveck_make_hint(&tmp->h, &tmp->w0, &tmp->w1);
    if (n > OMEGA) {
        goto rej;
    }

    /* Write signature */
    PQCLEAN_MLDSA87_CLEAN_pack_sig(sig, sig, &tmp->z, &tmp->h);
    *siglen = PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES;

    kfree(tmp);

    return 0;
}

/*************************************************
* Name:        crypto_sign
*
* Description: Compute signed message.
*
* Arguments:   - uint8_t *sm: pointer to output signed message (allocated
*                             array with PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES + mlen bytes),
*                             can be equal to m
*              - size_t *smlen: pointer to output length of signed
*                               message
*              - const uint8_t *m: pointer to message to be signed
*              - size_t mlen: length of message
*              - const uint8_t *ctx: pointer to context string
*              - size_t ctxlen: length of context string
*              - const uint8_t *sk: pointer to bit-packed secret key
*
* Returns 0 (success) or -1 (context string too long)
**************************************************/
int PQCLEAN_MLDSA87_CLEAN_crypto_sign_ctx(uint8_t *sm,
        size_t *smlen,
        const uint8_t *m,
        size_t mlen,
        const uint8_t *ctx,
        size_t ctxlen,
        const uint8_t *sk) {
    int ret;
    size_t i;

    for (i = 0; i < mlen; ++i) {
        sm[PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES + mlen - 1 - i] = m[mlen - 1 - i];
    }
    ret = PQCLEAN_MLDSA87_CLEAN_crypto_sign_signature_ctx(sm, smlen, sm + PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES, mlen, ctx, ctxlen, sk);
    *smlen += mlen;
    return ret;
}

/*************************************************
* Name:        crypto_sign_verify
*
* Description: Verifies signature.
*
* Arguments:   - uint8_t *m: pointer to input signature
*              - size_t siglen: length of signature
*              - const uint8_t *m: pointer to message
*              - size_t mlen: length of message
*              - const uint8_t *ctx: pointer to context string
*              - size_t ctxlen: length of context string
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signature could be verified correctly and -1 otherwise
**************************************************/
int PQCLEAN_MLDSA87_CLEAN_crypto_sign_verify_ctx(const uint8_t *sig,
        size_t siglen,
        const uint8_t *m,
        size_t mlen,
        const uint8_t *ctx,
        size_t ctxlen,
        const uint8_t *pk) {
    unsigned int i;
    struct {
        uint8_t buf[K * POLYW1_PACKEDBYTES];
        uint8_t rho[SEEDBYTES];
        uint8_t mu[CRHBYTES];
        uint8_t c[CTILDEBYTES];
        uint8_t c2[CTILDEBYTES];
        poly cp;
        polyvecl mat[K], z;
        polyveck t1, w1, h;
    } *tmp;
    shake256incctx state;

    tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);

    if (ctxlen > 255 || siglen != PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES) {
        goto fail;
    }

    PQCLEAN_MLDSA87_CLEAN_unpack_pk(tmp->rho, &tmp->t1, pk);
    if (PQCLEAN_MLDSA87_CLEAN_unpack_sig(tmp->c, &tmp->z, &tmp->h, sig)) {
        goto fail;
    }
    if (PQCLEAN_MLDSA87_CLEAN_polyvecl_chknorm(&tmp->z, GAMMA1 - BETA)) {
        goto fail;
    }

    /* Compute CRH(H(rho, t1), msg) */
    shake256(tmp->mu, TRBYTES, pk, PQCLEAN_MLDSA87_CLEAN_CRYPTO_PUBLICKEYBYTES);
    shake256_inc_init(&state);
    shake256_inc_absorb(&state, tmp->mu, TRBYTES);
    tmp->mu[0] = 0;
    tmp->mu[1] = (uint8_t)ctxlen;
    shake256_inc_absorb(&state, tmp->mu, 2);
    shake256_inc_absorb(&state, ctx, ctxlen);
    shake256_inc_absorb(&state, m, mlen);
    shake256_inc_finalize(&state);
    shake256_inc_squeeze(tmp->mu, CRHBYTES, &state);
    shake256_inc_ctx_release(&state);

    /* Matrix-vector multiplication; compute Az - c2^dt1 */
    PQCLEAN_MLDSA87_CLEAN_poly_challenge(&tmp->cp, tmp->c);
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_expand(tmp->mat, tmp->rho);

    PQCLEAN_MLDSA87_CLEAN_polyvecl_ntt(&tmp->z);
    PQCLEAN_MLDSA87_CLEAN_polyvec_matrix_pointwise_montgomery(&tmp->w1, tmp->mat, &tmp->z);

    PQCLEAN_MLDSA87_CLEAN_poly_ntt(&tmp->cp);
    PQCLEAN_MLDSA87_CLEAN_polyveck_shiftl(&tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_ntt(&tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_pointwise_poly_montgomery(&tmp->t1, &tmp->cp, &tmp->t1);

    PQCLEAN_MLDSA87_CLEAN_polyveck_sub(&tmp->w1, &tmp->w1, &tmp->t1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_reduce(&tmp->w1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_invntt_tomont(&tmp->w1);

    /* Reconstruct w1 */
    PQCLEAN_MLDSA87_CLEAN_polyveck_caddq(&tmp->w1);
    PQCLEAN_MLDSA87_CLEAN_polyveck_use_hint(&tmp->w1, &tmp->w1, &tmp->h);
    PQCLEAN_MLDSA87_CLEAN_polyveck_pack_w1(tmp->buf, &tmp->w1);

    /* Call random oracle and verify challenge */
    shake256_inc_init(&state);
    shake256_inc_absorb(&state, tmp->mu, CRHBYTES);
    shake256_inc_absorb(&state, tmp->buf, K * POLYW1_PACKEDBYTES);
    shake256_inc_finalize(&state);
    shake256_inc_squeeze(tmp->c2, CTILDEBYTES, &state);
    shake256_inc_ctx_release(&state);
    for (i = 0; i < CTILDEBYTES; ++i) {
        if (tmp->c[i] != tmp->c2[i]) {
            goto fail;
        }
    }
    kfree(tmp);
    return 0;
fail:
    kfree(tmp);
    return -1;
}

/*************************************************
* Name:        crypto_sign_open
*
* Description: Verify signed message.
*
* Arguments:   - uint8_t *m: pointer to output message (allocated
*                            array with smlen bytes), can be equal to sm
*              - size_t *mlen: pointer to output length of message
*              - const uint8_t *sm: pointer to signed message
*              - size_t smlen: length of signed message
*              - const uint8_t *ctx: pointer to context tring
*              - size_t ctxlen: length of context string
*              - const uint8_t *pk: pointer to bit-packed public key
*
* Returns 0 if signed message could be verified correctly and -1 otherwise
**************************************************/
int PQCLEAN_MLDSA87_CLEAN_crypto_sign_open_ctx(uint8_t *m,
        size_t *mlen,
        const uint8_t *sm,
        size_t smlen,
        const uint8_t *ctx,
        size_t ctxlen,
        const uint8_t *pk) {
    size_t i;

    if (smlen < PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES) {
        goto badsig;
    }

    *mlen = smlen - PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES;
    if (PQCLEAN_MLDSA87_CLEAN_crypto_sign_verify_ctx(sm, PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES, sm + PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES, *mlen, ctx, ctxlen, pk)) {
        goto badsig;
    } else {
        /* All good, copy msg, return 0 */
        for (i = 0; i < *mlen; ++i) {
            m[i] = sm[PQCLEAN_MLDSA87_CLEAN_CRYPTO_BYTES + i];
        }
        return 0;
    }

badsig:
    /* Signature verification failed */
    *mlen = 0;
    for (i = 0; i < smlen; ++i) {
        m[i] = 0;
    }

    return -1;
}
