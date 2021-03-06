#include <io.h>
#include <errno.h>
#include <ctime>
#include <cmath>

#include "Constants.h"
#include "Helpers.h"
#include "D2Skills.h"
#include "D2Intercepts.h"
#include "D2BS.h"

#include "debugnew/debug_new.h"

void Log(char* szFormat, ...)
{
	va_list vaArgs;

	va_start(vaArgs, szFormat);
	int len = _vscprintf(szFormat, vaArgs);
	char* szString = new char[len+1];
	vsprintf_s(szString, len+1, szFormat, vaArgs);
	va_end(vaArgs);

	time_t tTime;
	time(&tTime);
	char szTime[128] = "";
	struct tm time;
	localtime_s(&time, &tTime);
	strftime(szTime, sizeof(szTime), "%x %X", &time);

	fprintf(stderr, "[%s] %s\r\n", szTime, szString);
	fflush(stderr);
	delete[] szString;
}

// NOTE TO CALLERS: szTmp must be a PRE-INITIALIZED string.
const char* GetUnitName(UnitAny* pUnit, CHAR* szTmp, size_t bufSize)
{
	if(!pUnit)
	{
		strcpy_s(szTmp, bufSize, "Unknown");
		return szTmp;
	}
	if(pUnit->dwType == UNIT_MONSTER) {
		wchar_t* wName = D2CLIENT_GetUnitName(pUnit);
		WideCharToMultiByte(CP_ACP, 0, wName, -1, szTmp, bufSize, 0, 0);
		return szTmp;
	}
	if(pUnit->dwType == UNIT_PLAYER && pUnit->pPlayerData)
	{
		//	return pUnit->pPlayerData->szName;
		strcpy_s(szTmp, bufSize, pUnit->pPlayerData->szName);
		return szTmp;
	}
	if(pUnit->dwType == UNIT_ITEM)
	{
		wchar_t wBuffer[8192] = {NULL};
		D2CLIENT_GetItemName(pUnit, wBuffer, sizeof(wBuffer));
		char* szBuffer = UnicodeToAnsi(wBuffer);
		if(strchr(szBuffer, '\n')) {
            *strchr(szBuffer,'\n') = 0x00;
		}

		strcpy_s(szTmp, bufSize, szBuffer);
		delete[] szBuffer;
		return szTmp;
	}
	if(pUnit->dwType == UNIT_OBJECT || pUnit->dwType == UNIT_TILE)
	{
		if(pUnit->pObjectData && pUnit->pObjectData->pTxt)
		{
			strcpy_s(szTmp, bufSize, pUnit->pObjectData->pTxt->szName);
			return szTmp;
		}
	}
	strcpy_s(szTmp, bufSize, "Unknown");
	return szTmp;
}

WORD GetUnitX(UnitAny* pUnit)
{
	if(!pUnit)
		return NULL;

	if(pUnit->dwType == UNIT_ITEM || pUnit->dwType == UNIT_OBJECT || pUnit->dwType == UNIT_TILE)
		return pUnit->pObjectPath ? (WORD)pUnit->pObjectPath->dwPosX : NULL;
	else
		return pUnit->pPath ? (WORD)pUnit->pPath->xPos : NULL;
	//return NULL;
}

WORD GetUnitY(UnitAny* pUnit)
{
	if(!pUnit)
		return NULL;

	if(pUnit->dwType == UNIT_ITEM || pUnit->dwType == UNIT_OBJECT || pUnit->dwType == UNIT_TILE)
		return pUnit->pObjectPath ? (WORD)pUnit->pObjectPath->dwPosY : NULL;
	else
		return pUnit->pPath ? (WORD)pUnit->pPath->yPos : NULL;
	//return NULL;
}

bool InArea(int x, int y, int x2, int y2, int sizex, int sizey) {
	return !!(x >= x2 && x < x2+sizex && y >= y2 && y < y2+sizey);
}

UnitAny* FindItemByPosition(DWORD x, DWORD y, DWORD Location) {
	for(UnitAny* pItem = D2COMMON_GetItemFromInventory(D2CLIENT_GetPlayerUnit()->pInventory); pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem)) {
		if((DWORD)GetItemLocation(pItem) == Location && InArea(x,y,pItem->pObjectPath->dwPosX,pItem->pObjectPath->dwPosY,D2COMMON_GetItemText(pItem->dwTxtFileNo)->xSize,D2COMMON_GetItemText(pItem->dwTxtFileNo)->ySize))
			return pItem;
	}
	return NULL;
}

VOID SelectInventoryItem(DWORD x, DWORD y, DWORD dwLocation)
{
	*(DWORD*)&p_D2CLIENT_SelectedInvItem = (DWORD)FindItemByPosition(x, y, dwLocation);
}

ClientGameState ClientState(VOID)
{
	if(*p_D2CLIENT_PlayerUnit && !(*p_D2WIN_FirstControl))
		return ClientStateInGame;
	else if(!(*p_D2CLIENT_PlayerUnit) && *p_D2WIN_FirstControl)
		return ClientStateMenu;
	else
		return ClientStateBusy;
}

BOOL GameReady(VOID)
{
	return !!(ClientState() == ClientStateInGame &&
			*p_D2CLIENT_PlayerUnit &&
			(*p_D2CLIENT_PlayerUnit)->pInventory &&
			(*p_D2CLIENT_PlayerUnit)->pPath &&
			(*p_D2CLIENT_PlayerUnit)->pPath->xPos &&
			(*p_D2CLIENT_PlayerUnit)->pPath->pRoom1 &&
			(*p_D2CLIENT_PlayerUnit)->pPath->pRoom1->pRoom2 &&
			(*p_D2CLIENT_PlayerUnit)->pPath->pRoom1->pRoom2->pLevel &&
			(*p_D2CLIENT_PlayerUnit)->pPath->pRoom1->pRoom2->pLevel->dwLevelNo);
}

DWORD GetPlayerArea(VOID)
{
	return (GameReady() ? (*p_D2CLIENT_PlayerUnit)->pPath->pRoom1->pRoom2->pLevel->dwLevelNo : NULL);
}

Level* GetLevel(DWORD dwLevelNo)
{
	for(Level* pLevel = D2CLIENT_GetPlayerUnit()->pAct->pMisc->pLevelFirst; pLevel; pLevel = pLevel->pNextLevel)
		if(pLevel->dwLevelNo == dwLevelNo)
			return pLevel;

	return D2COMMON_GetLevel(D2CLIENT_GetPlayerUnit()->pAct->pMisc, dwLevelNo);
}

// TODO: make this use SIZE for clarity
POINT CalculateTextLen(CHAR* szwText, INT Font)
{
	POINT ret = {0,0};
	wchar_t* Buffer = AnsiToUnicode(szwText);

	DWORD dwWidth, dwFileNo;
	DWORD dwOldSize = D2WIN_SetTextSize(Font);
	ret.y = D2WIN_GetTextWidthFileNo(Buffer, &dwWidth, &dwFileNo);
	ret.x = dwWidth;
	D2WIN_SetTextSize(dwOldSize);

	delete[] Buffer;
	return ret;
}

INT GetSkill(WORD wSkillId)
{
	if(!D2CLIENT_GetPlayerUnit()) return 0;

	for(Skill* pSkill = D2CLIENT_GetPlayerUnit()->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill)
		if(pSkill->pSkillInfo->wSkillId == wSkillId)
			return D2COMMON_GetSkillLevel(D2CLIENT_GetPlayerUnit(), pSkill, TRUE);

	return 0;
}

BOOL SetSkill(WORD wSkillId, BOOL bLeft)
{
	if(!GameReady()) 
		return FALSE;

	if(!GetSkill(wSkillId))
		return FALSE;

	BYTE aPacket[9];

	aPacket[0] = 0x3C;
	*(WORD*)&aPacket[1] = wSkillId;
	aPacket[3] = 0;
	aPacket[4] = (bLeft) ? 0x80 : 0;
	*(DWORD*)&aPacket[5] = 0xFFFFFFFF;

	D2NET_SendPacket(9, 1, aPacket);

	UnitAny* Me = D2CLIENT_GetPlayerUnit();

	if(!bLeft) {
		if(!GameReady())
			return FALSE;
		while(Me->pInfo->pRightSkill->pSkillInfo->wSkillId != wSkillId)
		{
			Sleep(100);
			if(!GameReady())
				return FALSE;
		}
	}
	else {	
		if(!GameReady())
			return FALSE;
		
		while(Me->pInfo->pLeftSkill->pSkillInfo->wSkillId != wSkillId)
		{
			Sleep(100);
			if(!GameReady())
				return FALSE;
		}
	}

	return TRUE;
}

// Compare the skillname to the Game_Skills struct to find the right skill ID to return
WORD GetSkillByName(CHAR* skillname)
{
	for(int i = 0; i < 216; i++)
		if(_stricmp(Game_Skills[i].name, skillname) == 0)
			return Game_Skills[i].skillID;
	return (WORD)-1;
}

CHAR* GetSkillByID(WORD id)
{
	for(int i = 0; i < 216; i++)
		if(id == Game_Skills[i].skillID)
			return Game_Skills[i].name;
	return NULL;
}

VOID ScreenSizeCheck(POINT* pPoint)
{
	if(D2GFX_GetScreenSize() == 0)
	{
		pPoint->x -= 800;
		pPoint->y -= 600;

		pPoint->x += 640;
		pPoint->y += 400;
	}
}

VOID SendMouseClick(INT x, INT y, INT clicktype)
{
	LPARAM lp = x + (y << 16);
	switch(clicktype)
	{
	case 0:
		SendMessage(D2WIN_GetHwnd(), WM_LBUTTONDOWN, 0, lp);
		break;
	case 1:
		SendMessage(D2WIN_GetHwnd(), WM_LBUTTONUP, 0, lp);
		break;
	case 2:
		SendMessage(D2WIN_GetHwnd(), WM_RBUTTONDOWN, 0, lp);
		break;
	case 3:
		SendMessage(D2WIN_GetHwnd(), WM_RBUTTONUP, 0, lp);
		break;
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_InitAutomapLayer_STUB(DWORD nLayerNo)
{
	__asm 
	{
		push eax;
		mov eax, ecx;
		call D2CLIENT_InitAutomapLayer_I;
		pop eax;
		ret;
	}
}

AutomapLayer* InitAutomapLayer(DWORD levelno)
{
	AutomapLayer2 *pLayer = D2COMMON_GetLayer(levelno);
	return D2CLIENT_InitAutomapLayer(pLayer->nLayerNo);
}

VOID MapToScreenCoords(POINT* pPos)
{
	D2COMMON_MapToAbsScreen(&pPos->x, &pPos->y);
	pPos->x -= D2CLIENT_GetMouseXOffset();
	pPos->y -= D2CLIENT_GetMouseYOffset();
}

VOID ScreenToAutomap(POINT *ptPos, int x, int y)
{
	ptPos->x = ((x - y)/2/(*(INT*)p_D2CLIENT_Divisor))-(*p_D2CLIENT_Offset).x+8;
	ptPos->y = ((x + y)/4/(*(INT*)p_D2CLIENT_Divisor))-(*p_D2CLIENT_Offset).y-8;
	if(D2CLIENT_GetAutomapSize())
	{
		--ptPos->x;
		ptPos->y += 5;
	}
}

BOOL IsTownLevel(INT nLevel)
{
	if(nLevel == MAP_A1_ROGUE_ENCAMPMENT ||
		nLevel == MAP_A2_LUT_GHOLEIN || 
		nLevel == MAP_A3_KURAST_DOCKS ||
		nLevel == MAP_A4_THE_PANDEMONIUM_FORTRESS ||
		nLevel == MAP_A5_HARROGATH)
		return TRUE;
	return FALSE;
}


VOID myDrawText(char* szwText, int x, int y, int color, int font) 
{
	wchar_t* text = AnsiToUnicode(szwText);

	DWORD dwOld = D2WIN_SetTextSize(font);
	D2WIN_DrawText(text, x, y, color, 0);
	D2WIN_SetTextSize(dwOld);

	delete[] text;
}


VOID myDrawCenterText(char* szText, int x, int y, int color, int font, int div) 
{
	DWORD dwWidth = NULL, dwFileNo = NULL, dwOldSize = NULL;
	wchar_t* Buffer = AnsiToUnicode(szText);

	dwOldSize = D2WIN_SetTextSize(font);
	D2WIN_GetTextWidthFileNo(Buffer, &dwWidth, &dwFileNo);
	D2WIN_SetTextSize(dwOldSize);
	myDrawText(szText,x-(dwWidth >> div),y,color,font);

	delete[] Buffer;
}

VOID D2CLIENT_Interact(UnitAny* pUnit, DWORD dwMoveType) {
	
	if(!pUnit)
		return;

	if(!D2CLIENT_FindUnit(pUnit->dwUnitId,pUnit->dwType))
		return;

	InteractStruct pInteract = {
		dwMoveType,
		D2CLIENT_GetPlayerUnit(),
		pUnit,
		GetUnitX(pUnit),
		GetUnitY(pUnit),
		0, 0
	};

	D2CLIENT_Interact_(&pInteract);
}

typedef VOID (*fnClickEntry) (VOID);

BOOL ClickNPCMenu(DWORD NPCClassId, DWORD MenuId)
{
	NPCMenu* pMenu = (NPCMenu*)p_D2CLIENT_NPCMenu;
	fnClickEntry pClick = (fnClickEntry) NULL;

	for(UINT i = 0; i < *p_D2CLIENT_NPCMenuAmount; i++)
	{
		if(pMenu->dwNPCClassId == NPCClassId)
		{
			if(pMenu->wEntryId1 == MenuId)
			{
				pClick = (fnClickEntry)pMenu->dwEntryFunc1;
					if(pClick)
						pClick();
					else return FALSE;
				return TRUE;
			}
			else if(pMenu->wEntryId2 == MenuId)
			{
				pClick = (fnClickEntry)pMenu->dwEntryFunc2;
					if(pClick)
						pClick();
					else return FALSE;
				return TRUE;
			}
			else if(pMenu->wEntryId3 == MenuId)
			{
				pClick = (fnClickEntry)pMenu->dwEntryFunc3;
					if(pClick)
						pClick();
					else return FALSE;
				return TRUE;
			}
			else if(pMenu->wEntryId4 == MenuId)
			{
				pClick = (fnClickEntry)pMenu->dwEntryFunc4;
					if(pClick)
						pClick();
					else return FALSE;
				return TRUE;
			}
		}
		pMenu = (NPCMenu*) ((DWORD)pMenu + sizeof(NPCMenu));
	}

	return FALSE;
}

/*
Thanks to 99Elite for posting the list at ladderhall.com!

Layout of sgptDataTables

+0     :  ptr to playerclass.bin
+4     :  num records in playerclass.bin
+8     :  ptr to bodylocs.bin
+c     :  num records in bodylocs.bin
+10   :  ptr to storepage.bin
+14   :  num records in storepage.bin
+18   :  ptr to elemtypes.bin
+1c   :  num records in elemtypes.bin
+20   :  ptr to hitclass.bin
+24   :  num records in hitclass.bin
+28   :  ptr to monmode.bin
+2c   :  num records in monmode.bin
+30   :  ptr to plrmode.bin
+34   :  num records in plrmode.bin
+38   :  ptr to skillcalc.bin
+3c   :  num records in skillcalc.bin
+44   :  ptr to skillscode.bin
+48   :  num records in skillscode.bin
+50   :  ptr to skilldesccode.bin
+54   :  num records in skilldesccode.bin
+58   :  ptr to misscalc.bin
+5c   :  num records in misccalc.bin
+64   :  ptr to misscode.bin
+68   :  numrecords in misccode.bin
+70   :  ptr to events.bin
+74   :  numrecords in events.bin
+88   :  ptr to monai.bin
+8c    : num records in monai.bin
+A4   :  ptr to properties.bin
+Ac   :  num records in properties.bin
+b4   :  ptr to hiredesc.bin
+b8   :  num records in hiredesc.bin
+194 :  ptr to sounds.bin
+19c :  num records in sounds.bin
+1a0 :  ptr to heirling.bin
+1a4 :  num records in heirling.bin
+9a8 :  ptr to npc.txt
+9ac :  num records in npc.txt
+9b0 :  ptr to colors.bin
+9b4 :  num records in colors.bin
+a78 :  ptr to monstats.bin
+a80 :  num records in monstats.bin
+a84 :  ptr to monsounds.bin
+a8c :  num records in monsunds.bin
+a90 :  ptr to monstats2.bin
+a98 :  num records in monstats2.bin
+a9c :  ptr to monplace.bin
+aa0 :  numrecirds in monplace.bin
+aa8 :  ptr to monprest.bin
+aac :  ptr to monprest.bin (again) // check this
+ad4 :  ptr to superuniques.bin
+adc :  num records in superuniques.bin
+b64 :  missiles.bin
+b6c :  num records in missiles.bin
+b70 :  ptr to monlvl.bin
+b74 :  num records in monlvl.bin
+b78 :  ptr to monseq.bin
+b7c :  num records in monseq.bin
+b94 :  num records in skillsdesc.bin
+b98 :  ptr to skills.bin
+ba0 :  num records in skills table
+bbc :  ptr to overlay.bin
+bc0 :  num records in overlay.bin
+bc4 :  charstats table
+bcc :  ptr to itemstatcost table
+bd4 :  num records in itemstatcost
+be0 :  ptr to monequip.bin
+be4 :  num records in monequip.bin
+bf8  :  ptr to itemtypes.bin
+bfc  :  num records in itemtypes.bin
+c0c :  ptr to sets.bin
+c10 :  num records in sets.bin
+c18 :  ptr to setitems.bin
+c1c :  num records in setitems.bin
+c24 :  ptr to uniqueitems.bin
+c28 :  num records in uniqueitems.bin
+c30 :  ptr to monprop.bin
+c34 :  num records in monprop.bin
+c3c :  ptr to montype.bin
+c40 :  num records in montype.bin
+c50 :  ptr to monumod.bin
+c54 :  num records in monummod.bin
+c58 :  ptr to levels.bin
+c5c :  num records in levels.bin
+c60 :  ptr to lvldefs.bin
+c64 :  ptr to lvlPrest.bin
+c68 :  num records in lvlPrest.bin
+cb8 :  ptr to chartemplate.bin
+cbc :  num records in chartemplate.bin
+cc0 :  ptr to arena.bin
+cc4 :  ptr to lvlTypes.bin
+cd0 :  num records in lvlTypes.bin
+cd4 :  ptr to lvlwarp.bin
+cd8 :  num records in lvlwarp.bin
+cdc :  ptr to lvlmaze.bin
+ce0 :  num records in lvlmaze.bin
+ce4 :  ptr to levelsub.bin
+ce8 :  num records in levelsub.bin
+d04 :  ptr to cubemain.bin
+d08 :  num records in cubemain.bin
*/

INT GetItemLocation(UnitAny *pItem)
{
	if(!pItem || !pItem->pItemData)
		return -1;

	switch(pItem->pItemData->ItemLocation)
	{
		case STORAGE_INVENTORY:
			return STORAGE_INVENTORY;

		case STORAGE_CUBE:
			return STORAGE_CUBE;

		case STORAGE_STASH:
			return STORAGE_STASH;

		case STORAGE_NULL:
			switch(pItem->pItemData->NodePage)
			{
				case NODEPAGE_EQUIP:
					return STORAGE_EQUIP;

				case NODEPAGE_BELTSLOTS:
					return STORAGE_BELT;
			}
	}

	return STORAGE_NULL;
}

BYTE CalcPercent(DWORD dwVal, DWORD dwMaxVal, BYTE iMin)
{
	if(dwVal == 0 || dwMaxVal == 0)
		return 0;

	BYTE iRes = (BYTE)((double)dwVal / (double)dwMaxVal * 100.0);
	if (iRes > 100)
		iRes = 100;

	return max(iRes, iMin);
}


DWORD GetTileLevelNo(Room2* lpRoom2, DWORD dwTileNo)
{
	for(RoomTile* pRoomTile = lpRoom2->pRoomTiles; pRoomTile; pRoomTile = pRoomTile->pNext)
	{
		if(*(pRoomTile->nNum) == dwTileNo)
			return pRoomTile->pRoom2->pLevel->dwLevelNo;
	}

	return NULL;
}

UnitAny* D2CLIENT_FindUnit(DWORD dwId, DWORD dwType)
{
	UnitAny* pUnit = D2CLIENT_FindServerSideUnit(dwId, dwType);
	return pUnit ? pUnit : D2CLIENT_FindClientSideUnit(dwId, dwType);
}

// TODO: Rewrite this and split it into two functions

CellFile* LoadCellFile(CHAR* lpszPath, DWORD bMPQ)
{
	// AutoDetect the Cell File
	if(bMPQ == 3)
	{
		// Check in our directory first

		HANDLE hFile = OpenFileRead(lpszPath);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return LoadCellFile(lpszPath, FALSE);
		}
		else
		{
			return LoadCellFile(lpszPath, TRUE);
		}

		//return NULL;
	}

	if(bMPQ == TRUE)
	{
		return (CellFile*)D2CLIENT_LoadUIImage_ASM(lpszPath);
	}
	else if(bMPQ == FALSE)
	{
		// see if the file exists first
		if(!(_access(lpszPath, 0) != 0 && errno == ENOENT))
			return myInitCellFile((CellFile*)LoadBmpCellFile(lpszPath));
	}

	return NULL;
}

int RemoveColorSpecs(wchar_t *lpwsz)
{
	if (lpwsz == NULL)
		return 0;

	const int LEN = (int)wcslen(lpwsz);
	wchar_t* p = new wchar_t[LEN + 1];
	::memset(p, 0, sizeof(wchar_t) * (LEN + 1));

	int nIndex = 0;
	for(int i = 0; i < LEN; i++)
	{
		if (lpwsz[i] == 0xF8F5 || lpwsz[i] == 0xFF)
			i += 2;
		else
			p[nIndex++] = lpwsz[i];
	}

	wcscpy_s(lpwsz, sizeof(lpwsz), p);
	return (int)wcslen(lpwsz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Original Sourcecode from Stings or Mousepad's
///////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: make this use SIZE instead of POINT for clarity
POINT GetScreenSize()
{
	POINT sizes[] = { {640, 480}, {800, 600}, {800, 600} };
	return sizes[D2GFX_GetScreenSize()];
}

INT D2GetScreenSizeX()
{
	return GetScreenSize().x;
}

INT D2GetScreenSizeY()
{
	return GetScreenSize().y;
}

VOID myDrawAutomapCell(CellFile *cellfile, int xpos, int ypos, BYTE col)
{
	if(!cellfile)return;
	CellContext ct;
	memset(&ct, 0, sizeof(ct));
	ct.pCellFile = cellfile;

	xpos -= (cellfile->cells[0]->width/2);
	ypos += (cellfile->cells[0]->height/2);

	int xpos2 = xpos - cellfile->cells[0]->xoffs, ypos2 = ypos - cellfile->cells[0]->yoffs;
	if ((xpos2 >= D2GetScreenSizeX()) || ((xpos2 + (int)cellfile->cells[0]->width) <= 0) || (ypos2 >= D2GetScreenSizeY()) || ((ypos2 + (int)cellfile->cells[0]->height) <= 0)) return;

	static BYTE coltab[2][256];//, tabno = 0, lastcol = 0;
	if (!coltab[0][1]) for (int k = 0; k < 255; k++) coltab[0][k] = coltab[1][k] = (BYTE)k;
	cellfile->mylastcol = coltab[cellfile->mytabno ^= (col != cellfile->mylastcol)][255] = col;

	D2GFX_DrawAutomapCell2(&ct, xpos, ypos, (DWORD)-1, 5, coltab[cellfile->mytabno]);
}

DWORD ReadFile(HANDLE hFile, void *buf, DWORD len)
//NOTE :- validates len bytes of buf
{
	DWORD numdone = 0;
	ReadFile(hFile, buf, len, &numdone, NULL);
	return numdone;
}
VOID *memcpy2(void *dest, const void *src, size_t count)
{
	return (char *)memcpy(dest, src, count)+count;
}

HANDLE OpenFileRead(char *filename)
{
	return CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

BYTE *AllocReadFile(char *filename)
{
	HANDLE hFile = OpenFileRead(filename);
	int filesize = GetFileSize(hFile, 0);
	if (filesize <= 0) return 0;
	BYTE *buf = new BYTE[filesize];
	ReadFile(hFile, buf, filesize);
	CloseHandle(hFile);
	return buf;
}

CellFile *LoadBmpCellFile(BYTE *buf1, int width, int height)
{
	BYTE *buf2 = new BYTE[(width*height*2)+height], *dest = buf2;

	for (int i = 0; i < height; i++) {
		BYTE *src = buf1+(i*((width+3)&-4)), *limit = src+width;
		while (src < limit) {
			BYTE *start = src, *limit2 = min(limit, src+0x7f), trans = !*src;
			do src++; while ((trans == (BYTE)!*src) && (src < limit2));
			if (!trans || (src < limit)) *dest++ = (BYTE)((trans?0x80:0)+(src-start));
			if (!trans) while (start < src) *dest++ = *start++;
		}
		*dest++ = 0x80;
	}

	static DWORD dc6head[] = {6, 1, 0, 0xeeeeeeee, 1, 1, 0x1c,  0, (DWORD)-1, (DWORD)-1, 0, 0, 0, (DWORD)-1, (DWORD)-1};
	dc6head[8] = width;
	dc6head[9] = height;
	dc6head[14] = dest - buf2;
	dc6head[13] = sizeof(dc6head)+(dc6head[14])+3;
	BYTE *ret = new BYTE[dc6head[13]];
	memset(memcpy2(memcpy2(ret, dc6head, sizeof(dc6head)), buf2, dc6head[14]), 0xee, 3);
	delete[] buf2;
	
	return (CellFile *)ret;
}

CellFile *LoadBmpCellFile(char *filename)
{
	BYTE *ret = 0;
		
	BYTE *buf1 = AllocReadFile(filename);
	BITMAPFILEHEADER *bmphead1 = (BITMAPFILEHEADER *)buf1;
	BITMAPINFOHEADER *bmphead2 = (BITMAPINFOHEADER *)(buf1+sizeof(BITMAPFILEHEADER));
	if (buf1 && (bmphead1->bfType == 'MB') && (bmphead2->biBitCount == 8) && (bmphead2->biCompression == BI_RGB)) {
		ret = (BYTE *)LoadBmpCellFile(buf1+bmphead1->bfOffBits, bmphead2->biWidth, bmphead2->biHeight);
	}
	delete[] buf1;

	return (CellFile *)ret;
}

CellFile *myInitCellFile(CellFile *cf)
{
	if(cf)
		D2CMP_InitCellFile(cf, &cf, "?", 0, (DWORD)-1, "?");
	return cf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End of Sting's or Mousepad's
///////////////////////////////////////////////////////////////////////////////////////////////////



DWORD __declspec(naked) __fastcall D2CLIENT_GetUnitName_STUB(DWORD UnitAny)
{
	__asm
	{
		mov eax, ecx
		jmp D2CLIENT_GetUnitName_I
	}
}

DWORD __declspec(naked) __fastcall D2CLIENT_GetUIVar_STUB(DWORD varno)
{
	__asm 
	{
		mov eax, ecx;
		jmp D2CLIENT_GetUiVar_I;
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_SetSelectedUnit_STUB(DWORD UnitAny)
{
	__asm
	{
		mov eax, ecx
		jmp D2CLIENT_SetSelectedUnit_I
	}
}
DWORD __declspec(naked) __fastcall D2CLIENT_LoadUIImage_ASM(char* Path) 
{
	__asm {
		mov eax, ecx
		push 0
		call D2CLIENT_LoadUIImage_I
		retn
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_Interact_ASM(DWORD Struct)
{
	__asm {
		mov esi, ecx
		jmp D2CLIENT_Interact_I
	}
}

/*
DWORD __declspec(naked) __fastcall FindUnit_STUB(DWORD unitid, DWORD unittype)
{
	__asm
	{
		pop eax;
		push edx;
		push eax;

		shl edx, 9;
		mov eax, D2CLIENT_GetUnitFromId_I;
		add edx, eax;
		mov eax, ecx;
		and eax, 0x7F;
		jmp D2CLIENT_GetUnitFromId_II;
	}
}*/

DWORD __declspec(naked) __fastcall D2CLIENT_clickParty_ASM(DWORD RosterUnit, DWORD Mode)
{
	__asm
	{
		mov eax, ecx
		jmp D2CLIENT_clickParty_I
	}
}

// obsoleted - use D2CLIENT_ShopAction instead
// This isn't finished anyway!
VOID __declspec(naked) __fastcall D2CLIENT_ClickShopItem_ASM(DWORD x, DWORD y, DWORD BuyOrSell)
{
	__asm
	{
		mov esi, ecx
		mov edi, edx
		pop eax // Save return address to eax
		pop ecx // Buy or sell to ecx
		push ecx
		push 1
		push eax
		jmp D2CLIENT_ClickShopItem_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_ShopAction_ASM(DWORD pItem, DWORD pNpc, DWORD pNPC, DWORD _1, DWORD pTable2 /* Could be also the ItemCost?*/, DWORD dwMode, DWORD _2, DWORD _3)
{
	__asm {
		jmp D2CLIENT_ShopAction_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_clickBelt(DWORD x, DWORD y, DWORD IventoryData)
{
	__asm {
		mov eax, edx
		jmp D2CLIENT_clickBelt_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_clickItemRight_ASM(DWORD x, DWORD y, DWORD Location, DWORD Player, DWORD pUnitInventory)
{
	__asm
	{
		// ECX = y, EDX = x - Blizzard is weird :D
		MOV EAX, ECX
		MOV ECX, EDX
		MOV EDX, EAX

		POP EAX
		MOV EBX,EAX
		POP EAX
		PUSH EBX
		jmp D2CLIENT_clickItemRight_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_clickBeltRight_ASM(DWORD pInventory, DWORD pPlayer, DWORD HoldShift, DWORD dwPotPos)
{
	__asm
	{
		POP EAX
		MOV EBX,EAX
		POP EAX
		PUSH EBX
		JMP D2CLIENT_clickBeltRight_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_GetItemDesc_ASM(DWORD pUnit, wchar_t* pBuffer)
{
	__asm 
	{
		PUSH EDI
		MOV EDI, EDX
		PUSH NULL
		PUSH 1		// TRUE = New lines, FALSE = Comma between each stat value
		PUSH ECX
		CALL D2CLIENT_GetItemDesc_I
		POP EDI
		RETN
	}
}

VOID __declspec(naked) __fastcall D2COMMON_DisplayOverheadMsg_ASM(DWORD pUnit)
{
	__asm
	{
		LEA ESI, [ECX+0xA4]
		MOV EAX, [ECX+0xA4]

		PUSH EAX
		PUSH 0
		call D2COMMON_DisplayOverheadMsg_I

		RETN
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_MercItemAction_ASM(DWORD bPacketType, DWORD dwSlot)
{
	__asm 
	{
		mov eax, ecx
		mov ecx, edx
		jmp D2CLIENT_MercItemAction_I
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_PlaySound(DWORD dwSoundId)
{
	__asm
	{
		MOV EBX, ECX
		PUSH NULL
		PUSH NULL
		PUSH NULL
		CALL D2CLIENT_GetPlayerUnit
		PUSH EAX
		MOV ECX, EAX
		MOV EDX, EBX
		CALL D2CLIENT_PlaySound_I
		RETN
	}
}

VOID __declspec(naked) __fastcall D2CLIENT_TakeWP(DWORD dwUnitId, DWORD dwLevelId)
{
	__asm
	{
		MOV AL, 0x49
		CALL D2CLIENT_TakeWP_I
		RETN
	}
}

DWORD __declspec(naked) __fastcall TestPvpFlag_STUB(DWORD planum1, DWORD planum2, DWORD flagmask)
{
	__asm 
	{
		push esi;
		push [esp+8];
		mov esi, edx;
		mov edx, ecx;
		call D2CLIENT_TestPvpFlag_I;
		pop esi;
		ret 4;
	}
}

VOID __declspec(naked) __fastcall DrawRectFrame_STUB(RECT* rect)
{
	__asm
	{
		mov eax, ecx;
		jmp D2CLIENT_DrawRectFrame;
	}
}


VOID __declspec(naked) __stdcall myClickMap_ASM(DWORD MouseFlag, DWORD x, DWORD y, DWORD Type)
{
	__asm
	{
		MOV EAX,DWORD PTR DS:[D2CLIENT_clickMap_1]
		MOV EAX, [EAX]
		SUB ESP,0x20
		PUSH EBP
		XOR EBP,EBP
		CMP EAX,EBP
		JNZ L012
		PUSH EAX
		MOV EAX, [D2CLIENT_clickMap_2]
		CMP DWORD PTR DS:[EAX],EBP
		POP EAX
		JE L016
		CMP DWORD PTR SS:[ESP+0x2C],EBP
		JNZ L012
		CMP DWORD PTR SS:[ESP+0x30],EBP
		JE L016
	L012:
		XOR EAX,EAX
		POP EBP
		ADD ESP,0x20
		RETN 0x10
	L016:
		PUSH EBX
		PUSH ESI
		PUSH EDI
		LEA EDI,DWORD PTR SS:[ESP+0x3C]
		LEA EBX,DWORD PTR SS:[ESP+0x38]
		PUSH EAX
		MOV EAX, [D2CLIENT_clickMap_2]
		MOV DWORD PTR DS:[EAX],1
		POP EAX
		CALL D2CLIENT_clickMap_3
		MOV EBX, [p_D2CLIENT_PlayerUnit]
		MOV EBX,DWORD PTR DS:[EBX]
		XOR EAX,EAX
		MOV DWORD PTR SS:[ESP+0x10],EAX
		MOV DWORD PTR SS:[ESP+0x14],EAX
		MOV DWORD PTR SS:[ESP+0x18],EAX
		MOV DWORD PTR SS:[ESP+0x1C],EAX
		MOV DWORD PTR SS:[ESP+0x20],EAX
		MOV DWORD PTR SS:[ESP+0x24],EAX
		MOV DWORD PTR SS:[ESP+0x28],EAX
		MOV DWORD PTR SS:[ESP+0x2C],EAX
		XOR EDI,EDI
		MOV DWORD PTR SS:[ESP+0x14],EBX	
		CALL myGetSelectedUnit
		JMP D2CLIENT_clickMap_4
	}
/*
6FB0CE80  /$ A1 7C2BBB6F    MOV EAX,DWORD PTR DS:[6FBB2B7C]
6FB0CE85  |. 83EC 20        SUB ESP,20
6FB0CE88  |. 55             PUSH EBP
6FB0CE89  |. 33ED           XOR EBP,EBP
6FB0CE8B  |. 3BC5           CMP EAX,EBP
6FB0CE8D  |. 75 14          JNZ SHORT d2client.6FB0CEA3
6FB0CE8F  |. 392D D8C1BC6F  CMP DWORD PTR DS:[6FBCC1D8],EBP
6FB0CE95  |. 74 15          JE SHORT d2client.6FB0CEAC
6FB0CE97  |. 396C24 2C      CMP DWORD PTR SS:[ESP+2C],EBP
6FB0CE9B  |. 75 06          JNZ SHORT d2client.6FB0CEA3
6FB0CE9D  |. 396C24 30      CMP DWORD PTR SS:[ESP+30],EBP
6FB0CEA1  |. 74 09          JE SHORT d2client.6FB0CEAC
6FB0CEA3  |> 33C0           XOR EAX,EAX
6FB0CEA5  |. 5D             POP EBP
6FB0CEA6  |. 83C4 20        ADD ESP,20
6FB0CEA9  |. C2 1000        RETN 10
6FB0CEAC  |> 53             PUSH EBX
6FB0CEAD  |. 56             PUSH ESI
6FB0CEAE  |. 57             PUSH EDI
6FB0CEAF  |. 8D7C24 3C      LEA EDI,DWORD PTR SS:[ESP+3C]
6FB0CEB3  |. 8D5C24 38      LEA EBX,DWORD PTR SS:[ESP+38]
6FB0CEB7  |. C705 D8C1BC6F >MOV DWORD PTR DS:[6FBCC1D8],1
6FB0CEC1  |. E8 DA060000    CALL d2client.6FB0D5A0
6FB0CEC6  |. 8B1D D0C3BC6F  MOV EBX,DWORD PTR DS:[6FBCC3D0]
6FB0CECC  |. 33C0           XOR EAX,EAX
6FB0CECE  |. 894424 10      MOV DWORD PTR SS:[ESP+10],EAX
6FB0CED2  |. 894424 14      MOV DWORD PTR SS:[ESP+14],EAX
6FB0CED6  |. 894424 18      MOV DWORD PTR SS:[ESP+18],EAX
6FB0CEDA  |. 894424 1C      MOV DWORD PTR SS:[ESP+1C],EAX
6FB0CEDE  |. 894424 20      MOV DWORD PTR SS:[ESP+20],EAX
6FB0CEE2  |. 894424 24      MOV DWORD PTR SS:[ESP+24],EAX
6FB0CEE6  |. 894424 28      MOV DWORD PTR SS:[ESP+28],EAX
6FB0CEEA  |. 894424 2C      MOV DWORD PTR SS:[ESP+2C],EAX
6FB0CEEE  |. 33FF           XOR EDI,EDI
6FB0CEF0  |. 895C24 14      MOV DWORD PTR SS:[ESP+14],EBX
6FB0CEF4  |. E8 A71D0100    CALL d2client.6FB1ECA0
6FB0CEF9  |. 8B4C24 38      MOV ECX,DWORD PTR SS:[ESP+38]



*/
}

__declspec(naked) DWORD __fastcall D2CLIENT_GetMinionCount(UnitAny* pUnit, DWORD dwType)
{
	__asm
	{
		MOV EAX, ECX
		MOV ESI, EDX
		JMP [D2CLIENT_GetMinionCount_I]
	}
}

__declspec(naked) VOID __fastcall D2CLIENT_HostilePartyUnit(RosterUnit* pUnit, DWORD dwButton)
{
	__asm
	{
		MOV EAX, EDX
		JMP [D2CLIENT_clickParty_II]
	}
}

__declspec(naked) VOID __fastcall D2CLIENT_TakeWaypoint(DWORD dwWaypointId, DWORD dwArea)
{
	__asm
	{
		POP EAX
		PUSH 0
		PUSH EAX
		PUSH EBP
		PUSH ESI
		PUSH EDI
		PUSH EBX
		XOR EDI,EDI
		MOV EBX,1
		LEA EBP,DWORD PTR SS:[EBP-0x20]
		JMP [D2CLIENT_TakeWaypoint_I]
	}
}

double GetDistance(long x1, long y1, long x2, long y2, DistanceType type)
{
	double dist = 0;
	switch(type)
	{
		case Euclidean:
			{
				double dx = (double)(x2 - x1);
				double dy = (double)(y2 - y1);
				dx = pow(dx, 2);
				dy = pow(dy, 2);
				dist = sqrt(dx + dy); 
			}
			break;
		case Chebyshev:
			{
				long dx = (x2 - x1);
				long dy = (y2 - y1);
				dx = abs(dx);
				dy = abs(dy);
				dist = max(dx, dy); 
			}
			break;
		case Manhattan:
			{
				long dx = (x2 - x1);
				long dy = (y2 - y1);
				dx = abs(dx);
				dy = abs(dy);
				dist = (dx + dy);
			}
			break;
		default: 
			dist = -1; 
			break;
	}
 	return dist;
}
