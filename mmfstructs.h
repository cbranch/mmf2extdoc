/* Excerpt of structures from cncf.h of the MMF2SDK. */

struct kpxRunInfos {
	void* conditions;			// 00 Offset to condition jump list
	void* actions;				// 04 Offset to action jump list
	void* expressions;			// 08 Offset to expression jump list
	short	numOfConditions;			// 0C Number of conditions
	short	numOfActions;				// 0E Number of actions
	short	numOfExpressions;			// 10 Number of expressions
	WORD	editDataSize;				// 12 Size of the data zone when exploded
	DWORD	editFlags;					// 14 Object flags
	char	windowProcPriority;			// 16 Priority of the routine 0-255
	char	free;						
	short	editPrefs;					// 18 Preferences d'edition
	long	identifier;					// 1A Chaine d'identification
	short	version;					// 1E Version courante
										// 20
};
typedef struct infosEventsV2 {
	short 	code;					// Le numero de type + code event
	short	flags;					// Les flags a mettre dans l'event
	short	nParams;				// Le nombre de parametres
//	short	param[X];				// Le type des parametres
//	short	paramTitle[X];			// Le titre de chacun des params
} infosEventsV2;
typedef struct {
	short			menu;			// Menu identifier
	short			string;			// String identifier
	infosEventsV2	infos;			// Sub structure
} eventInformations2;
class mv {
public:
	// Common to editor and runtime
	HINSTANCE			mvHInst;				// Application HINSTANCE
	LPVOID				mvIdAppli;				// Application object in DLL
	LPVOID				mvIdMainWin;			// Main window object in DLL
	LPVOID				mvIdEditWin;			// Child window object in DLL
	HWND				mvHMainWin;				// Main window handle
	HWND				mvHEditWin;				// Child window handle
	HPALETTE			mvHPal256;				// 256 color palette
	WORD				mvAppMode;				// Screen mode with flags
	WORD				mvScrMode;				// Screen mode
	DWORD				mvEditDXDocToClient;	// Edit time only: top-left coordinates
	DWORD				mvEditDYDocToClient;
	LPVOID	mvImgFilterMgr;			// Image filter manager
	LPVOID	mvSndFilterMgr;			// Sound filter manager
	LPVOID		mvSndMgr;				// Sound manager

	union {
		LPVOID		mvEditApp;				// Current application, edit time (not used)
		LPVOID		mvRunApp;				// Current application, runtime
	};
	union {
		LPVOID		mvEditFrame;
		LPVOID		mvRunFrame;
	};

	// Runtime
	LPVOID			mvRunHdr;
	DWORD				mvPrefs;				// Preferences (sound on/off)
	LPTSTR				subType;
	BOOL				mvFullScreen;			// Full screen mode
	LPTSTR				mvMainAppFileName;		// App filename
	int					mvAppListCount;
	int					mvAppListSize;
	LPVOID*			mvAppList;
	int					mvExtListCount;
	int					mvExtListSize;
	LPTSTR *			mvExtList;
	int					mvNbDllTrans;
	LPVOID			mvDllTransList;
	DWORD				mvJoyCaps[32];
	HHOOK				mvHMsgHook;
	int					mvModalLoop;
	int					mvModalSubAppCount;
	LPVOID				mvFree[5];

	// Functions
	////////////

	// Editor: Open Help file
	void				(CALLBACK * mvHelpA) (LPCSTR pHelpFile, UINT nID, LPARAM lParam);

	// Editor: Get default font for object creation
	BOOL				(CALLBACK * mvGetDefaultFontA) (LPLOGFONTA plf, LPSTR pStyle, int cbSize);

	// Editor: Edit images and animations
	BOOL				(CALLBACK * mvEditSurfaceA) (LPVOID edPtr, LPVOID pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditImageA) (LPVOID edPtr, LPVOID pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditAnimationA) (LPVOID edPtr, LPVOID pParams, HWND hParent);

	// Runtime: Extension User data
	LPVOID				(CALLBACK * mvGetExtUserData) (LPVOID pApp, HINSTANCE hInst);
	LPVOID				(CALLBACK * mvSetExtUserData) (LPVOID pApp, HINSTANCE hInst, LPVOID pData);

	// Runtime: Register dialog box
	void				(CALLBACK * mvRegisterDialogBox) (HWND hDlg);
	void				(CALLBACK * mvUnregisterDialogBox) (HWND hDlg);

	// Runtime: Add surface as backdrop object
	void				(CALLBACK * mvAddBackdrop) (LPVOID pSf, int x, int y, DWORD dwInkEffect, DWORD dwInkEffectParam, int nObstacleType, int nLayer);

	// Runtime: Binary files
	BOOL				(CALLBACK * mvGetFileA)(LPCSTR pPath, LPSTR pFilePath, DWORD dwFlags);
	void				(CALLBACK * mvReleaseFileA)(LPCSTR pPath);
	HANDLE				(CALLBACK * mvOpenHFileA)(LPCSTR pPath, LPDWORD pDwSize, DWORD dwFlags);
	void				(CALLBACK * mvCloseHFile)(HANDLE hf);

	// Plugin: download file
	int					(CALLBACK * mvLoadNetFileA) (LPSTR pFilename);

	// Plugin: send command to Vitalize
	int					(CALLBACK * mvNetCommandA) (int, LPVOID, DWORD, LPVOID, DWORD);

	// Editor & Runtime: Returns the version of MMF or of the runtime
	DWORD				(CALLBACK * mvGetVersion) ();

	// Editor & Runtime: callback function for properties or other functions
	LRESULT			(CALLBACK * mvCallFunction) (LPVOID edPtr, int nFnc, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);

	// Editor: Open Help file (UNICODE)
	void				(CALLBACK * mvHelpW) (LPCWSTR pHelpFile, UINT nID, LPARAM lParam);

	// Editor: Get default font for object creation (UNICODE)
	BOOL				(CALLBACK * mvGetDefaultFontW) (LPLOGFONTW plf, LPWSTR pStyle, int cbSize);

	// Editor: Edit images and animations (UNICODE)
	BOOL				(CALLBACK * mvEditSurfaceW) (LPVOID edPtr, LPVOID pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditImageW) (LPVOID edPtr, LPVOID pParams, HWND hParent);
	BOOL				(CALLBACK * mvEditAnimationW) (LPVOID edPtr, LPVOID pParams, HWND hParent);

	// Runtime: Binary files (UNICODE
	BOOL				(CALLBACK * mvGetFileW)(LPCWSTR pPath, LPWSTR pFilePath, DWORD dwFlags);
	void				(CALLBACK * mvReleaseFileW)(LPCWSTR pPath);
	HANDLE				(CALLBACK * mvOpenHFileW)(LPCWSTR pPath, LPDWORD pDwSize, DWORD dwFlags);

	// Plugin: download file
	int					(CALLBACK * mvLoadNetFileW) (LPWSTR pFilename);

	// Plugin: send command to Vitalize
	int					(CALLBACK * mvNetCommandW) (int, LPVOID, DWORD, LPVOID, DWORD);

	// Place-holder for next versions
	LPVOID				mvAdditionalFncs[6];
};
typedef short (WINAPI *GetRunObjectInfos)(mv *mV, kpxRunInfos *infoPtr);
typedef infosEventsV2* (WINAPI *GetACEInfos)(mv *mV, short code);
typedef HMENU (WINAPI *GetACEMenu)(mv *mV, LPVOID oiPtr, LPVOID edPtr);