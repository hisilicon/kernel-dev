/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/crypto.h>
#include <crypto/internal/aead.h>
#include <crypto/des.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/authenc.h>
#include <linux/dma-mapping.h>

//#include "../wd/wd.h"

#include "sec_main.h"
#include "sec_algs.h"


static void sec_free_opdata(struct hisi_acc_qp *queue,
			struct sec_crypto_request *sec_req);
static DEFINE_MUTEX(algs_lock);
static unsigned int active_devs;


#ifdef SEC_DEBUG
void Test_sec_print_bd(void *bd_ptr)
{
	struct sec_bd_info *bd = (struct sec_bd_info *)bd_ptr;

	pr_info("******* word 0 *******\r\n");
	pr_info("bd_type:0x%x\r\n", bd->bd_type);
	pr_info("cipher:0x%x\r\n", bd->cipher);
	pr_info("auth:0x%x\r\n", bd->auth);
	pr_info("seq:0x%x\r\n", bd->seq);
	pr_info("de:0x%x\r\n", bd->de);
	pr_info("scene:0x%x\r\n", bd->scene);
	pr_info("src_addr_type:0x%x\r\n", bd->src_addr_type);
	pr_info("dst_addr_type:0x%x\r\n", bd->dst_addr_type);
	pr_info("rsv_w0:0x%x\r\n", bd->rsv_w0);

	pr_info("******* word 1 *******\r\n");
	pr_info("nonce_len:0x%x\r\n", bd->nonce_len);
	pr_info("huk:0x%x\r\n", bd->huk);
	pr_info("key_s:0x%x\r\n", bd->key_s);
	pr_info("ci_gen:0x%x\r\n", bd->ci_gen);
	pr_info("ai_gen:0x%x\r\n", bd->ai_gen);
	pr_info("auth_pad:0x%x\r\n", bd->auth_pad);
	pr_info("c_s:0x%x\r\n", bd->c_s);
	pr_info("rsv1_w1:0x%x\r\n", bd->rsv1_w1);
	pr_info("rhf:0x%x\r\n", bd->rhf);
	pr_info("cipher_key_type:0x%x\r\n", bd->cipher_key_type);
	pr_info("auth_key_type:0x%x\r\n", bd->auth_key_type);
	pr_info("write_frame_len:0x%x\r\n", bd->write_frame_len);
	pr_info("rsv2_w1:0x%x\r\n", bd->rsv2_w1);
	pr_info("bd_invalid:0x%x\r\n", bd->bd_invalid);

	pr_info("******* word 2 *******\r\n");
	pr_info("mac_len:0x%x\r\n", bd->mac_len);
	pr_info("akey_len:0x%x\r\n", bd->akey_len);
	pr_info("a_alg:0x%x\r\n", bd->a_alg);
	pr_info("rsv_w2:0x%x\r\n", bd->rsv_w2);

	pr_info("******* word 3 *******\r\n");
	pr_info("c_icv_len:0x%x\r\n", bd->c_icv_len);
	pr_info("c_width:0x%x\r\n", bd->c_width);
	pr_info("c_key_len:0x%x\r\n", bd->c_key_len);
	pr_info("c_mode:0x%x\r\n", bd->c_mode);
	pr_info("c_alg:0x%x\r\n", bd->c_alg);
	pr_info("rsv_w3:0x%x\r\n", bd->rsv_w3);

	pr_info("******* word 4 *******\r\n");
	pr_info("auth_len:0x%x\r\n", bd->auth_len);
	pr_info("rsv_w4:0x%x\r\n", bd->rsv_w4);

	pr_info("******* word 5 *******\r\n");
	pr_info("cipher_len:0x%x\r\n", bd->cipher_len);
	pr_info("rsv_w5:0x%x\r\n", bd->rsv_w5);

	pr_info("******* word 6 *******\r\n");
	pr_info("auth_src_offset:0x%x\r\n", bd->auth_src_offset);
	pr_info("cipher_src_offset:0x%x\r\n", bd->cipher_src_offset);

	pr_info("******* word 7 *******\r\n");
	pr_info("cipher_len:0x%x\r\n", bd->cipher_len);
	pr_info("rsv_w5:0x%x\r\n", bd->rsv_w5);

	pr_info("******* word 8 *******\r\n");
	pr_info("pass_word_len:0x%x\r\n", bd->pass_word_len);
	pr_info("dk_len:0x%x\r\n", bd->dk_len);

	pr_info("******* word 9 *******\r\n");
	pr_info("salt:0x%x\r\n", bd->salt);

	pr_info("******* word 10 *******\r\n");
	pr_info("cipher_pad_type:0x%x\r\n", bd->cipher_pad_type);
	pr_info("cipher_pad_len:0x%x\r\n", bd->cipher_pad_len);
	pr_info("cipher_pad_data_type:0x%x\r\n", bd->cipher_pad_data_type);
	pr_info("cipher_pad_len_field:0x%x\r\n", bd->cipher_pad_len_field);
	pr_info("rsv_w10:0x%x\r\n", bd->rsv_w10);

	pr_info("******* word 11 *******\r\n");
	pr_info("tag:0x%x\r\n", bd->tag);
	pr_info("rsv_w11:0x%x\r\n", bd->rsv_w11);

	pr_info("******* word 12 13 14 ... 27 ******\r\n");
	pr_info("long_auth_data_len:0x%x\r\n", bd->long_auth_data_len);
	pr_info("long_auth_data_len_hi:0x%x\r\n", bd->long_auth_data_len_hi);
	pr_info("auth_key_addr:0x%x\r\n", bd->auth_key_addr);
	pr_info("auth_key_addr_hi:0x%x\r\n", bd->auth_key_addr_hi);
	pr_info("auth_ivin_addr:0x%x\r\n", bd->auth_ivin_addr);
	pr_info("auth_iviv_addr_hi:0x%x\r\n", bd->auth_iviv_addr_hi);
	pr_info("mac_addr:0x%x\r\n", bd->mac_addr);
	pr_info("mac_addr_hi:0x%x\r\n", bd->mac_addr_hi);
	pr_info("cipher_key_addr:0x%x\r\n", bd->cipher_key_addr);
	pr_info("cipher_key_addr_hi:0x%x\r\n", bd->cipher_key_addr_hi);
	pr_info("cipher_iv_addr:0x%x\r\n", bd->cipher_iv_addr);
	pr_info("cipher_iv_addr_hi:0x%x\r\n", bd->cipher_iv_addr_hi);
	pr_info("data_src_addr:0x%x\r\n", bd->data_src_addr);
	pr_info("data_src_addr_hi:0x%x\r\n", bd->data_src_addr_hi);
	pr_info("data_dst_addr:0x%x\r\n", bd->data_dst_addr);
	pr_info("data_dst_addr_hi:0x%x\r\n", bd->data_dst_addr_hi);

	pr_info("******* word 28 *******\r\n");
	pr_info("done:0x%x\r\n", bd->done);
	pr_info("icv:0x%x\r\n", bd->icv);
	pr_info("csc:0x%x\r\n", bd->csc);
	pr_info("flag:0x%x\r\n", bd->flag);
	pr_info("rsv_w28:0x%x\r\n", bd->rsv_w28);
	pr_info("error_type:0x%x\r\n", bd->error_type);
	pr_info("warning_type:0x%x\r\n", bd->warning_type);

	pr_info("******* word 29 *******\r\n");
	pr_info("mac_i:0x%x\r\n", bd->mac_i);

	pr_info("******* word 30*******\r\n");
	pr_info("checksum_i:0x%x\r\n", bd->checksum_i);

	pr_info("******* word 31 *******\r\n");
	pr_info("counter:0x%x\r\n", bd->counter);

	pr_info("******* sqe data end *******\r\n");
}

static void _sec_show_sgl(struct sec_hw_sgl *sgl)
{
	int i  = 0;

	pr_info("sgl->next_sgl = %llx \r\n", sgl->next_sgl);
	pr_info("sgl->entry_sum_in_chain = %d \r\n", sgl->entry_sum_in_chain);
	pr_info("sgl->entry_sum_in_sgl = %d \r\n", sgl->entry_sum_in_sgl);
	pr_info("sgl->flag = %d \r\n", sgl->flag);
	pr_info("sgl->serial_num = %lld \r\n", sgl->serial_num);
	pr_info("sgl->cpuid = %d \r\n", sgl->cpuid);
	pr_info("sgl->data_bytes_in_sgl = %d \r\n",
		sgl->data_bytes_in_sgl);
	while (i < sgl->entry_sum_in_sgl) {
		pr_info("sgl->sge_entrys[%d].buf=%llx \r\n",
			i, sgl->sge_entrys[i].buf);
		pr_info("sgl->sge_entrys[%d].len=%d \r\n",
			i, sgl->sge_entrys[i].len);
		pr_info("sgl->sge_entrys[%d].pad=%d \r\n",
			i, sgl->sge_entrys[i].pad);
		i++;
	}
}
#endif

static void sec_ablkcipher_alg_callback(struct sec_bd_info *sec_resp,
				  struct sec_crypto_request *sec_req)
{
	struct sec_alg_ablkcipher_ctx *ctx = sec_req->ablkcipher_ctx;
	struct hisi_acc_qp *queue = ctx->queue;
	struct ablkcipher_request *areq = sec_req->ablkcipher_req;
	int ret = 0;

	if (unlikely(sec_resp->bd_invalid == 1 ||
		sec_resp->icv == 3))
		ret = -EINVAL;

#ifdef SEC_DEBUG
	Test_sec_print_bd(sec_resp);
#endif
	sec_free_opdata(queue, sec_req);
	areq->base.complete(&areq->base, ret);
}
void sec_alg_handler(void *resp)
{
	struct sec_bd_info *sec_resp = resp;
	struct sec_crypto_request *sec_req =
			*(struct sec_crypto_request **)&sec_resp->mac_addr;
	if (sec_req)
		sec_req->cb(sec_resp, sec_req);
}

static int sec_alg_info_trans(int key_len, int *sec_alg, const char *alg)
{
	if (!strncmp(alg, "cbc", 3)) {
		if (!strncmp(alg + 4, "aes", 3)) {
			if (key_len == AES_KEYSIZE_128)
				*sec_alg = SEC_AES_CBC_128;
			else if (key_len == AES_KEYSIZE_192)
				*sec_alg = SEC_AES_CBC_192;
			else if (key_len == AES_KEYSIZE_256)
				*sec_alg = SEC_AES_CBC_256;
			else
				return -EINVAL;
			return 0;
		}
		if (!strncmp(alg + 4, "des3_ede", 8)) {
			if (key_len == DES3_EDE_KEY_SIZE)
				*sec_alg = SEC_3DES_CBC_192_3KEY;
			else
				return -EINVAL;
			return 0;
		}
		if (!strncmp(alg + 4, "des", 3)) {
			if (key_len == DES_KEY_SIZE)
				*sec_alg = SEC_DES_CBC_64;
			else
				return -EINVAL;
			return 0;
		}
	} else if (!strncmp(alg, "ecb", 3)) {
		if (!strncmp(alg + 4, "aes", 3)) {
			if (key_len == AES_KEYSIZE_128)
				*sec_alg = SEC_AES_ECB_128;
			else if (key_len == AES_KEYSIZE_192)
				*sec_alg = SEC_AES_ECB_192;
			else if (key_len == AES_KEYSIZE_256)
				*sec_alg = SEC_AES_ECB_256;
			else
				return -EINVAL;
			return 0;
		}
		if (!strncmp(alg + 4, "des3_ede", 8)) {
			if (key_len == DES3_EDE_KEY_SIZE)
				*sec_alg = SEC_3DES_ECB_192_3KEY;
			else
				return -EINVAL;
			return 0;
		}
		if (!strncmp(alg + 4, "des", 3)) {
			if (key_len == DES_KEY_SIZE)
				*sec_alg = SEC_DES_ECB_64;
			else
				return -EINVAL;
			return 0;
		}
	} else if (!strncmp(alg, "ctr", 3)) {
		if (!strncmp(alg + 4, "aes", 3)) {
			if (key_len == AES_KEYSIZE_128)
				*sec_alg = SEC_AES_CTR_128;
			else if (key_len == AES_KEYSIZE_192)
				*sec_alg = SEC_AES_CTR_192;
			else if (key_len == AES_KEYSIZE_256)
				*sec_alg = SEC_AES_CTR_256;
			else
				return -EINVAL;
			return 0;
		}
	} else if (!strncmp(alg, "xts", 3)) {
		if (!strncmp(alg + 4, "aes", 3)) {
			if (key_len == AES_KEYSIZE_128)
				*sec_alg = SEC_AES_XTS_128;
			else if (key_len == AES_KEYSIZE_256)
				*sec_alg = SEC_AES_XTS_256;
			else
				return -EINVAL;
			return 0;
		}
	} else
		/* other cipher algs are not supported by this version */
		return -EINVAL;
	return -EINVAL;
}

static void sec_alg_ablkcipher_com_init(struct sec_alg_ablkcipher_ctx *ctx,
					struct sec_bd_info *req, int alg)
{
	req->bd_invalid = 0;
	req->done = 0;
	req->c_alg    = (alg >> 10) & 0xf;
	req->c_mode   = (alg >> 6)  & 0xf;
	req->c_key_len = (alg >> 3)  & 0x7;
	req->c_width  = (alg >> 0)  & 0x7;
	req->seq = SEC_SEQ_CIPHER_AUTH;
	req->cipher_key_addr = lower_32_bits(ctx->enc_pkey);
	req->cipher_key_addr_hi = upper_32_bits(ctx->enc_pkey);
}

static void sec_alg_ablkcipher_init_enc(struct sec_alg_ablkcipher_ctx *ctx,
					int alg, const uint8_t *key,
					unsigned int keylen)
{
	struct sec_bd_info *req = &ctx->enc_req;

	req->cipher = SEC_CIPHER_ENCRYPT;
	memcpy(ctx->enc_key, key, keylen);
	sec_alg_ablkcipher_com_init(ctx, req, alg);
}

static void sec_alg_ablkcipher_init_dec(struct sec_alg_ablkcipher_ctx *ctx,
					int alg, const uint8_t *key,
					unsigned int keylen)
{
	struct sec_bd_info *req = &ctx->dec_req;

	req->cipher = SEC_CIPHER_DECRYPT;
	memcpy(ctx->dec_key, key, keylen);
	sec_alg_ablkcipher_com_init(ctx, req, alg);
}


static int sec_alg_ablkcipher_init_context(struct sec_alg_ablkcipher_ctx *ctx,
					    const uint8_t *key,
					    unsigned int keylen,
					    const char *alg)
{
	int sec_alg;

	if (sec_alg_info_trans(keylen, &sec_alg, alg))
		goto bad_info;

	sec_alg_ablkcipher_init_enc(ctx, sec_alg, key, keylen);
	sec_alg_ablkcipher_init_dec(ctx, sec_alg, key, keylen);
	return 0;
bad_info:
	crypto_tfm_set_flags(ctx->tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
	return -EINVAL;
}
static void sec_destroy_sec_sgl(struct sec_hw_sgl *sgl,
			dma_addr_t psec_sgl, struct device *dev)
{
	size_t sz = sizeof(struct sec_hw_sgl);
	int n = sgl->entry_sum_in_chain, sg_nctr = 0, total = 0;
	struct sec_hw_sgl *temp, *temp1;

	temp = sgl;
	for (; total < n;) {
		if (sg_nctr < HISI_MAX_SGE_NUM) {
			if (!dma_mapping_error(dev,
				temp->sge_entrys[sg_nctr].buf)) {
				dma_unmap_single(dev,
					temp->sge_entrys[sg_nctr].buf,
					temp->sge_entrys[sg_nctr].len,
					DMA_BIDIRECTIONAL);
				sg_nctr++;
				total++;
			} else
				break;
		} else {
			temp1 = temp->next;
			if (!dma_mapping_error(dev, temp->next_sgl)) {
				dma_unmap_single(dev, temp->next_sgl,
					sz, DMA_BIDIRECTIONAL);
				kfree(temp);
			} else {
				kfree(temp);
				kfree(temp1);
				break;
			}
			temp = temp1;
			sg_nctr = 0;
		}
	}
	if (dma_mapping_error(dev, psec_sgl))
		kfree(sgl);
	else
		dma_unmap_single(dev, psec_sgl, sz, DMA_BIDIRECTIONAL);
}

static struct sec_hw_sgl *sec_setup_sec_sgl(dma_addr_t *psecsgl,
				struct scatterlist *sgl, struct device *dev)
{
	struct sec_hw_sgl *sec_sgl, *temp, *temp1;
	dma_addr_t psec_sgl, ptemp;
	struct scatterlist *sg;
	int n = sg_nents(sgl), i, sg_nctr = 0, total = 0, ret;
	size_t sz = sizeof(struct sec_hw_sgl);

	if (unlikely(!n))
		return ERR_PTR(-EINVAL);
	sec_sgl = kzalloc_node(sz, GFP_ATOMIC, dev_to_node(dev));
	if (unlikely(!sec_sgl))
		return ERR_PTR(-ENOMEM);

	temp = sec_sgl;
	for_each_sg(sgl, sg, n, i) {
		if (!sg->length)
			continue;

		temp->sge_entrys[sg_nctr].buf = dma_map_single(dev,
						sg_virt(sg), sg->length,
						DMA_BIDIRECTIONAL);
		if (unlikely(dma_mapping_error(dev,
			temp->sge_entrys[sg_nctr].buf))) {
			ret = -ENOMEM;
			goto err;
		}
		temp->sge_entrys[sg_nctr].len = sg->length;

		sg_nctr++;
		total++;
		temp->data_bytes_in_sgl += sg->length;
		sec_sgl->entry_sum_in_chain = total;
		temp->entry_sum_in_sgl = sg_nctr;
		if (sg_nctr == HISI_MAX_SGE_NUM) {
			temp1 = kzalloc_node(sz, GFP_ATOMIC,
					dev_to_node(dev));
			if (unlikely(!temp1)) {
				ret = -ENOMEM;
				goto err;
			}
			temp->next = temp1;
			ptemp = dma_map_single(dev, temp1, sz,
					DMA_BIDIRECTIONAL);
			temp->next_sgl = ptemp;
			if (unlikely(dma_mapping_error(dev, ptemp))) {
				ret = -ENOMEM;
				goto err;
			}
			temp = temp1;
			sg_nctr = 0;
		}
	}
	temp->next_sgl = 0;

	psec_sgl = dma_map_single(dev, sec_sgl, sz, DMA_BIDIRECTIONAL);
	if (unlikely(dma_mapping_error(dev, psec_sgl))) {
		ret = -ENOMEM;
		goto err;
	}
	*psecsgl = psec_sgl;

	return sec_sgl;
err:
	sec_destroy_sec_sgl(sec_sgl, psec_sgl, dev);
	*psecsgl = 0;
	return ERR_PTR(ret);
}
static void sec_free_opdata(struct hisi_acc_qp *queue,
			struct sec_crypto_request *sec_req)
{
	struct device *dev = queue->p_dev;

	sec_destroy_sec_sgl(sec_req->sec_udata.in,
			sec_req->sec_udata.dma_in, dev);
	if (sec_req->ablkcipher_req->src != sec_req->ablkcipher_req->dst)
		sec_destroy_sec_sgl(sec_req->sec_udata.out,
			sec_req->sec_udata.dma_out, dev);
	if (sec_req->sec_udata.iv) {
		dma_unmap_single(dev, sec_req->sec_udata.dma_iv,
			   HISI_SEC_MAX_CIPHER_IV, DMA_TO_DEVICE);
		sec_req->sec_udata.dma_iv = 0;
		kfree(sec_req->sec_udata.iv);
		sec_req->sec_udata.iv = NULL;
	}
}
static int sec_create_opdata(struct hisi_acc_qp *queue,
			       struct scatterlist *sgl,
			       struct scatterlist *sglout,
			       struct sec_crypto_request *sec_req)
{
	struct device *dev =  queue->p_dev;
	struct sec_hw_sgl *inbufs;
	struct sec_hw_sgl *outbufs;
	dma_addr_t pinbufs;
	dma_addr_t poutbufs = 0;
	int ret;
	struct ablkcipher_request *req = sec_req->ablkcipher_req;
	struct crypto_ablkcipher *atfm = crypto_ablkcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(atfm);

	inbufs = sec_setup_sec_sgl(&pinbufs, sgl, dev);
	if (IS_ERR(inbufs)) {
		dev_err(dev, "sec_setup_sec_sgl for in sgl fail!\n");
		return PTR_ERR(inbufs);
	}
	sec_req->sec_udata.in = inbufs;
	sec_req->sec_udata.dma_in = pinbufs;
	if (sgl != sglout) {
		outbufs = sec_setup_sec_sgl(&poutbufs, sglout, dev);
		if (IS_ERR(outbufs)) {
			sec_destroy_sec_sgl(inbufs, pinbufs, dev);
			dev_err(dev, "sec_setup_sec_sgl for out sgl fail!\n");
			ret = PTR_ERR(outbufs);
			goto err;
		}
		sec_req->sec_udata.out = outbufs;
		sec_req->sec_udata.dma_out = poutbufs;
		sec_req->req.de = 1;
	} else
		sec_req->req.de = 0;

	sec_req->sec_udata.iv = kzalloc_node(HISI_SEC_MAX_CIPHER_IV,
		GFP_ATOMIC, dev_to_node(dev));
	if (unlikely(!sec_req->sec_udata.iv))
		return -ENOMEM;
	memcpy(sec_req->sec_udata.iv, req->info, tfm->crt_ablkcipher.ivsize);
	sec_req->sec_udata.dma_iv = dma_map_single(dev, sec_req->sec_udata.iv,
		HISI_SEC_MAX_CIPHER_IV, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(dev, sec_req->sec_udata.dma_iv))) {
		sec_free_opdata(queue, sec_req);
		ret = -ENOMEM;
		goto err;
	}

	return 0;
err:
	dev_err(dev, "Failed to create operatinal data for sec queue\n");

	return ret;
}

static int sec_alg_ablkcipher_setkey(struct crypto_ablkcipher *tfm,
				     const u8 *key, unsigned int keylen)
{
	struct sec_alg_ablkcipher_ctx *ctx = crypto_ablkcipher_ctx(tfm);
	struct device *dev = NULL;
	const char *alg = crypto_tfm_alg_name(&tfm->base);

	spin_lock(&ctx->lock);
	if (ctx->enc_key) {
		/* rekeying */
		dev = HISI_SEC_Q_DEV(ctx->queue);
		memset(ctx->enc_key, 0, HISI_SEC_MAX_CIPHER_KEY);
		memset(ctx->dec_key, 0, HISI_SEC_MAX_CIPHER_KEY);
		memset(&ctx->enc_req, 0, sizeof(ctx->enc_req));
		memset(&ctx->dec_req, 0, sizeof(ctx->dec_req));
	} else {
		/* new key */
		dev = HISI_SEC_Q_DEV(ctx->queue);
		ctx->enc_key = dma_zalloc_coherent(dev,
			HISI_SEC_MAX_CIPHER_KEY,
			&ctx->enc_pkey,
			GFP_ATOMIC);
		if (!ctx->enc_key) {
			spin_unlock(&ctx->lock);
			return -ENOMEM;
		}
		ctx->dec_key = dma_zalloc_coherent(dev,
			HISI_SEC_MAX_CIPHER_KEY,
			&ctx->dec_pkey,
			GFP_ATOMIC);
		if (!ctx->dec_key) {
			spin_unlock(&ctx->lock);
			goto out_free_enc;
		}
	}
	spin_unlock(&ctx->lock);
	if (sec_alg_ablkcipher_init_context(ctx, key, keylen, alg))
		goto out_free_all;
#ifdef SEC_DEBUG
	pr_info("%s  set key seccess\n", __func__);
#endif
	return 0;

out_free_all:
	memset(ctx->dec_key, 0, HISI_SEC_MAX_CIPHER_KEY);
	dma_free_coherent(dev, HISI_SEC_MAX_CIPHER_KEY,
		ctx->dec_key, ctx->dec_pkey);
	ctx->dec_key = NULL;
out_free_enc:
	memset(ctx->enc_key, 0, HISI_SEC_MAX_CIPHER_KEY);
	dma_free_coherent(dev, HISI_SEC_MAX_CIPHER_KEY,
		ctx->enc_key, ctx->enc_pkey);
	ctx->enc_key = NULL;
	return -ENOMEM;
}

static int sec_alg_ablkcipher_crypto(struct ablkcipher_request *req, int direct)
{
	struct crypto_ablkcipher *atfm = crypto_ablkcipher_reqtfm(req);
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(atfm);
	struct sec_alg_ablkcipher_ctx *ctx = crypto_tfm_ctx(tfm);
	struct sec_crypto_request *sec_req = ablkcipher_request_ctx(req);
	struct sec_bd_info *msg;
	int ret, ctr = 0;

	memset(sec_req, 0, sizeof(*sec_req));
	msg = &sec_req->req;
	if (direct)
		memcpy(msg, &(ctx->enc_req), sizeof(*msg));
	else
		memcpy(msg, &(ctx->dec_req), sizeof(*msg));

	sec_req->ablkcipher_ctx = ctx;
	sec_req->ablkcipher_req = req;
	sec_req->cb = sec_ablkcipher_alg_callback;
	*(uint64_t *)(&sec_req->req.mac_addr) = (uint64_t)sec_req;
	/*fixme cipher_len*/
	sec_req->req.cipher_len = req->nbytes;
	/*fixme gran_num cs null? */
	/*sec_req->req.gran_num = 1;*/
	sec_req->req.cipher_src_offset = 0;
	sec_req->req.src_addr_type = 1;/*0:pbuffer   1:SGL   2:PRP  todo3*/
	sec_req->req.dst_addr_type = 1;/*0:pbuffer   1:SGL   2:PRP  todo3*/

	ret = sec_create_opdata(ctx->queue, req->src, req->dst, sec_req);
	if (unlikely(ret))
		return ret;

	/* sec_req->req.data_addr_hi is filled by the this setting */
	sec_req->req.data_src_addr = lower_32_bits(sec_req->sec_udata.dma_in);
	sec_req->req.data_src_addr_hi =
				upper_32_bits(sec_req->sec_udata.dma_in);
	if (sec_req->req.de) {
		sec_req->req.data_dst_addr =
			lower_32_bits(sec_req->sec_udata.dma_out);
		sec_req->req.data_dst_addr_hi =
			upper_32_bits(sec_req->sec_udata.dma_out);
	} else {
		sec_req->req.data_dst_addr =
			lower_32_bits(sec_req->sec_udata.dma_in);
		sec_req->req.data_dst_addr_hi =
			upper_32_bits(sec_req->sec_udata.dma_in);
	}
	sec_req->req.cipher_iv_addr =
		lower_32_bits(sec_req->sec_udata.dma_iv);
	sec_req->req.cipher_iv_addr_hi =
		upper_32_bits(sec_req->sec_udata.dma_iv);
	do {
		ret = sec_queue_send((void *)ctx->queue, (void *)msg);
	} while (ret == -EAGAIN && ctr++ < 10);

	if (unlikely(ret == -EAGAIN)) {
		sec_free_opdata(ctx->queue, sec_req);
		return -EBUSY;
	}

#ifdef SEC_DEBUG
	pr_info("%s send task seccess\n", __func__);
#endif
	return -EINPROGRESS;
}

static int sec_alg_ablkcipher_encrypt(struct ablkcipher_request *req)
{
	return sec_alg_ablkcipher_crypto(req, 1);
}

static int sec_alg_ablkcipher_decrypt(struct ablkcipher_request *req)
{
	return sec_alg_ablkcipher_crypto(req, 0);
}

static int sec_alg_ablkcipher_init(struct crypto_tfm *tfm)
{
	struct hisi_sec *hisec;
	int node;
	struct sec_alg_ablkcipher_ctx *ctx = crypto_tfm_ctx(tfm);

	memset(ctx, 0, sizeof(struct sec_alg_ablkcipher_ctx));
	spin_lock_init(&ctx->lock);
	tfm->crt_ablkcipher.reqsize = sizeof(struct sec_crypto_request);
	ctx->tfm = tfm;
	/*node = cpu_to_node(smp_processor_id()); todo*/
	node = 0;
	if (node) {
		node = 0;
		pr_info("hisilicon sec_alg cpunode=%d, modify manual\n", node);
	}
	hisec = sec_device_get(node);
	if (!hisec)
		return -ENODEV;
	ctx->queue = sec_crypto_queue_alloc(hisec);//todo...
	if (!ctx->queue)
		return -ENODEV;

	return 0;
}

static void sec_alg_ablkcipher_exit(struct crypto_tfm *tfm)
{
	struct sec_alg_ablkcipher_ctx *ctx = crypto_tfm_ctx(tfm);
	//struct wd_queue *queue = ctx->queue;
	struct device *dev = HISI_SEC_Q_DEV(ctx->queue);

	if (ctx->enc_key) {
		memset(ctx->enc_key, 0, HISI_SEC_MAX_CIPHER_KEY);
		dma_free_coherent(dev, HISI_SEC_MAX_CIPHER_KEY,
			ctx->enc_key, ctx->enc_pkey);
	}
	if (ctx->dec_key) {
		memset(ctx->dec_key, 0, HISI_SEC_MAX_CIPHER_KEY);
		dma_free_coherent(dev, HISI_SEC_MAX_CIPHER_KEY,
			ctx->dec_key, ctx->dec_pkey);
	}
	sec_crypto_queue_release(ctx->queue);//fixme........
	//ctx->queue = NULL;
}

static struct crypto_alg sec_algs[] = { {
	.cra_name = "cbc(aes)",
	.cra_driver_name = "hisi_sec_aes_cbc",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = AES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "ctr(aes)",
	.cra_driver_name = "hisi_sec_aes_ctr",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = AES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "xts(aes)",
	.cra_driver_name = "hisi_sec_aes_xts",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = AES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = 2 * AES_MIN_KEY_SIZE,
			.max_keysize = 2 * AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "cbc(des)",
	.cra_driver_name = "hisi_sec_des_cbc",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = DES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = DES_KEY_SIZE,
			.max_keysize = DES_KEY_SIZE,
			.ivsize = DES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "cbc(des3_ede)",
	.cra_driver_name = "hisi_sec_3des_cbc",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = DES3_EDE_KEY_SIZE,
			.max_keysize = DES3_EDE_KEY_SIZE,
			.ivsize = DES3_EDE_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "ecb(aes)",
	.cra_driver_name = "hisi_sec_aes_ecb",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = AES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = AES_MIN_KEY_SIZE,
			.max_keysize = AES_MAX_KEY_SIZE,
			.ivsize = AES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "ecb(des)",
	.cra_driver_name = "hisi_sec_des_ecb",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = DES_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = DES_KEY_SIZE,
			.max_keysize = DES_KEY_SIZE,
			.ivsize = DES_BLOCK_SIZE,
		},
	},
}, {
	.cra_name = "ecb(des3_ede)",
	.cra_driver_name = "hisi_sec_3des_ecb",
	.cra_priority = 4001,
	.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
	.cra_blocksize = DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct sec_alg_ablkcipher_ctx),
	.cra_alignmask = 0,
	.cra_type = &crypto_ablkcipher_type,
	.cra_module = THIS_MODULE,
	.cra_init = sec_alg_ablkcipher_init,
	.cra_exit = sec_alg_ablkcipher_exit,
	.cra_u = {
		.ablkcipher = {
			.setkey = sec_alg_ablkcipher_setkey,
			.decrypt = sec_alg_ablkcipher_decrypt,
			.encrypt = sec_alg_ablkcipher_encrypt,
			.min_keysize = DES3_EDE_KEY_SIZE,
			.max_keysize = DES3_EDE_KEY_SIZE,
			.ivsize = DES3_EDE_BLOCK_SIZE,
		},
	},
} };

int sec_algs_register(void)
{
	int ret = 0, i;

	mutex_lock(&algs_lock);
	if (++active_devs != 1)
		goto unlock;

	for (i = 0; i < ARRAY_SIZE(sec_algs); i++)
		sec_algs[i].cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
			CRYPTO_ALG_ASYNC;

	ret = crypto_register_algs(sec_algs, ARRAY_SIZE(sec_algs));

unlock:
	mutex_unlock(&algs_lock);
	return ret;
}

void sec_algs_unregister(void)
{
	mutex_lock(&algs_lock);
	if (--active_devs != 0)
		goto unlock;

	crypto_unregister_algs(sec_algs, ARRAY_SIZE(sec_algs));

unlock:
	mutex_unlock(&algs_lock);
}
