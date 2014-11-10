/*
 * @f ccnl-ext-nfnprefix.c
 * @b CCN-lite, methods to
 *
 * Copyright (C) 2014, Christian Tschudin, University of Basel
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
 * 2014-10-12 created 
 */

// ----------------------------------------------------------------------

int
ccnl_nfnprefix_isNFN(struct ccnl_prefix_s *p)
{
    return p->nfnflags & CCNL_PREFIX_NFN;
}

int
ccnl_nfnprefix_isTHUNK(struct ccnl_prefix_s *p)
{
    return p->nfnflags & CCNL_PREFIX_THUNK;
}

int
ccnl_nfnprefix_contentIsNACK(struct ccnl_content_s *c)
{
    return !memcmp(c->content, ":NACK", 5);
}

void
ccnl_nfnprefix_set(struct ccnl_prefix_s *p, unsigned int flags)
{
    p->nfnflags |= flags;
}

void
ccnl_nfnprefix_clear(struct ccnl_prefix_s *p, unsigned int flags)
{
    p->nfnflags &= ~flags;
}

int
ccnl_nfnprefix_fillCallExpr(char *buf, struct fox_machine_state_s *s,
                            int exclude_param)
{
    int len, j;
    struct stack_s *entry;

    DEBUGMSG(99, "exclude parameter: %d\n", exclude_param);
    if (exclude_param >= 0){
        len = sprintf(buf, "(@x call %d", s->num_of_params);
    }
    else{
        len = sprintf(buf, "call %d", s->num_of_params);
    }

    for (j = 0; j < s->num_of_params; j++) {
        if (j == exclude_param) {
            len += sprintf(buf + len, " x");
            continue;
        }
        entry = s->params[j];
        switch (entry->type) {
        case STACK_TYPE_INT:
            len += sprintf(buf + len, " %d", *((int*)entry->content));
            break;
        case STACK_TYPE_PREFIX:
            len += sprintf(buf + len, " %s",
                           ccnl_prefix_to_path((struct ccnl_prefix_s*)entry->content));
            break;
        default:
            DEBUGMSG(1, "Invalid type in createComputationString() #%d (%d)\n",
                     j, entry->type);
            break;
        }
    }
    if (exclude_param >= 0)
        len += sprintf(buf + len, ")");
    return len;
}

struct ccnl_prefix_s *
ccnl_nfnprefix_mkCallPrefix(struct ccnl_prefix_s *name, int thunk_request,
                            struct configuration_s *config, int parameter_num)
{
    int i, len, offset = 0;
    struct ccnl_prefix_s *p;
    char *bytes = ccnl_malloc(CCNL_MAX_PACKET_SIZE);

    p = ccnl_prefix_new(name->suite, name->compcnt + 1);
    if (!p)
        return NULL;
	
    p->compcnt = name->compcnt + 1;
    p->nfnflags = CCNL_PREFIX_NFN;
    if (thunk_request)
        p->nfnflags |= CCNL_PREFIX_THUNK;

    for (i = 0, len = 0; i < name->compcnt; i++) {
        p->complen[i] = name->complen[i];
        p->comp[i] = (unsigned char*)(bytes + len);
        memcpy(p->comp[i], name->comp[i], p->complen[i]);
        len += p->complen[i];
    }

/*#ifdef USE_SUITE_CCNTLV
    if (p->suite == CCNL_SUITE_CCNTLV)
        offset = 4;
#endif*/
    p->comp[i] = (unsigned char*)(bytes + len);
    p->complen[i] = ccnl_nfnprefix_fillCallExpr(bytes + len + offset,
                                                config->fox_state,
                                                parameter_num);
/*#ifdef USE_SUITE_CCNTLV
    if (p->suite == CCNL_SUITE_CCNTLV) {
        ccnl_ccntlv_prependTL(CCNX_TLV_N_NameSegment, p->complen[i],
                              &offset, (unsigned char*) bytes + len);
        p->complen[i] += 4;
    }
#endif*/
    len += p->complen[i];

    p->bytes = ccnl_realloc(bytes, len);
    for (i = 0; i < p->compcnt; i++)
        p->comp[i] = (unsigned char*)(p->bytes + ((char*)p->comp[i] - bytes));

    return p;
}

struct ccnl_prefix_s*
ccnl_nfnprefix_mkComputePrefix(struct configuration_s *config, int suite)
{
    int i, len = 0, offset = 0;
    struct ccnl_prefix_s *p;
    char *bytes = ccnl_malloc(CCNL_MAX_PACKET_SIZE);

    p = ccnl_prefix_new(suite, 2);
    p->compcnt = 2;
    if (!p)
        return NULL;
    p->nfnflags = CCNL_PREFIX_NFN;
    if (config->fox_state->thunk_request)
        p->nfnflags |= CCNL_PREFIX_THUNK;

    p->comp[0] = (unsigned char*) bytes;
    len = p->complen[0] = ccnl_pkt_mkComponent(suite, p->comp[0], "COMPUTE");

#ifdef USE_SUITE_CCNTLV
    if (suite == CCNL_SUITE_CCNTLV)
        offset = 4;
#endif
    p->comp[1] = (unsigned char*) (bytes + len);
    p->complen[1] = ccnl_nfnprefix_fillCallExpr(bytes + len + offset,
                                                config->fox_state, -1);
#ifdef USE_SUITE_CCNTLV
    if (suite == CCNL_SUITE_CCNTLV) {
        ccnl_ccntlv_prependTL(CCNX_TLV_N_NameSegment, p->complen[1],
                              &offset, p->comp[1]);
        p->complen[1] += 4;
    }
#endif
    len += p->complen[1];

    p->bytes = ccnl_realloc(bytes, len);
    for (i = 0; i < p->compcnt; i++)
        p->comp[i] = (unsigned char*)(p->bytes + ((char*)p->comp[i] - bytes));

    return p;
}

// eof
