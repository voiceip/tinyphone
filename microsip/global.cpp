/* 
 * Copyright (C) 2011-2018 MicroSIP (http://www.microsip.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "global.h"
#include "settings.h"
#include "utf.h"
#include "langpack.h"
#include <afxinet.h>

#ifdef UNICODE
#define CF_TEXT_T CF_UNICODETEXT
#else
#define CF_TEXT_T CF_TEXT
#endif

struct call_tonegen_data *tone_gen = NULL;
int transport;
pjsua_acc_id account;
pjsua_acc_id account_local;
CString lastTransferNumber;
pjsua_conf_port_id msip_conf_port_id;
pjsua_call_id msip_conf_port_call_id;

int msip_audio_input;
int msip_audio_output;
int msip_audio_ring;

CList<pjmedia_port*,pjmedia_port*> DTMFTonegens;


CString GetErrorMessage(pj_status_t status)
{
	CStringA str;
	char *buf = str.GetBuffer(PJ_ERR_MSG_SIZE-1);
	pj_strerror(status, buf, PJ_ERR_MSG_SIZE);
	str.ReleaseBuffer();
	int i = str.ReverseFind( '(' );
	if (i!=-1) {
		str = str.Left(i-1);
	}
	if (str == "Invalid Request URI" || str == "Invalid URI") {
		str = "Invalid number";
	}
	return Translate(CString(str).GetBuffer());
}

BOOL ShowErrorMessage(pj_status_t status)
{
	if (status!=PJ_SUCCESS) {
		AfxMessageBox(GetErrorMessage(status));
		return TRUE;
	} else {
		return FALSE;
	}
}

CString RemovePort(CString domain)
{
	int pos = domain.Find(_T(":"));
	if (pos != -1) {
		return domain.Mid(0,pos);
	} else {
		return domain;
	}
}

BOOL IsIP(CString host)
{
	CStringA hostA(host);
	char *pHost = hostA.GetBuffer();
	unsigned long ulAddr  = inet_addr(pHost);
	if (ulAddr !=INADDR_NONE && ulAddr != INADDR_ANY) {
		struct in_addr antelope;
		antelope.S_un.S_addr = ulAddr;
		if (strcmp(inet_ntoa(antelope),pHost)==0) {
			return TRUE;
		}
	}
	return FALSE;
}

void ParseSIPURI(CString in, SIPURI* out)
{
	//	tone_gen.toneslot = -1;
	//	tone_gen = NULL;

	// "WEwewew rewewe" <sip:qqweqwe@qwerer.com;rrrr=tttt;qweqwe=rrr?qweqwr=rqwrqwr>
	// sip:qqweqwe@qwerer.com;rrrr=tttt;qweqwe=rrr?qweqwr=rqwrqwr
	if (in.Right(1) == _T(">")) {
		in = in.Left(in.GetLength()-1);
	}
	out->name = _T("");
	out->user = _T("");
	out->domain = _T("");
	out->parameters = _T("");

	int start = in.Find( _T("sip:") );
	int end;
	if (start>0)
	{
		out->name = in.Left(start);
		out->name.Trim(_T(" \" <"));
		if (!out->name.CompareNoCase(_T("unknown")))
		{
			out->name = _T("");
		}
	}
	if (start>=0)
	{
		start+=4;
	} else {
		start = 0;
	}
	end = in.Find( _T("@"), start );
	if (end>=0)
	{
		out->user=in.Mid(start,end-start);
		start=end+1;
	}
	end = in.Find( _T(";"), start );
	if (end>=0) {
		out->domain = in.Mid(start,end-start);
		start=end;
		out->parameters = in.Mid(start);
	} else {
		end = in.Find( _T("?"), start );
		if (end>=0) {
			out->domain = in.Mid(start,end-start);
			start=end;
			out->parameters = in.Mid(start);
		} else {
			out->domain = in.Mid(start);
		}
	}
}

CString PjToStr(const pj_str_t* str, BOOL utf)
{
	CStringA rab;
	rab.Format("%.*s", str->slen, str->ptr);
	if (utf)
	{
#ifdef _UNICODE
		WCHAR* msg;
		Utf8DecodeCP(rab.GetBuffer(), CP_ACP, &msg);
		return msg;
#else
		return Utf8DecodeCP(rab.GetBuffer(), CP_ACP, NULL);
#endif
	} else 
	{
		return CString(rab);
	}
}

pj_str_t StrToPjStr(CString str)
{
	return pj_str(StrToPj(str));
}

char* StrToPj(CString str)
{
#ifdef _UNICODE
	return Utf8EncodeUcs2(str.GetBuffer());
#else
	return Utf8EncodeCP(str.GetBuffer(), CP_ACP);
#endif
}

CString Utf8DecodeUni(CStringA str)
{
#ifdef _UNICODE
	LPTSTR msg;
	Utf8DecodeCP(str.GetBuffer(), CP_ACP, &msg);
	return msg;
#else
	return Utf8DecodeCP(str.GetBuffer(), CP_ACP, NULL);
#endif
}

CStringA UnicodeToAnsi(CString str)
{
	CStringA res; 
	int nCount = str.GetLength();            
	for( int nIdx =0; nIdx < nCount; nIdx++ )      
	{          
		res+=str[nIdx];
	}
	return res;
}

CString AnsiToUnicode(CStringA str)
{
	CString res; 
	int nCount = str.GetLength();            
	for( int nIdx =0; nIdx < nCount; nIdx++ )      
	{          
		res+=str[nIdx];
	}
	return res;
}

CString XMLEntityDecode(CString str)
{
	str.Replace(_T("&lt;"),_T("<"));
	str.Replace(_T("&gt;"),_T(">"));
	str.Replace(_T("&quot;"),_T("\""));
	str.Replace(_T("&amp;"),_T("&"));
	return str;
}

CString XMLEntityEncode(CString str)
{
	str.Replace(_T("&"),_T("&amp;"));
	str.Replace(_T("<"),_T("&lt;"));
	str.Replace(_T(">"),_T("&gt;"));
	str.Replace(_T("\""),_T("&quot;"));
	return str;
}

void OpenURL(CString url)
{
	CString param;
	param.Format(_T("url.dll,FileProtocolHandler %s"),url);
	ShellExecute(NULL, NULL, _T("rundll32.exe"), param, NULL, SW_SHOWNORMAL);
}

CString GetDuration(int sec, bool zero)
{
	CString duration;
	if (sec || zero) {
		int h,m,s;
		s = sec;
		h = s/3600;
		s = s%3600;
		m = s/60;
		s = s%60;
		if (h) {
			duration.Format(_T("%d:%02d:%02d"),h,m,s);
		} else {
			duration.Format(_T("%d:%02d"),m,s);
		}
	}
	return duration;
}

void AddTransportSuffix(CString &str)
{
	switch (transport)
	{ 
	case MSIP_TRANSPORT_TCP:
		str.Append(_T(";transport=tcp"));
		break;
	case MSIP_TRANSPORT_TLS:
		str.Append(_T(";transport=tls"));
		break;
	}
}

CString GetSIPURI(CString str, bool isSimple, bool isLocal, CString domain)
{
	CString rab = str;
	rab.MakeLower();
	int pos = rab.Find(_T("sip:"));
	if (pos==-1)
	{
		str=_T("sip:")+str;
	}
	if (!isLocal) {
		pos = str.Find(_T("@"));
		if (accountSettings.accountId && pos == -1) {
			str.Append(_T("@") + (!domain.IsEmpty() ? domain : accountSettings.account.domain));
		}
	}
	if (str.GetAt(str.GetLength()-1)=='>')
	{
		str = str.Left(str.GetLength()-1);
		if (!isSimple) {
			if (!isLocal || !accountSettings.accountId) {
				AddTransportSuffix(str);
			}
		}
		str += _T(">");
	} else {
		if (!isSimple) {
			if (!isLocal || !accountSettings.accountId) {
				AddTransportSuffix(str);
			}
		}
		str = _T("<") + str + _T(">");
	}
	return str;
}

bool SelectSIPAccount(CString number, pjsua_acc_id &acc_id, pj_str_t &pj_uri)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return false;
	}
	SIPURI sipuri;
	ParseSIPURI(number, &sipuri);
	if (pjsua_acc_is_valid(account) && pjsua_acc_is_valid(account_local)) {
		acc_id = account;
		if (accountSettings.account.domain != sipuri.domain) {
			int pos = sipuri.domain.Find(_T(":"));
			CString domainWithoutPort = RemovePort(sipuri.domain);
			if (domainWithoutPort.CompareNoCase(_T("localhost"))==0 || IsIP(domainWithoutPort)) {
				acc_id = account_local;
			}
		}
	} else if (pjsua_acc_is_valid(account)) {
		acc_id = account;
	} else if (pjsua_acc_is_valid(account_local)) {
		acc_id = account_local;
	} else {
		return false;
	}
	pj_uri = StrToPjStr ( GetSIPURI(number, acc_id == account_local, acc_id == account_local) );
	return true;
}

bool IsPSTNNnmber(CString number)
{
	bool isDigits = true;
	for (int i = 0; i < number.GetLength(); i++)
	{
		if ((number[i] > '9' || number[i] < '0') && number[i] != '*' && number[i] != '#' && number[i] != '.' && number[i] != '-' && number[i] != '(' && number[i] != ')' && number[i] != ' ' && number[0] != '+')
		{
			isDigits = false;
			break;
		}
	}
	return isDigits;
}

CString FormatNumber(CString number, CString *commands) {
	int pos = number.Find(',');
	if (pos>0 && pos<number.GetLength()-1) {
		if (commands) {
			*commands = number.Mid(pos+1);
		}
		number = number.Mid(0,pos);
	}
	CString numberFormated = number;
	pjsua_acc_id acc_id;
	pj_str_t pj_uri;
	bool isLocal = SelectSIPAccount(number,acc_id,pj_uri) && acc_id == account_local;
	if (!isLocal) {
		if (IsPSTNNnmber(number)) {
			numberFormated.Remove('.');
			numberFormated.Remove('-');
			numberFormated.Remove('(');
			numberFormated.Remove(')');
			numberFormated.Remove(' ');
		}
	}
	return GetSIPURI(numberFormated,true,isLocal);
}

bool IniSectionExists(CString section, CString iniFile)
{
	CString str;
	LPTSTR ptr = str.GetBuffer(3);
	int result = GetPrivateProfileString(section,NULL, NULL, ptr, 3, iniFile);
	str.ReleaseBuffer();
	return result;
}

void OpenHelp(CString code)
{
	CString url = _T(_GLOBAL_HELP_WEBSITE);
	url.Append(_T("#"));
	OpenURL(url+code);
}

void MSIP::GetScreenRect(CRect *rect)
{
	rect->left = GetSystemMetrics (SM_XVIRTUALSCREEN);
	rect->top = GetSystemMetrics (SM_YVIRTUALSCREEN);
	rect->right = GetSystemMetrics (SM_CXVIRTUALSCREEN) - rect->left;
	rect->bottom = GetSystemMetrics (SM_CYVIRTUALSCREEN) - rect->top;
}

struct call_tonegen_data *call_init_tonegen(pjsua_call_id call_id)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return NULL;
	}
    pj_status_t status;
	pj_pool_t *pool;
	struct call_tonegen_data *cd;
	pjsua_call_info ci;

	if (call_id !=-1 ) {
		pjsua_call_get_info(call_id, &ci);

		if (ci.media_status != PJSUA_CALL_MEDIA_ACTIVE)
			return NULL;
	}

	pool = pjsua_pool_create("mycall", 512, 512);
	cd = PJ_POOL_ZALLOC_T(pool, struct call_tonegen_data);
	cd->pool = pool;

	status = pjmedia_tonegen_create(cd->pool, 8000, 1, 64, 16, 0, &cd->tonegen);
	if (status != PJ_SUCCESS) {
		return NULL;
	}

	pjsua_conf_add_port(cd->pool, cd->tonegen, &cd->toneslot);

	if (call_id !=-1 ) {
		pjsua_conf_connect(cd->toneslot, ci.conf_slot);
	}
	if (accountSettings.localDTMF || call_id==-1) {
		pjsua_conf_connect(cd->toneslot, 0);
	}

	if (call_id !=-1 ) {
			call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
			if (!user_data) {
				user_data = new call_user_data(call_id);
				pjsua_call_set_user_data(call_id, user_data);
			}
			user_data->tonegen_data = cd;
	}
	return cd;
}


static UINT_PTR destroyDTMFPlayerTimer = NULL;
static UINT_PTR tonegenBusyTimer = NULL;

void destroyDTMFPlayerTimerHandler(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime)
{
	if (!tone_gen || pjsua_var.state!=PJSUA_STATE_RUNNING ||!pjmedia_tonegen_is_busy(tone_gen->tonegen)) {
		if (destroyDTMFPlayerTimer) {
			KillTimer(NULL,destroyDTMFPlayerTimer);
			destroyDTMFPlayerTimer = NULL;
		}
		call_deinit_tonegen(-1);
	}
}

void DTMFQueueTimerHandler(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	pjsua_call_id call_id = (pjsua_call_id) idEvent;
	if (pjsua_var.state==PJSUA_STATE_RUNNING && pjsua_call_is_active(call_id)) {
		call_user_data *user_data = (call_user_data *) pjsua_call_get_user_data(call_id);
		if (user_data && !user_data->commands.IsEmpty()) {
			CString dtmf;
			int pos = user_data->commands.Find(',');
			if (pos!=-1) {
				dtmf = user_data->commands.Mid(0,pos);
				user_data->commands = user_data->commands.Mid(pos+1);
			} else {
				dtmf = user_data->commands;
				user_data->commands.Empty();
			}
			if (!dtmf.IsEmpty()) {
				msip_call_dial_dtmf(call_id, dtmf);
			}
			if (!user_data->commands.IsEmpty()) {
				::SetTimer(hwnd, idEvent, 1000 + 200 * dtmf.GetLength() , (TIMERPROC)DTMFQueueTimerHandler);
			}
		}
	}
}

void tonegenBusyHandler(
  HWND hwnd,
  UINT uMsg,
  UINT_PTR idEvent,
  DWORD dwTime) 
{
	POSITION pos = DTMFTonegens.GetHeadPosition();
	while (pos) {
		POSITION posKey = pos;
		pjmedia_port *port = DTMFTonegens.GetNext(pos);
		if (pjsua_var.state!=PJSUA_STATE_RUNNING || pjmedia_tonegen_is_busy(port) == PJ_FALSE) {
			DTMFTonegens.RemoveAt(posKey);
		}
	};
	if (DTMFTonegens.IsEmpty()) {
		KillTimer(NULL,tonegenBusyTimer);
		tonegenBusyTimer = NULL;
		CWnd *hWnd = AfxGetApp()->m_pMainWnd;
		if (hWnd) {
			hWnd->PostMessage(UM_REFRESH_LEVELS, NULL, NULL);
		}
	}
}

void msip_set_sound_device(int outDev, bool forse){
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	int in,out;
	if (forse || pjsua_get_snd_dev(&in,&out)!=PJ_SUCCESS || msip_audio_input!=in || outDev!=out ) {
		pjsua_set_snd_dev(msip_audio_input, outDev);
	}
}


void msip_call_dial_dtmf(pjsua_call_id call_id, CString digits)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	bool simulate = true;
	if (call_id != PJSUA_INVALID_ID) {
		pjsua_call_info call_info;
		pjsua_call_get_info(call_id, &call_info);
		if (call_info.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
			pj_str_t pj_digits = StrToPjStr ( digits );
			if (accountSettings.DTMFMethod == 1) {
				// in-band
				simulate = !call_play_digit(call_id, StrToPj(digits));
			} else if (accountSettings.DTMFMethod == 2) {
				// RFC2833
				pjsua_call_dial_dtmf(call_id, &pj_digits);
			} else if (accountSettings.DTMFMethod == 3) {
				// sip-info
				msip_call_send_dtmf_info(call_id, pj_digits);
			} else {
				// auto
				if (pjsua_call_dial_dtmf(call_id, &pj_digits) != PJ_SUCCESS) {
					simulate = !call_play_digit(call_id, StrToPj(digits));
				}
			}
		}
	}
	if (simulate && accountSettings.localDTMF) {
		msip_set_sound_device(msip_audio_output);
		call_play_digit(-1, StrToPj(digits));
	}
}

BOOL call_play_digit(pjsua_call_id call_id, const char *digits, int duration)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return FALSE;
	}
	pjmedia_tone_digit d[16];
	unsigned i, count = strlen(digits);
	struct call_tonegen_data *cd;
	call_user_data *user_data = NULL;
	if (call_id !=-1 ) {
			user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
			if (user_data && user_data->tonegen_data) {
				cd = user_data->tonegen_data;
			} else {
				cd = NULL;
			}
	} else {
		cd = tone_gen;
	}
	if (!cd)
		cd = call_init_tonegen(call_id);
	if (!cd) 
		return FALSE;
	if (call_id == -1 ) {
		tone_gen = cd;
	}

	if (count > PJ_ARRAY_SIZE(d))
		count = PJ_ARRAY_SIZE(d);

	pj_bzero(d, sizeof(d));
	for (i=0; i<count; ++i) {
		d[i].digit = digits[i];
		d[i].on_msec = duration;
		d[i].off_msec = 50;
		d[i].volume = 0;
	}

	if (call_id != -1) {
		// mute microphone before play in-band tones
		msip_audio_input_set_volume(0, true);
		if (DTMFTonegens.Find(cd->tonegen)==NULL) {
			DTMFTonegens.AddTail(cd->tonegen);
		}
		if (tonegenBusyTimer) {
			KillTimer(NULL,tonegenBusyTimer);
		}
		tonegenBusyTimer = SetTimer(NULL, NULL, 800, (TIMERPROC)tonegenBusyHandler);

	}
	
	pjmedia_tonegen_play_digits(cd->tonegen, count, d, 0);

	if (call_id == -1 ) {
		if (destroyDTMFPlayerTimer) {
			KillTimer(NULL,destroyDTMFPlayerTimer);
		}
		destroyDTMFPlayerTimer = SetTimer(NULL, NULL, 5000, (TIMERPROC)destroyDTMFPlayerTimerHandler);
	}
	return TRUE;
}

void call_deinit_tonegen(pjsua_call_id call_id)
{
	struct call_tonegen_data *cd;
	call_user_data *user_data = NULL;

	if (call_id !=-1 ) {
		if (pjsua_var.state==PJSUA_STATE_RUNNING) {
			user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
		}
		if (user_data && user_data->tonegen_data) {
			cd = user_data->tonegen_data;
			POSITION position = DTMFTonegens.Find(cd->tonegen);
			if (position!=NULL) {
				DTMFTonegens.RemoveAt(position);
			}

		} else {
			cd = NULL;
		}
	} else {
		cd = tone_gen;
	}
	if (!cd)
		return;

	if (pjsua_var.state==PJSUA_STATE_RUNNING) {
		pjsua_conf_remove_port(cd->toneslot);
		pjmedia_port_destroy(cd->tonegen);
		pj_pool_release(cd->pool);
	}

	if (call_id !=-1 ) {
		if (user_data) {
			user_data->tonegen_data = NULL;
		}
	} else {
		tone_gen = NULL;
	}
}

unsigned call_get_count_noincoming()
{
	unsigned noincoming_count = 0;
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_var.state==PJSUA_STATE_RUNNING && pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_call_info call_info;
			pjsua_call_get_info(call_ids[i], &call_info);
			if (call_info.role!=PJSIP_ROLE_UAS || (call_info.state!=PJSIP_INV_STATE_INCOMING && call_info.state!=PJSIP_INV_STATE_EARLY)) {
				noincoming_count++;
			}
		}
	}
	return noincoming_count;
}

void call_hangup_all_noincoming(bool onHold)
{
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_var.state==PJSUA_STATE_RUNNING && pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_call_info call_info;
			pjsua_call_get_info(call_ids[i], &call_info);
			if (call_info.role!=PJSIP_ROLE_UAS || (call_info.state!=PJSIP_INV_STATE_INCOMING && call_info.state!=PJSIP_INV_STATE_EARLY)) {
				if (onHold && call_info.media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD) {
					continue;
				}
				msip_call_hangup_fast(call_ids[i]);
			}
		}
	}
}

CStringA urlencode(CStringA str)
{
    CStringA escaped;
	int max = str.GetLength();
    for(int i=0; i<max; i++)
    {
		const char chr = str.GetAt(i);
        if ( (48 <= chr && chr <= 57) ||//0-9
             (65 <= chr && chr <= 90) ||//abc...xyz
             (97 <= chr && chr <= 122) || //ABC...XYZ
             (chr=='~' || chr=='!' || chr=='*' || chr=='(' || chr==')' || chr=='\'')
        )
        {
			escaped.AppendFormat("%c",chr);
        }
        else
        {
            escaped.Append("%");
            escaped.Append(char2hex(chr));//converts char 255 to string "ff"
        }
    }
    return escaped;
}

CStringA char2hex( char dec )
{
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48inascii
    if (10<= dig1 && dig1<=15) dig1+=97-10; //a,97inascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    if (10<= dig2 && dig2<=15) dig2+=97-10;

    CStringA r;
    r.AppendFormat ("%c", dig1);
    r.AppendFormat ("%c", dig2);
    return r;
}

static DWORD WINAPI URLGetAsyncThread( LPVOID lpParam ) 
{
	URLGetAsyncData *data = (URLGetAsyncData *)lpParam;
	data->body.Empty();
	data->headers.Empty();
	data->statusCode = 0;
	if (!data->url.IsEmpty()) {
		try {
			CInternetSession session;
			CHttpConnection* pHttp = NULL;
			CHttpFile* pFile = NULL;
			DWORD dwServiceType;
			CString strServer;
			CString strObject;
			INTERNET_PORT nPort;
			if (AfxParseURL(data->url, dwServiceType, strServer, strObject, nPort)) {
				pHttp = session.GetHttpConnection(strServer, (dwServiceType==AFX_INET_SERVICE_HTTPS ? INTERNET_FLAG_SECURE : 0), nPort);
				pFile = pHttp->OpenRequest(CHttpConnection::HTTP_VERB_GET,strObject, 0, 1, 0, 0,
					INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | (dwServiceType==AFX_INET_SERVICE_HTTPS ? INTERNET_FLAG_SECURE : 0));
				if (dwServiceType==AFX_INET_SERVICE_HTTPS) {
					pFile->SetOption(INTERNET_OPTION_SECURITY_FLAGS,
						SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
						SECURITY_FLAG_IGNORE_UNKNOWN_CA |
						SECURITY_FLAG_IGNORE_WRONG_USAGE
						);
				}
				pFile->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,10000);
				if (pFile->SendRequest()) {
					pFile->QueryInfoStatusCode(data->statusCode);
					CStringA buf;
					int i;
					UINT len = 0;
					do {
						LPSTR p = data->body.GetBuffer(len+1024);
						i = pFile->Read(p+len,1024);
						len+=i;
						data->body.ReleaseBuffer(len);
					} while (i>0);
					//--
					pFile->QueryInfo(
						HTTP_QUERY_RAW_HEADERS_CRLF,
						data->headers
						);
					pFile->Close();
				}
				session.Close();
			} else {
				data->statusCode = 0;
			}
		} catch (CInternetException *e) {
			data->statusCode = 0;
		}
	}
	if (data->message) {
		if (data->hWnd) {
			PostMessage(data->hWnd, data->message, (WPARAM)data, 0);
		}
	} else {
		delete data;
	}
	return 0;
}

void URLGetAsync(CString url, HWND hWnd, UINT message)
{
	HANDLE hThread;
	URLGetAsyncData *data = new URLGetAsyncData();
	data->hWnd = hWnd;
	data->message = message;
	data->statusCode = 0;
	data->url = url;
	if (!CreateThread(NULL,0, URLGetAsyncThread, data, 0, NULL)) {
		data->url.Empty();
		URLGetAsyncThread(data);
	}
}

URLGetAsyncData URLGetSync(CString url)
{
	URLGetAsyncData data;
	data.hWnd = 0;
	data.message = 1;
	data.url = url;
	URLGetAsyncThread(&data);
	return data;
}

CString Bin2String(CByteArray *ca)
{
	CString res;
	int k=ca->GetSize();
	for(int i=0;i<k;i++) {
		unsigned char ch=ca->GetAt(i);
		res.AppendFormat(_T("%02x"),ca->GetAt(i));
	}
	return res;
}

void String2Bin(CString str, CByteArray *res)
{
	res->RemoveAll();
	int k=str.GetLength();
	CStringA rab;
	for(int i=0;i<str.GetLength();i+=2) {
		rab = CStringA(str.Mid(i,2));
		char *p = NULL;
		unsigned long bin = strtoul(rab.GetBuffer(), &p, 16);
		res->Add(bin);
	}
}

void CommandLineToShell(CString cmd, CString &command, CString &params)
{
	cmd.Trim();
	command.Empty();
	params.Empty();
	int nArgs;
	LPWSTR *szArglist = CommandLineToArgvW(cmd, &nArgs);
	if (NULL == szArglist) {
		AfxMessageBox(_T("Wrong command: ") + cmd);
	}
	else for (int i = 0; i < nArgs; i++) {
		if (!i) {
			command = szArglist[i];
		}
		else {
			params.AppendFormat(_T("%s "), szArglist[i]);
		}
	}
	params.TrimRight();
	LocalFree(szArglist);
}

CString get_account_username()
{
	CString res = accountSettings.account.username;
	return res;
}

CString get_account_password()
{
	CString res = accountSettings.account.password;
	return res;
}

CString get_account_server()
{
	CString res = accountSettings.account.server;
	return res;
}

CString URLMask(CString url, SIPURI* sipuri, pjsua_acc_id acc)
{
	//-- replace server
	CString str;
	if (pjsua_acc_is_valid(acc) && !get_account_server().IsEmpty()) {
		str = get_account_server();
	} else {
		str = _T("localhost");
	}
	url.Replace(_T("{server}"),str);
	//--
	CTime t = CTime::GetCurrentTime();
	time_t time = t.GetTime();
	str.Format(_T("%d"), time);
	url.Replace(_T("{time}"), str);
	//--
	if (sipuri) {
		//-- replace callerid
		CString num = !sipuri->name.IsEmpty()?sipuri->name:sipuri->user;
		url.Replace(_T("{callerid}"),CString(urlencode(Utf8EncodeUcs2(num))));
		//-- replace
		url.Replace(_T("{user}"),CString(urlencode(Utf8EncodeUcs2(sipuri->user))));
		url.Replace(_T("{domain}"),CString(urlencode(Utf8EncodeUcs2(sipuri->domain))));
		url.Replace(_T("{name}"),CString(urlencode(Utf8EncodeUcs2(sipuri->name))));
		//--
	}
	return url;
}

HICON LoadImageIcon(int i)
{
	return (HICON)LoadImage(
		AfxGetInstanceHandle(),
		MAKEINTRESOURCE(i),
		IMAGE_ICON, 0, 0, LR_SHARED);
}

void msip_call_send_dtmf_info(pjsua_call_id current_call, pj_str_t digits)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	if (current_call == -1) {
		PJ_LOG(3,(THIS_FILE, "No current call"));
	} else {
		const pj_str_t SIP_INFO = pj_str("INFO");
		int call = current_call;
		pj_status_t status;
		for (int i=0; i<digits.slen; ++i) {
			char body[80];
			pjsua_msg_data msg_data_;

			pjsua_msg_data_init(&msg_data_);
			msg_data_.content_type = pj_str("application/dtmf-relay");

			pj_ansi_snprintf(body, sizeof(body),
				"Signal=%c\r\n"
				"Duration=160",
				digits.ptr[i]);
			msg_data_.msg_body = pj_str(body);

			status = pjsua_call_send_request(current_call, &SIP_INFO,
				&msg_data_);
			if (status != PJ_SUCCESS) {
				return;
			}
		}
	}
}

void msip_call_hangup_fast(pjsua_call_id call_id, pjsua_call_info *p_call_info)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	pjsua_call_info call_info;
	if (!p_call_info) {
		if (pjsua_call_get_info(call_id, &call_info) == PJ_SUCCESS) {
			p_call_info = &call_info;
		}
	}
	if (p_call_info) {
		if (p_call_info->state==PJSIP_INV_STATE_CALLING
			|| (p_call_info->role==PJSIP_ROLE_UAS && p_call_info->state==PJSIP_INV_STATE_CONNECTING)
			) {
				pjsua_call *call = &pjsua_var.calls[call_id];
				pjsip_tx_data *tdata = NULL;
				// Generate an INVITE END message
				if (pjsip_inv_end_session(call->inv, 487, NULL, &tdata) != PJ_SUCCESS || !tdata) {
					pjsip_inv_terminate(call->inv,487,PJ_TRUE);
				} else {
					// Send that END request
					if (pjsip_endpt_send_request(pjsua_get_pjsip_endpt(), tdata, -1, NULL, NULL) != PJ_SUCCESS) {
						pjsip_inv_terminate(call->inv,487,PJ_TRUE);
					}
				}
				return;
		}
	}
	pjsua_call_hangup(call_id, 0, NULL, NULL);
}

void msip_call_end(pjsua_call_id call_id)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_id);
	if (user_data && user_data->inConference) {
		pjsua_call_info call_info;
		if (pjsua_call_get_info(call_id, &call_info) == PJ_SUCCESS && call_info.state == PJSIP_INV_STATE_CONFIRMED) {
			pjsua_call_id call_ids[PJSUA_MAX_CALLS];
			unsigned count = PJSUA_MAX_CALLS;
			if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
				for (unsigned i = 0; i < count; ++i) {
					if (call_id == call_ids[i]) {
						continue;
					}
					call_user_data *curr_user_data = (call_user_data *) pjsua_call_get_user_data(call_ids[i]);
					if (curr_user_data && curr_user_data->inConference) {
						msip_call_hangup_fast(call_ids[i]);
					}
				}
			}
		}
	}
	msip_call_hangup_fast(call_id);
}

void msip_conference_join(pjsua_call_info *call_info)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_info->id);
	if (user_data && user_data->inConference) {
		pjsua_call_id call_ids[PJSUA_MAX_CALLS];
		unsigned count = PJSUA_MAX_CALLS;
		if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
			for (unsigned i = 0; i < count; ++i) {
				if (call_info->id == call_ids[i]) {
					continue;
				}
				if (!pjsua_call_has_media(call_ids[i])) {
					continue;
				}
				call_user_data *curr_user_data = (call_user_data *) pjsua_call_get_user_data(call_ids[i]);
				if (curr_user_data && curr_user_data->inConference) {
					if (call_info->conf_slot!=PJSUA_INVALID_ID) {
						pjsua_conf_port_id conf_port_id = pjsua_call_get_conf_port(call_ids[i]);
						if (conf_port_id!=PJSUA_INVALID_ID) {
							pjsua_conf_connect(call_info->conf_slot, conf_port_id);
							pjsua_conf_connect(conf_port_id, call_info->conf_slot);
						}
					}
					CWnd *hWnd = AfxGetApp()->m_pMainWnd;
					if (hWnd) {
						hWnd->PostMessage(UM_TAB_ICON_UPDATE, (WPARAM) call_ids[i], NULL);
					}
				}
			}
		}
	}
}

void msip_conference_leave(pjsua_call_info *call_info, bool hold)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	call_user_data *user_data = (call_user_data *)pjsua_call_get_user_data(call_info->id);
	if (user_data && user_data->inConference) {
		pjsua_call_id call_ids[PJSUA_MAX_CALLS];
		unsigned count = PJSUA_MAX_CALLS;
		if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
			int qty = 0;
			call_user_data *last_conf_user_data = NULL;
			for (unsigned i = 0; i < count; ++i) {
				if (call_info->id == call_ids[i]) {
					continue;
				}
				call_user_data *curr_user_data = (call_user_data *) pjsua_call_get_user_data(call_ids[i]);
				if (curr_user_data && curr_user_data->inConference) {
					last_conf_user_data = curr_user_data;
					qty++;
					if (call_info->conf_slot!=PJSUA_INVALID_ID) {
						pjsua_conf_port_id conf_port_id = pjsua_call_get_conf_port(call_ids[i]);
						if (conf_port_id!=PJSUA_INVALID_ID) {
							pjsua_conf_disconnect(call_info->conf_slot, conf_port_id);
							pjsua_conf_disconnect(conf_port_id, call_info->conf_slot);
						}
					}
					if (!hold) {
						CWnd *hWnd = AfxGetApp()->m_pMainWnd;
						if (hWnd) {
							hWnd->PostMessage(UM_TAB_ICON_UPDATE, (WPARAM) call_ids[i], NULL);
						}
					}
				}
			}
			if (qty == 1) {
				if (!hold) {
					last_conf_user_data->inConference = false;
					CWnd *hWnd = AfxGetApp()->m_pMainWnd;
					if (hWnd) {
						hWnd->PostMessage(UM_TAB_ICON_UPDATE, (WPARAM) call_info->id, NULL);
					}
				}
			}
		}
		if (!hold) {
			user_data->inConference = false;
		}
	}
}

void msip_call_hold(pjsua_call_info *call_info)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	call_user_data *user_data = (call_user_data *) pjsua_call_get_user_data(call_info->id);
	if (user_data && user_data->inConference) {
		pjsua_call_id call_ids[PJSUA_MAX_CALLS];
		unsigned count = PJSUA_MAX_CALLS;
		if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
			for (unsigned i = 0; i < count; ++i) {
				if (call_ids[i] != call_info->id ) {
					call_user_data *user_data_curr = (call_user_data *) pjsua_call_get_user_data(call_ids[i]);
					if (user_data_curr && user_data_curr->inConference) {
						pjsua_call_info call_info_curr;
						pjsua_call_get_info(call_ids[i], &call_info_curr);
						if (call_info_curr.state == PJSIP_INV_STATE_CONFIRMED) {
							if (call_info_curr.media_status != PJSUA_CALL_MEDIA_LOCAL_HOLD && call_info_curr.media_status != PJSUA_CALL_MEDIA_NONE) {
								pjsua_call_set_hold(call_info_curr.id, NULL);
							}
						}
					}
				}
			}
		}
	}
	if (call_info->state == PJSIP_INV_STATE_CONFIRMED) {
		if (call_info->media_status != PJSUA_CALL_MEDIA_LOCAL_HOLD && call_info->media_status != PJSUA_CALL_MEDIA_NONE) {
			pjsua_call_set_hold(call_info->id, NULL);
		}
	}
}

void msip_call_unhold(pjsua_call_info *call_info)
{	
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	call_user_data *user_data = NULL;
	if (call_info) {
		user_data = (call_user_data *) pjsua_call_get_user_data(call_info->id);
	}
	bool inConference = user_data && user_data->inConference;
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			if (!call_info || call_ids[i] != call_info->id ) {
				pjsua_call_info call_info_curr;
				pjsua_call_get_info(call_ids[i], &call_info_curr);
				if (call_info_curr.state == PJSIP_INV_STATE_CONFIRMED) {
					call_user_data *user_data_curr = (call_user_data *) pjsua_call_get_user_data(call_ids[i]);
					bool inConferenceCurr = user_data_curr && user_data_curr->inConference;
					if (inConference && inConferenceCurr) {
						// unhold
						if (call_info_curr.media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD || call_info_curr.media_status == PJSUA_CALL_MEDIA_NONE) {
							pjsua_call_reinvite(call_ids[i], PJSUA_CALL_UNHOLD, NULL);
						}
					} else {
						// hold
						if (call_info_curr.media_status != PJSUA_CALL_MEDIA_LOCAL_HOLD && call_info_curr.media_status != PJSUA_CALL_MEDIA_NONE) {
							pjsua_call_set_hold(call_ids[i], NULL);
						}
					}
				}
			}
		}
	}
	if (call_info && call_info->state == PJSIP_INV_STATE_CONFIRMED) {
		if (call_info->media_status == PJSUA_CALL_MEDIA_LOCAL_HOLD || call_info->media_status == PJSUA_CALL_MEDIA_NONE) {
			pjsua_call_reinvite(call_info->id, PJSUA_CALL_UNHOLD, NULL);
		}
	}
}

void msip_call_answer(pjsua_call_id call_id)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned calls_count = PJSUA_MAX_CALLS;
	unsigned calls_count_cmp = 0;
	if (pjsua_enum_calls ( call_ids, &calls_count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < calls_count; ++i) {
			pjsua_call_info call_info;
			if (pjsua_call_get_info(call_ids[i], &call_info) == PJ_SUCCESS) {
				if (call_info.role==PJSIP_ROLE_UAS && (call_info.state==PJSIP_INV_STATE_INCOMING || call_info.state==PJSIP_INV_STATE_EARLY)) {
					CWnd *hWnd = AfxGetApp()->m_pMainWnd;
					if (hWnd) {
						hWnd->PostMessage(UM_CALL_ANSWER, (WPARAM) call_ids[i], NULL);
					}
					break;
				}
			}
		}
	}
}

CStringA msip_md5sum(CString *str)
{
	CStringA md5sum;
	CStringA utf8 = Utf8EncodeUcs2(str->GetBuffer());
    DWORD cbContent= utf8.GetLength(); 
    BYTE* pbContent= (BYTE*)utf8.GetBuffer(cbContent); 
    pj_md5_context ctx;
    pj_uint8_t digest[16];
    pj_md5_init(&ctx);
	pj_md5_update(&ctx, (pj_uint8_t*)pbContent,cbContent);
	pj_md5_final(&ctx, digest);
	char *p = md5sum.GetBuffer(32);
    for (int i = 0; i<16; ++i) {
		pj_val_to_hex_digit(digest[i], p);
		p += 2;
    }
	md5sum.ReleaseBuffer();
	return md5sum;
}

CString msip_url_mask(CString url)
{
	if (accountSettings.accountId) {
		url.Replace(_T("{server}"),get_account_server());
		url.Replace(_T("{username}"),CString(urlencode(Utf8EncodeUcs2(accountSettings.account.username))));
		url.Replace(_T("{password}"),CString(urlencode(Utf8EncodeUcs2(accountSettings.account.password))));
		url.Replace(_T("{md5_password}"),CString(msip_md5sum(&accountSettings.account.password)));
	} else {
		url.Replace(_T("{server}"),_T("localhost"));
		url.Replace(_T("{username}"),_T(""));
		url.Replace(_T("{password}"),_T(""));
		url.Replace(_T("{md5_password}"),_T(""));
	}
	return url;
}

//void msip_audio_output_set_volume(int val, bool mute)
//{
//	if (mute) {
//		val = 0;
//	} else {
//		pj_status_t status = 
//			pjsua_snd_set_setting(
//			PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
//			&val, PJ_TRUE);
//		if (status == PJ_SUCCESS) {
//			val = 100;
//		}
//	}
//	pjsua_conf_adjust_tx_level(0, (float)val/100);
//}

void msip_audio_conf_set_volume(int val, bool mute)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	if (mute) {
		val = 0;
	}
	pjsua_call_id call_ids[PJSUA_MAX_CALLS];
	unsigned count = PJSUA_MAX_CALLS;
	if (pjsua_enum_calls ( call_ids, &count)==PJ_SUCCESS)  {
		for (unsigned i = 0; i < count; ++i) {
			pjsua_conf_port_id conf_port_id = pjsua_call_get_conf_port(call_ids[i]);
			if (conf_port_id!=PJSUA_INVALID_ID) {
				pjsua_conf_adjust_rx_level(conf_port_id, (float)val/100);
			}
		}
	}
}

void msip_audio_input_set_volume(int val, bool mute)
{
	if (pjsua_var.state!=PJSUA_STATE_RUNNING) {
		return;
	}
	if (mute) {
		val = 0;
	} else {
		pj_status_t status = -1;
		if (!accountSettings.swLevelAdjustment) {
			int valHW;
			if (accountSettings.micAmplification) {
				valHW = val>=50 ? 100 : val*2;
			} else {
				valHW = val;
			}
			status = 
				pjsua_snd_set_setting(
				PJMEDIA_AUD_DEV_CAP_INPUT_VOLUME_SETTING,
				&valHW, PJ_TRUE);
		}
		if (status==PJ_SUCCESS) {
			if (accountSettings.micAmplification && val>50) {
				val = 100 + pow((float)val-50,1.68f);
			} else {
				val = 100;
			}
		} else {
			if (accountSettings.micAmplification) {
				if (val>50) {
					val = 50+pow((float)val-50,1.7f);
				}
			}		
		}
	}
	pjsua_conf_adjust_rx_level(0, (float)val/100);
}
