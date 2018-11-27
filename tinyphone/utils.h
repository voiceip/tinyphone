#pragma once

#include <pj/assert.h>
#include <pj/pool.h>
#include <pj/string.h>
#include <string.h>
#include <pjsua-lib/pjsua.h>


PJ_IDEF(pj_str_t) pj_str(std::string str)
{
	pj_str_t dst;
	dst.ptr = (char *) str.c_str();
	dst.slen =  str.length();
	return dst;
}

/*
* pj_thread_auto_register(void)
*/
PJ_DEF(pj_status_t) pj_thread_auto_register(void)
{
	pj_status_t rc;

	if (!pj_thread_is_registered())
	{
		pj_thread_desc rpc_thread_desc;
		pj_thread_t* thread_ptr;

		rc = pj_thread_register("auto_thr%p", rpc_thread_desc, &thread_ptr);
	}
	else
	{
		rc = PJ_SUCCESS;
	}

	return rc;
}
