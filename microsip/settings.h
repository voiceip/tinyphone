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

#pragma once

#include "global.h"

struct Account {
	CString label;
	CString server;
	CString proxy;
	CString username;
	CString domain;
	int port;
	CString authID;
	CString password;
	bool rememberPassword;
	CString displayName;
	CString voicemailNumber;
	CString srtp;
	CString transport;
	CString publicAddr;
	bool publish;
	bool ice;
	bool allowRewrite;
	bool disableSessionTimer;
	bool operator==(const Account& a) const	{
		if (label == a.label
			&& server == a.server
			&& proxy == a.proxy
			&& username == a.username
			&& domain == a.domain
			&& authID == a.authID
			&& password == a.password
			&& displayName == a.displayName
			&& srtp == a.srtp
			&& transport == a.transport
			&& publish == a.publish
			&& ice == a.ice
			&& allowRewrite == a.allowRewrite
			)
			return true;
		return false;
	};
	bool operator!=(const Account& a) const {
		return !(*this == a);
	}
	void operator=(const Account& a)
	{
		label = a.label;
		server = a.server;
		proxy = a.proxy;
		username = a.username;
		domain = a.domain;
		authID = a.authID;
		password = a.password;
		displayName = a.displayName;
		srtp = a.srtp;
		transport = a.transport;
		publish = a.publish;
		ice = a.ice;
		allowRewrite = a.allowRewrite;
	};
	Account() : port(0)
		, rememberPassword(false)
		, publish(false)
		, ice(false)
		, allowRewrite(false)
		, disableSessionTimer(false)
	{}
};

struct AccountSettings {

	int accountId;
	Account account;
	bool singleMode;
	CString ringingSound;
	CString audioRingDevice;
	CString audioOutputDevice;
	CString audioInputDevice;
	bool micAmplification;
	bool swLevelAdjustment;
	CString audioCodecs;
	bool vad;
	bool ec;
	bool forceCodec;
	CString videoCaptureDevice;
	CString videoCodec;
	bool videoH264;
	bool videoH263;
	bool videoVP8;
	int videoBitrate;
	bool rport;
	int sourcePort;
	int rtpPortMin;
	int rtpPortMax;
	bool dnsSrv;
	CString stun;
	bool enableSTUN;
	int DTMFMethod;
	bool AA;
	bool DND;
	CString autoAnswer;
	CString denyIncoming;
	CString usersDirectory;
	bool enableMediaButtons;
	bool localDTMF;
	bool enableLocalAccount;
	bool crashReport;
	bool enableLog;
	bool bringToFrontOnIncoming;
	bool randomAnswerBox;
	CString userAgent;
	CString portKnockerHost;
	CString portKnockerPorts;

	CString lastCallNumber;
	bool lastCallHasVideo;

	CString updatesInterval;

	int activeTab;
	bool alwaysOnTop;

	int mainX;
	int mainY;
	int mainW;
	int mainH;
	bool noResize;

	int messagesX;
	int messagesY;
	int messagesW;
	int messagesH;

	int ringinX;
	int ringinY;

	int callsWidth0;
	int callsWidth1;
	int callsWidth2;
	int callsWidth3;
	int callsWidth4;

	int contactsWidth0;
	int contactsWidth1;

	int volumeOutput;
	int volumeInput;
	
	CString iniFile;
	CString logFile;
	CString exeFile;
	CString pathRoaming;
	CString pathLocal;
	CString pathExe;

	int checkUpdatesTime;

	bool hidden;
	bool silent;
	
	int autoHangUpTime;
	int maxConcurrentCalls;

	CString cmdCallStart;
	CString cmdCallEnd;
	CString cmdIncomingCall;
	CString cmdCallAnswer;
	bool enableShortcuts;
	bool shortcutsBottom;
	AccountSettings();
	void Init();
	bool AccountLoad(int id, Account *account);
	void AccountSave(int id, Account *account);
	void AccountDelete(int id);
	void SettingsSave();
};

extern AccountSettings accountSettings;
extern bool firstRun;
extern bool pj_ready;
extern CTime startTime;

CString ShortcutEncode(Shortcut *pShortcut);
void ShortcutDecode(CString str, Shortcut *pShortcut);
void ShortcutsLoad();
void ShortcutsSave();
extern CArray<Shortcut,Shortcut> shortcuts;
