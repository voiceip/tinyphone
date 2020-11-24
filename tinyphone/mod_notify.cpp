#include "stdafx.h"
#include <pjsua2/account.hpp>
#include <pjsua2/call.hpp>
#include <pjsua2/endpoint.hpp>
#include <pj/ctype.h>
#include <pjsua-lib/pjsua_internal.h>
#include "phone.h"
#include "consts.h"
#include "config.h"

using namespace pj;

/*****************************************************************************
 * A simple module to handle incoming NOTIFY request
 */

/* Notification on incoming request */
static pj_bool_t options_on_rx_request(pjsip_rx_data *rdata)
{
    pjsip_tx_data *tdata;
    pjsip_response_addr res_addr;
    pj_status_t status;
    auto endpoint = pjsua_get_pjsip_endpt();

    /* Only want to handle NOTIFY requests */
    if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method,pjsip_get_notify_method()) != 0){
        return PJ_FALSE;
    }

    /* Create basic response. */
    status = pjsip_endpt_create_response(endpoint, rdata, 200, NULL,&tdata);
    if (status != PJ_SUCCESS) {
        pjsua_perror(__FILENAME__, "Unable to create NOTIFY response", status);
        return PJ_TRUE;
    }

    /* Add User-Agent header */
    if (pjsua_var.ua_cfg.user_agent.slen) {
        const pj_str_t USER_AGENT = { "User-Agent", 10};
        pjsip_hdr *h;
        h = (pjsip_hdr*) pjsip_generic_string_hdr_create(tdata->pool, &USER_AGENT,&pjsua_var.ua_cfg.user_agent);
        pjsip_msg_add_hdr(tdata->msg, h);
    }
 
    /* Send response */
    pjsip_get_response_addr(tdata->pool, rdata, &res_addr);
    status = pjsip_endpt_send_response(endpoint, &res_addr, tdata, NULL, NULL);
    if (status != PJ_SUCCESS)
        pjsip_tx_data_dec_ref(tdata);

    return PJ_TRUE;
}


/* The module instance. */
static pjsip_module notify_handler =
{
    NULL, NULL,                /* prev, next.        */
    { "mod-notify", 51 },    /* Name.        */
    -1,                    /* Id            */
    PJSIP_MOD_PRIORITY_APPLICATION,    /* Priority            */
    NULL,                /* load()        */
    NULL,                /* start()        */
    NULL,                /* stop()        */
    NULL,                /* unload()        */
    &options_on_rx_request,        /* on_rx_request()    */
    NULL,                /* on_rx_response()    */
    NULL,                /* on_tx_request.    */
    NULL,                /* on_tx_response()    */
    NULL,                /* on_tsx_state()    */

};

namespace tp {
    bool TinyPhone::InitOptionsModule(){
        /* Register our module to receive incoming requests. */
        if (ApplicationConfig.handleNOTIFY) {
            auto status = pjsip_endpt_register_module(pjsua_get_pjsip_endpt(), &notify_handler);
            return status == PJ_SUCCESS;
        }
        return true;
    }
}
