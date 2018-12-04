#pragma once

#ifndef UTILS_HEADER_FILE_H
#define UTILS_HEADER_FILE_H

#include "stdafx.h"
#include <pj/pool.h>
#include <pj/string.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>
#include <stdio.h>
#include <thread>
#include "consts.h"

void DisplayError(std::string message);

void print_thread_name();

static PJ_IDEF(pj_str_t) pj_str(std::string str)
{
	char* chr = strdup(str.c_str());
	pj_str_t dst;
	dst.ptr = chr;
	dst.slen = str.length();
	return dst;
}


/*
* pj_thread_auto_register(void)
* Provided by Jim Gomes on pjsip forum
* Note, here we use calloc to allocated storage for thread description
* without freeing the descriptors. This leaks 256 bytes per thread
registration.
*/
static PJ_DEF(pj_status_t) pj_thread_auto_register(void)
{

	pj_status_t rc;

	if (!pj_thread_is_registered())
	{
		pj_thread_desc *p_thread_desc;
		pj_thread_t* thread_ptr;
		p_thread_desc = (pj_thread_desc *)calloc(1, sizeof(pj_thread_desc));
		rc = pj_thread_register("auto_thr%p", *p_thread_desc, &thread_ptr);
	}
	else
	{
		rc = PJ_SUCCESS;
	}
	return rc;
}

#endif
 