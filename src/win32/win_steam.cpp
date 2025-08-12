// LWSS: this is a Rudimentary Steam Authentication for the COD4 Client -> Dedicated Server
// The Client Generates a Steam ticket and sends it to the Server (in lieu of a CD Key)
// The Server checks this Ticket and then puts in a delayed networked check to Steam.

// You can try to disable this code, but you really shouldn't since Server GUID's are set to the SteamID64
// An added benefit of Steam is that their SteamID can't be changed without them spending another 20$

#include "win_steam.h"

#include <Windows.h>

#include <qcommon/qcommon.h>

#include <steamsdk/steam/steam_api.h>
#include <steamsdk/steam/isteamgameserver.h>
#include <sys/stat.h>

#include <atomic>
#include <mutex>
#include <vector>
#include <qcommon/cmd.h>

#ifdef KISAK_MP
#include <server_mp/server_mp.h>
#endif

#include <functional>

bool g_steamInitialized = false;
uint64_t g_steamID = 0;
char g_steamName[64]{ 0 };

static HAuthTicket steamClientAuthTicket = 0;
static unsigned char steamClientTicket[1024]{ 0 };
static uint32 steamClientTicketSize = 0;

std::mutex kickPlayerMutex;
std::vector<uint64_t> steamIDsToKick;

uint64_t Steam_GetClientSteamID64()
{
	iassert(g_steamID != 0);
	return g_steamID;
}

bool Steam_GetRawClientTicket(unsigned char **pBuffer, uint32 *pSize)
{
	if (!steamClientTicketSize || !steamClientTicket[0])
	{
		iassert(0);
		return false;
	}

	*pSize = steamClientTicketSize;
	*pBuffer = steamClientTicket;

	return true;
}

void Steam_CancelClientTicket()
{
	if (steamClientAuthTicket)
	{
		SteamUser()->CancelAuthTicket(steamClientAuthTicket);
		steamClientAuthTicket = 0;
	}
}

// This seems to require a class... This sucks
class SteamCallbackHandler
{
public:
	SteamCallbackHandler();
	~SteamCallbackHandler() = default;

	STEAM_CALLBACK(SteamCallbackHandler, Callback_BeginAuthResponse, ValidateAuthTicketResponse_t, m_CallbackBeginAuthResponse);

};

SteamCallbackHandler::SteamCallbackHandler() 
	: m_CallbackBeginAuthResponse(this, &SteamCallbackHandler::Callback_BeginAuthResponse)
{
}
static SteamCallbackHandler *callbackHandler;


void SteamCallbackHandler::Callback_BeginAuthResponse(ValidateAuthTicketResponse_t *pCallback)
{
	uint64_t id64 = pCallback->m_SteamID.ConvertToUint64();

	if (pCallback->m_eAuthSessionResponse != k_EAuthSessionResponseOK)
	{
		// KICK PLAYER FROM SERVER
		{
			std::lock_guard<std::mutex> _(kickPlayerMutex);
			steamIDsToKick.push_back(id64);
		}
	}
	else
	{
		Com_Printf(0, "[Kisak] Player %lld passed Steam Auth Check", id64);
	}
}

void Steam_CheckClients()
{
#ifdef KISAK_MP
	SteamAPI_RunCallbacks();

	std::lock_guard<std::mutex> _(kickPlayerMutex);

	if (steamIDsToKick.empty())
	{
		return;
	}

	int clientNum = 0;
	client_t *client = &svs.clients[0];
	while (clientNum < sv_maxclients->current.integer)
	{
		char *endptr;
		uint64_t steamID64 = strtoull(client->cdkeyHash, &endptr, 10);

		for (int i = 0; i < steamIDsToKick.size(); i++)
		{
			if (steamID64 == steamIDsToKick[i])
			{
				SV_DropClient(client, "Steam Auth Timed out", true);
			}
		}

		clientNum++;
		client++;
	}

	steamIDsToKick.clear();
#endif
}


// callback hook for debug text emitted from the Steam API
extern "C" void __cdecl SteamAPIDebugTextHook(int nSeverity, const char *pchDebugText)
{
	Com_Printf(0, pchDebugText);
}

// This Creates a "steam_appid.txt" next to the executable for the Steam API to read
// 7940 = COD4 APPID
static void CreateAppIdFile()
{
	// Check if File Exists Already
	struct stat fileinfo;
	if (stat("steam_appid.txt", &fileinfo) == 0)
	{
		return;
	}

	FILE *fp = fopen("steam_appid.txt", "w");
	if (!fp)
	{
		Com_Printf(0, "[Kisak] Error opening 'steam_appid.txt'.");
		return;
	}

	fprintf(fp, "%d", 7940);
	fclose(fp);
}

void Steam_Init()
{
	CreateAppIdFile();
	SetEnvironmentVariableA("SteamAppId", "7940");
	SetEnvironmentVariableA("SteamGameId", "7940");
	SetEnvironmentVariableA("SteamOverlayGameId", "7940");

	if (!SteamAPI_Init())
	{
		Com_Printf(0, "[Kisak] SteamAPI_Init() failed. Is the Steam Client Running? Do you own COD4?");
		return;
	}

	if (!SteamUser()->BLoggedOn())
	{
		Com_Printf(0, "[Kisak] Steam User isn't Logged On.");
		return;
	}

	g_steamInitialized = true;

	SteamClient()->SetWarningMessageHook(&SteamAPIDebugTextHook);

	strncpy(g_steamName, SteamFriends()->GetPersonaName(), 63);
	g_steamID = SteamUser()->GetSteamID().ConvertToUint64();

	callbackHandler = new SteamCallbackHandler();
}

void Steam_Shutdown()
{
	SteamAPI_Shutdown();

	delete callbackHandler;

	g_steamInitialized = false;
	memset(g_steamName, 0, 64);
	g_steamID = 0;
}

bool Steam_UpdateClientAuthTicket(netadr_t serverIpv4)
{
	if (!g_steamInitialized)
	{
		return false;
	}

	SteamNetworkingIdentity snid;
	uint32 ip32 = 0;
	ip32 |= (serverIpv4.ip[0] << 24);
	ip32 |= (serverIpv4.ip[1] << 16);
	ip32 |= (serverIpv4.ip[2] << 8);
	ip32 |= (serverIpv4.ip[3]);
	snid.SetIPv4Addr(ip32, serverIpv4.port);

	steamClientAuthTicket = SteamUser()->GetAuthSessionTicket(steamClientTicket, sizeof(steamClientTicket), &steamClientTicketSize, &snid);

	return steamClientAuthTicket != k_HAuthTicketInvalid;
}

bool Steam_CheckClientTicket(const void* pAuthTicket, uint32 authTicketLen, uint64_t steamID64)
{
	iassert(steamID64);

	if (!g_steamInitialized)
	{
		Com_Printf(0, "[Kisak] Steam_CheckClientTicket() - Steam is not Initialized! Returning False");
		return false;
	}

	CSteamID id;
	id.SetFromUint64(steamID64);

	EBeginAuthSessionResult res = SteamUser()->BeginAuthSession(pAuthTicket, authTicketLen, id);
	
	if (res != k_EBeginAuthSessionResultOK)
	{
		return false;
	}

	// Good for Now, the callback will kick him later if needed.
	return true;
}

void Steam_OnClientDropped(uint64_t steamID64)
{
	iassert(steamID64);

	CSteamID sid;
	sid.SetFromUint64(steamID64);
	SteamUser()->EndAuthSession(sid);
}

static void Steam_SV_TestKick()
{
	if (!com_sv_running->current.enabled)
	{
		Com_Printf(0, "Server is not running.\n");
		return;
	}
	if (SV_Cmd_Argc() != 2)
	{
		const char *cmdName = SV_Cmd_Argv(0);
		Com_Printf(0, "Usage: %s <SteamID64>\n", cmdName);
		return;
	}

	const char *arg1 = SV_Cmd_Argv(1);
	char *endptr;
	uint64_t steamID64 = strtoull(arg1, &endptr, 10);
	{
		std::lock_guard<std::mutex> _(kickPlayerMutex);
		steamIDsToKick.push_back(steamID64);
	}
}

cmd_function_s Steam_SV_TestSteamKick_SERVER;
cmd_function_s Steam_SV_TestSteamKick;
void Steam_SV_AddTestCommands()
{
	Cmd_AddCommandInternal("steam_testkick", Cbuf_AddServerText_f, &Steam_SV_TestSteamKick);
	Cmd_AddServerCommandInternal("steam_testkick", Steam_SV_TestKick, &Steam_SV_TestSteamKick_SERVER);
}