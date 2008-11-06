/* version.c - Alpine Package Keeper (APK)
 *
 * Copyright (C) 2005-2008 Natanael Copa <n@tanael.org>
 * Copyright (C) 2008 Timo Teräs <timo.teras@iki.fi>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation. See http://www.gnu.org/ for details.
 */
#include <stdio.h>

#include <ctype.h>
#include "apk_defines.h"
#include "apk_version.h"

/* Gentoo version: {digit}{.digit}...{letter}{_suf{#}}...{-r#} */

enum PARTS {
	TOKEN_INVALID = -1,
	TOKEN_DIGIT_OR_ZERO,
	TOKEN_DIGIT,
	TOKEN_LETTER,
	TOKEN_SUFFIX,
	TOKEN_SUFFIX_NO,
	TOKEN_REVISION_NO,
	TOKEN_END,
};

static void next_token(int *type, apk_blob_t *blob)
{
	int n = TOKEN_INVALID;

	if (blob->len == 0 || blob->ptr[0] == 0) {
		n = TOKEN_END;
	} else if (islower(blob->ptr[0])) {
		n = TOKEN_LETTER;
	} else if (*type == TOKEN_SUFFIX && isdigit(blob->ptr[0])) {
		n = TOKEN_SUFFIX_NO;
	} else {
		switch (blob->ptr[0]) {
		case '.':
			n = TOKEN_DIGIT_OR_ZERO;
			break;
		case '_':
			n = TOKEN_SUFFIX;
			break;
		case '-':
			if (blob->len > 1 && blob->ptr[1] == 'r') {
				n = TOKEN_REVISION_NO;
				blob->ptr++;
				blob->len--;
			} else
				n = TOKEN_INVALID;
			break;
		}
		blob->ptr++;
		blob->len--;
	}

	if (n < *type) {
		if (! ((n == TOKEN_DIGIT_OR_ZERO && *type == TOKEN_DIGIT) ||
		       (n == TOKEN_SUFFIX && *type == TOKEN_SUFFIX_NO)))
			n = TOKEN_INVALID;
	}
	*type = n;
}

static int get_token(int *type, apk_blob_t *blob)
{
	static const char *pre_suffixes[] = { "alpha", "beta", "pre", "rc" };
	int v = 0, i = 0, nt = TOKEN_INVALID;

	switch (*type) {
	case TOKEN_DIGIT_OR_ZERO:
		/* Leading zero digits get a special treatment */
		if (blob->ptr[i] == '0') {
			while (blob->ptr[i] == '0' && i < blob->len)
				i++;
			nt = TOKEN_DIGIT;
			v = -i;
			break;
		}
	case TOKEN_DIGIT:
	case TOKEN_SUFFIX_NO:
	case TOKEN_REVISION_NO:
		while (isdigit(blob->ptr[i]) && i < blob->len) {
			v *= 10;
			v += blob->ptr[i++] - '0';
		}
		break;
	case TOKEN_LETTER:
		v = blob->ptr[i++];
		break;
	case TOKEN_SUFFIX:
		for (v = 0; v < ARRAY_SIZE(pre_suffixes); v++) {
			i = strlen(pre_suffixes[v]);
			if (i < blob->len &&
			    strncmp(pre_suffixes[v], blob->ptr, i) == 0)
				break;
		}
		if (v < ARRAY_SIZE(pre_suffixes)) {
			nt = TOKEN_SUFFIX_NO;
			v = v - ARRAY_SIZE(pre_suffixes);
			break;
		}
		if (strncmp("p", blob->ptr, 1) == 0) {
			nt = TOKEN_SUFFIX_NO;
			v = 1;
			break;
		}
		/* fallthrough: invalid suffix */
	default:
		*type = TOKEN_INVALID;
		return -1;
	}
	blob->ptr += i;
	blob->len -= i;
	if (nt != TOKEN_INVALID)
		*type = nt;
        else
		next_token(type, blob);

	return v;
}

int apk_version_validate(apk_blob_t ver)
{
	int t = TOKEN_DIGIT;

	while (t != TOKEN_END && t != TOKEN_INVALID)
		get_token(&t, &ver);

	return t == TOKEN_END;
}

int apk_version_compare(apk_blob_t a, apk_blob_t b)
{
	int at = TOKEN_DIGIT, bt = TOKEN_DIGIT;
	int av = 0, bv = 0;

	while (at == bt && at != TOKEN_END && av == bv) {
		av = get_token(&at, &a);
		bv = get_token(&bt, &b);
#if 0
		fprintf(stderr,
			"av=%d, at=%d, a.len=%d\n"
			"bv=%d, bt=%d, b.len=%d\n",
			av, at, a.len, bv, bt, b.len);
#endif
	}

	/* value of this token differs? */
	if (av < bv)
		return APK_VERSION_LESS;
	if (av > bv)
		return APK_VERSION_GREATER;
	if (at < bt)
		return get_token(&at, &a) < 0 ?
			APK_VERSION_LESS : APK_VERSION_GREATER;
	if (bt < at)
		return get_token(&bt, &b) > 0 ?
			APK_VERSION_LESS : APK_VERSION_GREATER;
	return APK_VERSION_EQUAL;
}