#ifndef X509_CHECK_FLAG_ALWAYS_CHECK_SUBJECT

static ngx_int_t
ngx_ssl_check_name(ngx_str_t *name, ASN1_STRING *pattern)
{
    u_char  *s, *p, *end;
    size_t   slen, plen;

    s = name->data;
    slen = name->len;

    p = ASN1_STRING_data(pattern);
    plen = ASN1_STRING_length(pattern);

    if (slen == plen && ngx_strncasecmp(s, p, plen) == 0) {
        return NGX_OK;
    }

    if (plen > 2 && p[0] == '*' && p[1] == '.') {
        plen -= 1;
        p += 1;

        end = s + slen;
        s = ngx_strlchr(s, end, '.');

        if (s == NULL) {
            return NGX_ERROR;
        }

        slen = end - s;

        if (plen == slen && ngx_strncasecmp(s, p, plen) == 0) {
            return NGX_OK;
        }
    }

    return NGX_ERROR;
}

#endif

#ifdef X509_CHECK_FLAG_ALWAYS_CHECK_SUBJECT
    {
    int                      n, i;
    X509_NAME               *sname;
    ASN1_STRING             *str;
    X509_NAME_ENTRY         *entry;
    GENERAL_NAME            *altname;
    STACK_OF(GENERAL_NAME)  *altnames;
    /*
     * As per RFC6125 and RFC2818, we check subjectAltName extension,
     * and if it's not present - commonName in Subject is checked.
     */
    altnames = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (altnames) {
        n = sk_GENERAL_NAME_num(altnames);
        for (i = 0; i < n; i++) {
            altname = sk_GENERAL_NAME_value(altnames, i);
            if (altname->type != GEN_DNS)
                continue;
            str = altname->d.dNSName;
            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                           "SSL subjectAltName: \"%*s\"",
                           ASN1_STRING_length(str), ASN1_STRING_data(str));

            if (ngx_ssl_check_name(name, str) == NGX_OK) {
                ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL subjectAltName: match");
                GENERAL_NAMES_free(altnames);
                goto found;
            }
        }
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL subjectAltName: no match");
        GENERAL_NAMES_free(altnames);
        goto failed;
    }

    /*
     * If there is no subjectAltName extension, check commonName
     * in Subject.  While RFC2818 requires to only check "most specific"
     * CN, both Apache and OpenSSL check all CNs, and so do we.
     */
    sname = X509_get_subject_name(cert);
    if (sname == NULL)
        goto failed;
  
    i = -1;
    for ( ;; ) {
        i = X509_NAME_get_index_by_NID(sname, NID_commonName, i);
        if (i < 0)
            break; 
        entry = X509_NAME_get_entry(sname, i);
        str = X509_NAME_ENTRY_get_data(entry);
        ngx_log_debug2(NGX_LOG_DEBUG_EVENT, c->log, 0,
                       "SSL commonName: \"%*s\"",
                       ASN1_STRING_length(str), ASN1_STRING_data(str));
        if (ngx_ssl_check_name(name, str) == NGX_OK) {
            ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL commonName: match");
            goto found;
        }
    }
    ngx_log_debug0(NGX_LOG_DEBUG_EVENT, c->log, 0, "SSL commonName: no match");
    }
#endif
