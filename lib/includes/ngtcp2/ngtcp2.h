/*
 * ngtcp2
 *
 * Copyright (c) 2017 ngtcp2 contributors
 * Copyright (c) 2017 nghttp2 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NGTCP2_H
#define NGTCP2_H

/* Define WIN32 when build target is Win32 API (borrowed from
   libcurl) */
#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#define WIN32
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#if defined(_MSC_VER) && (_MSC_VER < 1800)
/* MSVC < 2013 does not have inttypes.h because it is not C99
   compliant.  See compiler macros and version number in
   https://sourceforge.net/p/predef/wiki/Compilers/ */
#include <stdint.h>
#else /* !defined(_MSC_VER) || (_MSC_VER >= 1800) */
#include <inttypes.h>
#endif /* !defined(_MSC_VER) || (_MSC_VER >= 1800) */
#include <sys/types.h>
#include <stdarg.h>

#include <ngtcp2/version.h>

#ifdef NGTCP2_STATICLIB
#define NGTCP2_EXTERN
#elif defined(WIN32)
#ifdef BUILDING_NGTCP2
#define NGTCP2_EXTERN __declspec(dllexport)
#else /* !BUILDING_NGTCP2 */
#define NGTCP2_EXTERN __declspec(dllimport)
#endif /* !BUILDING_NGTCP2 */
#else  /* !defined(WIN32) */
#ifdef BUILDING_NGTCP2
#define NGTCP2_EXTERN __attribute__((visibility("default")))
#else /* !BUILDING_NGTCP2 */
#define NGTCP2_EXTERN
#endif /* !BUILDING_NGTCP2 */
#endif /* !defined(WIN32) */

typedef enum {
  NGTCP2_ERR_INVALID_ARGUMENT = -201,
  NGTCP2_ERR_UNKNOWN_PKT_TYPE = -202
} ngtcp2_error;

typedef enum {
  NGTCP2_PKT_FLAG_NONE = 0,
  NGTCP2_PKT_FLAG_LONG_FORM = 0x01,
  NGTCP2_PKT_FLAG_CONN_ID = 0x02,
  NGTCP2_PKT_FLAG_KEY_PHASE = 0x04
} ngtcp2_pkt_flag;

typedef enum {
  NGTCP2_PKT_VERSION_NEGOTIATION = 0x01,
  NGTCP2_PKT_CLIENT_INITIAL = 0x02,
  NGTCP2_PKT_SERVER_STATELESS_RETRY = 0x03,
  NGTCP2_PKT_SERVER_CLEARTEXT = 0x04,
  NGTCP2_PKT_CLIENT_CLEARTEXT = 0x05,
  NGTCP2_PKT_0RTT_PROTECTED = 0x06,
  NGTCP2_PKT_1RTT_PROTECTED_K0 = 0x07,
  NGTCP2_PKT_1RTT_PROTECTED_K1 = 0x08,
  NGTCP2_PKT_PUBLIC_RESET = 0x09,
  NGTCP2_PKT_01 = 0x01,
  NGTCP2_PKT_02 = 0x02,
  NGTCP2_PKT_03 = 0x03,
} ngtcp2_pkt_type;

typedef enum {
  NGTCP2_FRAME_PADDING = 0x00,
  NGTCP2_FRAME_RST_STREAM = 0x01,
  NGTCP2_FRAME_CONNECTION_CLOSE = 0x02,
  NGTCP2_FRAME_GOAWAY = 0x03,
  NGTCP2_FRAME_MAX_DATA = 0x04,
  NGTCP2_FRAME_MAX_STREAM_DATA = 0x05,
  NGTCP2_FRAME_MAX_STREAM_ID = 0x06,
  NGTCP2_FRAME_PING = 0x07,
  NGTCP2_FRAME_BLOCKED = 0x08,
  NGTCP2_FRAME_STREAM_BLOCKED = 0x09,
  NGTCP2_FRAME_STREAM_ID_NEEDED = 0x0a,
  NGTCP2_FRAME_NEW_CONNECTION_ID = 0x0b,
  NGTCP2_FRAME_ACK = 0xa0,
  NGTCP2_FRAME_STREAM = 0xc0
} ngtcp2_frame_type;

typedef struct {
  uint8_t flags;
  uint8_t type;
  uint64_t conn_id;
  uint32_t pkt_num;
  uint32_t version;
} ngtcp2_pkt_hd;

typedef struct {
  uint8_t type;
  uint8_t fin;
  uint64_t stream_id;
  uint64_t offset;
  size_t datalen;
  const uint8_t *data;
} ngtcp2_stream;

typedef struct {
  uint8_t type;
  uint64_t largest_ack;
} ngtcp2_ack;

typedef struct {
  uint8_t type;
  /**
   * The length of contiguous PADDING frames.
   */
  size_t len;
} ngtcp2_padding;

typedef union {
  uint8_t type;
  ngtcp2_stream stream;
  ngtcp2_ack ack;
  ngtcp2_padding padding;
} ngtcp2_frame;

/**
 * @function
 *
 * `ngtcp2_pkt_decode_hd` decodes QUIC packet header included in |pkt|
 * of length |pktlen|, and sotres the result in the object pointed by
 * |dest|.  This function can decode both long and short packet
 * header.  On success, if ``dest->flags & NGTCP2_PKT_FLAG_LONG_FORM``
 * is nonzero, the packet header has long form.
 *
 * This function returns the exact number of bytes to be read in order
 * to decode packet header, or one of the following negative error
 * codes:
 *
 * :enum:`NGTCP2_ERR_INVALID_ARGUMENT`
 *     Packet is too short
 * :enum:`NGTCP2_ERR_UNKNOWN_PKT_TYPE`
 *     Packet type is unknown
 */
NGTCP2_EXTERN ssize_t ngtcp2_pkt_decode_hd(ngtcp2_pkt_hd *dest,
                                           const uint8_t *pkt, size_t pktlen);

NGTCP2_EXTERN ssize_t ngtcp2_pkt_decode_frame(ngtcp2_frame *dest,
                                              const uint8_t *payload,
                                              size_t payloadlen);

/* Protected Packet Encoder: ppe */
struct ngtcp2_ppe;
typedef struct ngtcp2_ppe ngtcp2_ppe;

struct ngtcp2_crypto_ctx;
typedef struct ngtcp2_crypto_ctx ngtcp2_crypto_ctx;

NGTCP2_EXTERN int ngtcp2_ppe_init(ngtcp2_ppe *ppe, ngtcp2_crypto_ctx *cctx,
                                  uint8_t *out, size_t outlen);
NGTCP2_EXTERN ssize_t ngtcp2_ppe_encode_hd(ngtcp2_ppe *ppe,
                                           const ngtcp2_pkt_hd *hd);
NGTCP2_EXTERN ssize_t ngtcp2_ppe_encode_frame(ngtcp2_ppe *ppe,
                                              const ngtcp2_frame *fm);
NGTCP2_EXTERN ssize_t ngtcp2_ppe_final(ngtcp2_ppe *ppe);

/* Unprotected Packet Encoder: upe */
struct ngtcp2_upe;
typedef struct ngtcp2_upe ngtcp2_upe;

NGTCP2_EXTERN int ngtcp2_upe_init(ngtcp2_upe *upe, uint8_t *out, size_t outlen);
NGTCP2_EXTERN ssize_t ngtcp2_upe_encode_hd(ngtcp2_upe *upe,
                                           const ngtcp2_pkt_hd *hd);
NGTCP2_EXTERN ssize_t ngtcp2_upe_encode_frame(ngtcp2_upe *upe,
                                              const ngtcp2_frame *fm);
NGTCP2_EXTERN ssize_t ngtcp2_upe_final(ngtcp2_upe *upe);

/**
 * @function
 *
 * `ngtcp2_pkt_verify` verifies the integrity of QUIC unprotected
 * packet included in |pkt| of length |pktlen|.
 *
 * This function returns 0 if it succeeds, or one of the following
 * negative error codes:
 *
 * TBD
 */
NGTCP2_EXTERN int ngtcp2_pkt_verify(const uint8_t *pkt, size_t pktlen);

/**
 * @function
 *
 * `ngtcp2_crypto_ctx_decrypt` performs decryption of QUIC payload
 * included in QUIC packet |pkg| of length |pktlen|.  The result of
 * decryption is written to the memory pointed by |dest|.  The valid
 * length of |dest| is given in |destlen|.
 *
 * This function returns the number of bytes written to |dest| if it
 * succeeds, or one of the following negative error codes:
 *
 * TBD
 */
NGTCP2_EXTERN ssize_t ngtcp2_crypto_ctx_decrypt(ngtcp2_crypto_ctx *cctx,
                                                uint8_t *dest, size_t destlen,
                                                const uint8_t *pkt,
                                                size_t pktlen);

/**
 * TODO
 * ngtcp2_conn:
 * - Must have callback to get random byte string generator
 */
struct ngtcp2_conn;

typedef struct ngtcp2_conn ngtcp2_conn;

#ifdef __cplusplus
}
#endif

#endif /* NGTCP2_H */