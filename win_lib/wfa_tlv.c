/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

/**
 * @file wfa_tlv.c
 * @brief File containing the tlv tag encoding/decoding routines
*/

#include "wfa_main.h"

/** Encode a packet to TLV format 
 * @param the_tag The packet type.
 * @param the_len The value length.
 * @param the_value The value buffer.
 * @param tlv_data Encoded TLV packet buffer. Caller must allocate the buffer.
 * @return 0 as success.
 */
BOOL wfaEncodeTLV(WORD the_tag, WORD the_len, BYTE *the_value, BYTE *tlv_data)
{
    BYTE *data = tlv_data;

    DPRINT_INFOL(WFA_OUT, "Inside TLV...\n");
    
    ((wfaTLV *)data)->tag = the_tag;
    ((wfaTLV *)data)->len = the_len;
    
    if(the_value != NULL && the_len != 0)
        memcpy((data+4), (BYTE *)the_value, the_len);

    return WFA_SUCCESS;
}

/** Decoding a TLV format into actually values 
 * @param tlv_data The TLV format packet buffer.
 * @param tlv_len The total length of the TLV.
 * @param ptag The TLV tag.
 * @param pval_len The value length.
 * @param pvalue Value buffer, caller must allocate the buffer.
 * @return 0 as success.
 */
BOOL wfaDecodeTLV(BYTE *tlv_data, int tlv_len, WORD *ptag, int *pval_len, BYTE *pvalue)
{ 
    wfaTLV *data = (wfaTLV *)tlv_data;
    if(pvalue == NULL)
    {
        DPRINT_ERR(WFA_ERR, "Parm buf invalid\n");
        return WFA_FAILURE;
    }

    *ptag = (WORD) data->tag; 

    *pval_len = data->len;
    if(tlv_len < *pval_len)
        return WFA_FAILURE;
    if(*pval_len != 0 && *pval_len < MAX_PARMS_BUFF)
    {
        memcpy(pvalue, tlv_data+4, *pval_len);
    }

    return WFA_SUCCESS;
}

/** Retrieve a TLV tag
 * @param tlv_data TLV buffer.
 * @return The TLV type.
 */
WORD wfaGetTLVTag(BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(ptlv != NULL)
        return ptlv->tag;
   
    return WFA_SUCCESS;
}

/** Set TLV tag to the given one
 * @param new_tag The new TLV tag.
 * @param tlv_data A TLV buffer, caller must allocate this buffer.
 * @return 0 as success.
 */
BOOL wfaSetTLVTag(WORD new_tag, BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(tlv_data == NULL)
        return WFA_FAILURE;

    ptlv->tag = new_tag;

    return WFA_SUCCESS;
}

/** Retrieve a TLV value length 
 * @param tlv_data A TLV buffer.
 * @return The value length.
 */
WORD wfaGetTLVLen(BYTE *tlv_data)
{
    wfaTLV *ptlv = (wfaTLV *)tlv_data;

    if(tlv_data == NULL)
        return WFA_FAILURE;

    return ptlv->len;
}

/** Retrieve a TLV value 
 * @param value_len TLV value length.
 * @param tlv_data A TLV data buffer.
 * @param pvalue The value buffer, caller must allocate it.
 * @return 0 as success.
 */
BOOL wfaGetTLVvalue(int value_len, BYTE *tlv_data, BYTE *pvalue)
{
    if(tlv_data == NULL)
        return WFA_FAILURE;

    memcpy(pvalue, tlv_data+WFA_TLV_HEAD_LEN, value_len);

    return WFA_SUCCESS;
}
