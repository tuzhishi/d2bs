#ifndef __D2BS_H__
#define __D2BS_H__

#pragma once

#define XP_WIN
#define D2BS_VERSION "2.x"

#include <map>
#include <windows.h>

#include "CollisionMap.h"

struct Variables;

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

#define PRIVATE_UNIT		1
#define PRIVATE_ITEM		3

struct Private { DWORD dwPrivateType; };

struct Module
{	
	union {
		HMODULE hModule;
		DWORD dwBaseAddress;
	};
	DWORD _1;
	char szPath[MAX_PATH];
};

struct Variables
{
	int		nChickenHP;
	int		nChickenMP;
	DWORD	dwInjectTime;
	DWORD	dwGameTime;
	BOOL	bDontCatchNextMsg;
	BOOL	bClickAction;
	BOOL	bNeedShutdown;
	BOOL	bUseGamePrint;
	BOOL	bShutdownFromDllMain;
	BOOL	bChangedAct;
	BOOL	bGameLoopEntered;

	DWORD	dwMaxGameTime;
	DWORD	dwGameTimeout;
	BOOL	bBlockMinimize;
	BOOL	bQuitOnError;
	BOOL	bQuitOnHostile;
	BOOL	bStartAtMenu;
	BOOL	bActive;
	BOOL	bBlockKeys;
	BOOL	bBlockMouse;
	BOOL	bDisableCache;
	int		dwMemUsage;

	Module* pModule;
	char	szPath[_MAX_PATH];
	char	szScriptPath[_MAX_PATH];

	WNDPROC oldWNDPROC;
	HHOOK hMouseHook;
	HHOOK hKeybHook;

	UINT_PTR uTimer;
	long SectionCount;

	CCollisionMap cCollisionMap;
	std::map<unsigned __int32, CellFile*> mCachedCellFiles;

	CRITICAL_SECTION	cRoomSection;
	CRITICAL_SECTION	cMiscSection;
	CRITICAL_SECTION	cScreenhookSection;
	CRITICAL_SECTION	cPrintSection;
	CRITICAL_SECTION	cTextHookSection;
	CRITICAL_SECTION	cImageHookSection;
	CRITICAL_SECTION	cBoxHookSection;
	CRITICAL_SECTION	cFrameHookSection;
	CRITICAL_SECTION	cLineHookSection;
	CRITICAL_SECTION	cFlushCacheSection;
	CRITICAL_SECTION	cConsoleSection;
	CRITICAL_SECTION	cGameLoopSection;

	DWORD dwSelectedUnitId;
	DWORD dwSelectedUnitType;
};

extern Variables Vars;

BOOL Startup(void);
void Shutdown(void);

#endif
