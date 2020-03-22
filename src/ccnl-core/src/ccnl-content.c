/*
 * @f ccnl-content.c
 * @b CCN lite, core CCNx protocol logic
 *
 * Copyright (C) 2011-18 University of Basel
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * File history:
 * 2017-06-16 created
 */

#ifndef CCNL_LINUXKERNEL
#include "ccnl-content.h"
#include "ccnl-malloc.h"
#include "ccnl-prefix.h"
#include "ccnl-pkt.h"
#include "ccnl-os-time.h"
#include "ccnl-logging.h"
#include "ccnl-defs.h"
#else
#include <ccnl-content.h>
#include <ccnl-malloc.h>
#include <ccnl-prefix.h>
#include <ccnl-pkt.h>
#include <ccnl-os-time.h>
#include <ccnl-logging.h>
#endif

#include "ccn-lite-riot.h"

extern uint32_t num_ints;
extern uint32_t num_datas;
extern uint32_t num_pits;
extern uint32_t num_cs;
extern uint32_t num_drops;
extern uint32_t num_oom;
extern uint32_t num_repl;
extern uint32_t cs_age;

// TODO: remove unused ccnl parameter
struct ccnl_content_s*
ccnl_content_new(struct ccnl_pkt_s **pkt)
{
    if (!pkt) {
        return NULL;
    }

    struct ccnl_content_s *c;

    char s[CCNL_MAX_PREFIX_SIZE];
    (void) s;

    printf("ccnl_content_new %p <%s [%lu]>\n",
             (void*) *pkt, ccnl_prefix_to_str((*pkt)->pfx, s, CCNL_MAX_PREFIX_SIZE),
             ((*pkt)->pfx->chunknum) ? (long unsigned) *((*pkt)->pfx->chunknum) : (long unsigned) 0);

    c = (struct ccnl_content_s *) ccnl_calloc(1, sizeof(struct ccnl_content_s));
    if (!c) {

        num_oom++;
        printf("oom;%lu;%hu;%lu;%lu;%lu;%lu;%lu;%lu;%lu;%lu\n",
                (unsigned long) xtimer_now_usec64(),
                my_betw,
                (unsigned long) num_ints,
                (unsigned long) num_datas,
                (unsigned long) num_pits,
                (unsigned long) num_cs,
                (unsigned long) num_drops,
                (unsigned long) num_oom,
                (unsigned long) num_repl,
                (unsigned long) CCNL_NOW() - cs_age
        );

        printf("ccnl_content_new: calloc failed\n");
        /*if (*pkt) {*/
            /*ccnl_pkt_free(*pkt);*/
        /*}*/
        return NULL;
    }
    c->pkt = *pkt;
    *pkt = NULL;
    c->last_used = CCNL_NOW();
    c->flags = CCNL_CONTENT_FLAGS_NOT_STALE;

    return c;
}

int 
ccnl_content_free(struct ccnl_content_s *content) 
{
    if (content) {
        if (content->pkt) {
            ccnl_pkt_free(content->pkt);
        }
        
        if (content) {
            ccnl_free(content);
        }

        return 0;
    }

    return -1;
}
