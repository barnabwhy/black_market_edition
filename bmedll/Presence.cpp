#include "pch.h"
#include "Presence.h"
#include "TTFSDK.h"
#include "ICvar.h"
#include "ConVar.h"
#include "SigScanning.h"
#include "ConCommandManager.h"
#include "Util.h"

Presence& PresenceGlob()
{
    return SDK().GetPresence();
}

#define WRAPPED_MEMBER(name) MemberWrapper<decltype(&Presence::##name), &Presence::##name, decltype(&PresenceGlob), &PresenceGlob>::Call

int timestamp()
{
    const auto p1 = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::seconds>(
        p1.time_since_epoch()).count();
}

const char* Presence::getDisplayMapName(const char* map)
{
    if (std::strcmp(map, "mp_corporate") == 0) return "Corporate";
    else if (std::strcmp(map, "mp_fracture") == 0) return "Fracture";
    else if (std::strcmp(map, "mp_harmony_mines") == 0) return "Dig Site";
    else if (std::strcmp(map, "mp_haven") == 0) return "Haven";
    else if (std::strcmp(map, "mp_lagoon") == 0) return "Lagoon";
    else if (std::strcmp(map, "mp_lobby") == 0) return "Lobby";
    else if (std::strcmp(map, "mp_nexus") == 0) return "Nexus";
    else if (std::strcmp(map, "mp_npe") == 0) return "Pilot Training";
    else if (std::strcmp(map, "mp_outpost_207") == 0) return "Outpost 207";
    else if (std::strcmp(map, "mp_overlook") == 0) return "Overlook";
    else if (std::strcmp(map, "mp_sandtrap") == 0) return "Sandtrap";
    else if (std::strcmp(map, "mp_swampland") == 0) return "Swampland";
    else if (std::strcmp(map, "mp_wargames") == 0) return "War Games";
    else if (std::strcmp(map, "mp_relic") == 0) return "Relic";
    else if (std::strcmp(map, "mp_o2") == 0) return "Demeter";
    else if (std::strcmp(map, "mp_colony") == 0) return "Colony";
    else if (std::strcmp(map, "mp_runoff") == 0) return "Runoff";
    else if (std::strcmp(map, "mp_smugglers_cove") == 0) return "Smuggler\'s Cove";
    else if (std::strcmp(map, "mp_switchback") == 0) return "Export";
    else if (std::strcmp(map, "mp_angel_city") == 0) return "Angel City";
    else if (std::strcmp(map, "mp_backwater") == 0) return "Backwater";
    else if (std::strcmp(map, "mp_zone_18") == 0) return "Zone 18";
    //else return std::string(map).substr(3, strlen(map) - 3).c_str();
    else return map + 3;
}

const char* Presence::getDisplayPlaylistName(const char* playlist)
{
    if (std::strcmp(playlist, "private_match") == 0) return "Private Match";
    else if (std::strcmp(playlist, "mfd") == 0) return "Marked for Death";
    else if (std::strcmp(playlist, "mfdp") == 0) return "MFD Pro";
    else if (std::strcmp(playlist, "at") == 0) return "Attrition";
    else if (std::strcmp(playlist, "campaign_carousel") == 0) return "Campaign";
    else if (std::strcmp(playlist, "coop") == 0) return "Frontier Defense";
    else if (std::strcmp(playlist, "lts") == 0) return "Last Titan Standing";
    else if (std::strcmp(playlist, "lava") == 0) return "Deadly Ground";
    else if (std::strcmp(playlist, "wlts") == 0) return "Wingman LTS";
    else if (std::strcmp(playlist, "cp") == 0) return "Hardpoints";
    else if (std::strcmp(playlist, "ctf") == 0) return "Capture the Flag";
    else if (std::strcmp(playlist, "ps") == 0) return "Pilot Skirmish";
    else if (std::strcmp(playlist, "all") == 0) return "Variety Pack";
    else if (std::strcmp(playlist, "tdm") == 0) return "Pilot Hunter";
    else if (std::strcmp(playlist, "Load a map on the command line") == 0) return "";
    else return playlist;
}

const char* Presence::getDisplayNameForTrainingResumeChoice(int choice)
{
    switch (choice) {
    case 0: return "Look, Run, and Jump";
    case 1: return "Wallrunning";
    case 2: return "Freeform Wallrun";
    case 3: return "Double Jump";
    case 4: return "Freeform Double Jump";
    case 5: return "Cloak";
    case 6: return "Basic Combat";
    case 7: return "Firing Range";
    case 8: return "Grenade Range";
    case 9: return "Pilot Combat Scenario";
    case 10: return "Titan Dash";
    case 11: return "Titan Vortex Shield";
    case 12: return "Titan AI Control";
    case 13: return "Titan Combat Scenario";
    }
    return nullptr;
}

/*int Presence::getDisplayNumberForTrainingResumeChoice(int choice)
{
    switch (choice) {
    case 0: return "Look, Run, and Jump";
    case 1: return "Wallrunning";
    case 2: return "Freeform Wallrun";
    case 3: return "Double Jump";
    case 4: return "Freeform Double Jump";
    case 5: return "Cloak";
    case 6: return "Basic Combat";
    case 7: return "Firing Range";
    case 8: return "Grenade Range";
    case 9: return "Pilot Combat Scenario";
    case 10: return "Titan Dash";
    case 11: return "Titan Vortex Shield";
    case 12: return "Titan AI Control";
    case 13: return "Titan Combat Scenario";
    }
    return nullptr;
}*/

void Presence::updateRichPresenceLoading(bool requestOriginUpdateImmediately) {
    strncpy(richPresenceBuffer, "Loading...", 1024);
    strncpy(richPresenceBufferInGame, "Loading...", 256);

    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.SetDetails("Loading...");
    activity.SetState("Black Market Edition");
    /*activity.GetAssets().SetSmallImage("");
    activity.GetAssets().SetSmallText("");*/
    activity.GetAssets().SetLargeImage("titanfall_101");
    activity.GetAssets().SetLargeText("Titanfall");
    SDK().GetDiscord().UpdateActivity(activity);


    if (requestOriginUpdateImmediately) {
        requestOriginPresenceUpdate();
    }
}

void Presence::updateRichPresenceLoadingWithMap(const char* map, bool requestOriginUpdateImmediately) {
    if (*(char*)map == 0) return;
    if (std::strcmp(map, "mp_lobby") == 0) return;

    //const char* map = (const char*)(enginedllBaseAddress+0x79719C); // alt, faster (from cl_showfps???)
    //if (!*(char*)map || strncmp(map, "mp_", 3)) map = (const char*)(clientdllBaseAddress+0x16BC7E0);

    std::stringstream ss;
    ss << "Loading... (" << getDisplayMapName(map)  << ")";

    strncpy(richPresenceBuffer, ss.str().c_str(), 1024);
    strncpy(richPresenceBufferInGame, ss.str().c_str(), 256);

    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.SetDetails("Loading...");
    activity.SetState("Black Market Edition");
    /*activity.GetAssets().SetSmallImage("");
    activity.GetAssets().SetSmallText("");*/

    if (std::strcmp(map, "mp_npe") != 0) // it doesn't update the team to IMC correctly there
    {
        if (*teamNum == 2) {
            activity.GetAssets().SetSmallImage("team_imc");
            activity.GetAssets().SetSmallText("IMC");
        }
        else if (*teamNum == 3) {
            activity.GetAssets().SetSmallImage("team_mil");
            activity.GetAssets().SetSmallText("Militia");
        }
    }
    
    std::stringstream largeimagess; largeimagess << map << "_widescreen";
    activity.GetAssets().SetLargeImage(largeimagess.str().c_str());
    activity.GetAssets().SetLargeText(getDisplayMapName(map));

    SDK().GetDiscord().UpdateActivity(activity);

    if (requestOriginUpdateImmediately) {
        requestOriginPresenceUpdate();
    }

}

void Presence::updateRichPresence(bool requestOriginUpdateImmediately)
{
    std::stringstream ss;
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);

    if (lastMap && strcmp(lastMap, map) != 0)
    {
        gameStartTime = timestamp();
        gameEndTime = INT_MIN;
        gameEndTime2 = INT_MIN;
        switchedSides = false;
        roundNumber = 1;
        strncpy(lastMap, map, 32);
    }

    if (
        (
            (
                //(!updatePresence2LastInput || strcmp((char*)updatePresence2LastInput, "") == 0) // last presence captured is empty
                /*|| (canJoin && (!*(char*)joinSecretInGame || strcmp((char*)joinSecretInGame, "") == 0))*/ // or presence in game is empty (assuming it's not empty because anti-join is on)
                //(!joinSecretInGame || !*(char*)joinSecretInGame || strcmp((char*)joinSecretInGame, "") == 0) // presence in game is empty (anti-join not implemented in this code, so above commented out)
                (!presenceBuffer || !*(char*)presenceBuffer || strcmp((char*)presenceBuffer, "") == 0) // presence in game is empty (anti-join not implemented in this code, so above commented out)
                )
            && !*isConnectedAndInLobby // and we are not connected
            )
        //|| (!*(char*)map && !*(char*)map_old) // or we're not on any map
        || !*(char*)map // or we're not on any map
        ) {

        ss << "Main menu";
        activity.SetDetails("Main Menu");
        activity.SetState("Black Market Edition");
        /*activity.GetAssets().SetSmallImage("");
        activity.GetAssets().SetSmallText("");*/
        activity.GetAssets().SetLargeImage("titanfall_101");
        activity.GetAssets().SetLargeText("Titanfall");
        gameStartTime = timestamp(); // also a hack, because "main menu" is set for a short while while changing lobbies
        gameEndTime = INT_MIN;
        gameEndTime2 = INT_MIN;
        playerCount = 1;
        switchedSides = false;
        roundNumber = 1;
    }
    else if (strcmp(map, "mp_npe") == 0 || *isConnectedAndInLobby == 2) {
        ss << "Training"; // IMC
        activity.SetDetails("Pilot Training");
        const char* pilotTrainingStageDesc = Presence::getDisplayNameForTrainingResumeChoice(trainingStage);
        if (pilotTrainingStageDesc) {
            activity.SetState(pilotTrainingStageDesc);
            activity.GetParty().GetSize().SetMaxSize(14);
            activity.GetParty().GetSize().SetCurrentSize(trainingStage);
        }
        else {
            activity.SetState("Solo"); // TODO: Add pilot training stage here
            activity.GetParty().GetSize().SetMaxSize(1);
            activity.GetParty().GetSize().SetCurrentSize(1);
        }
        activity.GetAssets().SetSmallImage("team_imc");
        activity.GetAssets().SetSmallText("IMC");
        activity.GetAssets().SetLargeImage("mp_npe_widescreen");
        activity.GetAssets().SetLargeText("Pilot Training Map");
    }
    else if (std::strcmp(map, "mp_lobby") == 0 && *isPrivateLobby) {
        ss << "Private lobby";
        //activity.SetDetails("Private lobby");
        activity.SetState("Private lobby");
        activity.GetAssets().SetLargeImage("mp_lobby_widescreen");
        activity.GetAssets().SetLargeText("Lobby");
        activity.GetParty().GetSize().SetMaxSize(6);
        activity.GetParty().GetSize().SetCurrentSize(playerCount);
        activity.GetParty().SetId(serverIPAndPortBufferInGame);
        if (playerCount < 6)
            activity.GetSecrets().SetJoin(getJoinSecret().c_str());
    }
    else if (std::strcmp(match_playlist, "private_match") == 0 && std::strcmp(map, "mp_lobby") == 0) { // meh..........
        ss << "Private match lobby";
        activity.SetDetails("Private Match");
        activity.SetState("In lobby");
        if (*teamNum == 2) {
            activity.GetAssets().SetSmallImage("team_imc");
            activity.GetAssets().SetSmallText("IMC");
        }
        else if (*teamNum == 3) {
            activity.GetAssets().SetSmallImage("team_mil");
            activity.GetAssets().SetSmallText("Militia");
        }
        activity.GetAssets().SetLargeImage("mp_lobby_widescreen");
        activity.GetAssets().SetLargeText("Lobby");
        activity.GetParty().GetSize().SetMaxSize(*maxplayers);
        activity.GetParty().GetSize().SetCurrentSize(playerCount);
        activity.GetParty().SetId(serverIPAndPortBufferInGame);
        if (playerCount < *maxplayers)
            activity.GetSecrets().SetJoin(getJoinSecret().c_str());
    }
    else {
        activity.SetDetails(getDisplayPlaylistName(match_playlist));
        if (std::strcmp(map, "mp_lobby") == 0) {
            ss << "In lobby"; // seems we cannot tell the kind of lobby...
            activity.SetState("In lobby");
            if (std::strcmp(match_playlist, "campaign_carousel") == 0) {
                if (*teamNum == 2) 
                    activity.GetAssets().SetLargeImage("menu_background_imc");
                else if (*teamNum == 3)
                    activity.GetAssets().SetLargeImage("menu_background_militia");
            }
            else {
                activity.GetAssets().SetLargeImage("mp_lobby_widescreen");
            }
            activity.GetAssets().SetLargeText("Lobby");
        }
        else {
            ss << getDisplayPlaylistName(match_playlist) << " on " << getDisplayMapName(map);
            ConVar* gamemode_cvar = SDK().GetVstdlibCvar()->FindVar("mp_gamemode");

            if (std::strcmp(match_playlist, "private_match") != 0 && std::strcmp(match_playlist, "campaign_carousel") != 0 && std::strcmp(match_playlist, "all") != 0) {

                // standard texts
                switch (gameState)
                {
                case WaitingForCustomStart: activity.SetState("Waiting"); break;
                case WaitingForPlayers: activity.SetState("Waiting for players"); break;
                //case Prematch: activity.SetState("Prematch"); break;
                case SuddenDeath: activity.SetState("Sudden Death"); break;
                case SwitchingSides: activity.SetState("Switching Sides"); break;
                case Epilogue: activity.SetState("Epilogue"); break;
                case Postmatch: activity.SetState("Postmatch"); break;
                default: {
                    if (isSwitchSidesBased)
                    {
                        if (!switchedSides)
                            activity.SetState("First half");
                        else
                            activity.SetState("Second half");

                    }
                    else if (isRoundBased && roundNumber > 0) {
                        std::stringstream ssround;
                        ssround << "Round " << roundNumber;
                        if (maxRounds && std::strcmp(gamemode_cvar->GetString(), "mfdp") == 0) ssround << "/" << maxRounds; // I think only MFDP has predictable amount of max rounds (besides FD)
                        activity.SetState(ssround.str().c_str());
                    }
                    else if (std::strcmp(gamemode_cvar->GetString(), "coop") == 0 && roundNumber > 0) {
                        std::stringstream ssround;
                        ssround << "Wave " << roundNumber;
                        if (maxRounds) ssround << "/" << maxRounds;
                        activity.SetState(ssround.str().c_str());
                    }
                    else activity.SetState("Playing"); // just playing
                }
                }

            }
            else {
                // display the precise gamemode, as it's different from playlist name
                activity.SetState(getDisplayPlaylistName(gamemode_cvar->GetString()));
            }

            std::stringstream largeimagess; largeimagess << map << "_widescreen";
            activity.GetAssets().SetLargeImage(largeimagess.str().c_str());
            activity.GetAssets().SetLargeText(getDisplayMapName(map));
        }

        if (*teamNum == 2) {
            activity.GetAssets().SetSmallImage("team_imc");
            activity.GetAssets().SetSmallText("IMC");
        }
        else if (*teamNum == 3) {
            activity.GetAssets().SetSmallImage("team_mil");
            activity.GetAssets().SetSmallText("Militia");
        }

        activity.GetParty().GetSize().SetMaxSize(*maxplayers);
        activity.GetParty().GetSize().SetCurrentSize(playerCount);
        activity.GetParty().SetId(serverIPAndPortBufferInGame);
        if (playerCount < *maxplayers)
            activity.GetSecrets().SetJoin(getJoinSecret().c_str());

    }

    if (gameEndTime != INT_MIN)
    {
        activity.GetTimestamps().SetEnd(gameEndTime);
    }
    else if (gameEndTime2 != INT_MIN)
    {
        activity.GetTimestamps().SetEnd(gameEndTime2);
    }
    else {
        activity.GetTimestamps().SetStart(gameStartTime);
    }

    
    strncpy(richPresenceBuffer, ss.str().c_str(), 1024);
    bool wasPresenceStringChanged = std::strcmp(richPresenceBuffer, richPresenceBuffer) != 0;
    strncpy(richPresenceBufferInGame, ss.str().c_str(), 256);
    SDK().GetDiscord().UpdateActivity(activity);

    if (requestOriginUpdateImmediately && wasPresenceStringChanged) {
        requestOriginPresenceUpdate();
    }

    if (trainingStage >= 0 && std::strcmp(map, "mp_npe") != 0)
        trainingStage = -1;
}

std::string Presence::getJoinSecret()
{
    std::stringstream ss;
    //ss << sessionIDBufferInGame;
    ss << presenceBuffer;
    ss << "|" << SDK().origin->uid;
    return ss.str();
}

// beware: passing via this struct doesn't seem to fucking work, use the below function instead!
ourInGameJoinData Presence::parseJoinSecret(char* str)
{
    ourInGameJoinData data{};
    data.ok = false;
    // we'll do it the Respawn way xD
    char* pch;
    pch = strtok(str, "|");
    if (pch != NULL)
    {
        pch = strtok(NULL, "|");
        if (pch != NULL)
        {
            std::string srv = Util::RemoveChar(std::string(pch), '|');
            data.serverIPAndPort = srv;
            pch = strtok(NULL, "|");
            if (pch != NULL)
            {
                std::string key = Util::RemoveChar(std::string(pch), '|');
                data.encryptionKeyBase64 = key;
                pch = strtok(NULL, "|");
                if (pch != NULL)
                {
                    data.uid = _atoi64(pch);
                    //data.ok = data.uid && data.encryptionKeyBase64 && data.serverIPAndPort;
                    data.ok = true;
                }
            }
        }
    }
    return data;
}

bool Presence::joinGameWithDiscordJoinSecret(char* str)
{
    bool ok = false;
    std::string serverIPAndPort;
    std::string encryptionKeyBase64;
    unsigned long long uid;
    // we'll do it the Respawn way xD
    char* pch;
    pch = strtok(str, "|");
    if (pch != NULL)
    {
        pch = strtok(NULL, "|");
        if (pch != NULL)
        {
            serverIPAndPort = Util::RemoveChar(std::string(pch), '|');
            pch = strtok(NULL, "|");
            if (pch != NULL)
            {
                encryptionKeyBase64 = Util::RemoveChar(std::string(pch), '|');
                pch = strtok(NULL, "|");
                if (pch != NULL)
                {
                    uid = _atoi64(pch);
                    ok = true;
                }
            }
        }
    }

    if (ok) {
        std::stringstream cmd;
        cmd << "connectwithinvite";
        cmd << ' ' << '"' << serverIPAndPort << '"';
        cmd << ' ' << '"' << encryptionKeyBase64 << '"';
        cmd << ' ' << uid;
        spdlog::get("logger")->info("Joining game of origin user ID: {}", uid);
        SDK().GetEngineClient()->ClientCmd_Unrestricted(cmd.str().c_str());
    }

    return ok;
}

void Presence::updateRichPresenceCCommand(const CCommand& args)
{
    //SPDLOG_DEBUG(spdlog::get("logger"), "bme_update_rich_presence");
    spdlog::get("logger")->debug("bme_update_rich_presence");
    updateRichPresence(true);
}

inline void Presence::requestOriginPresenceUpdate() {
    *isPresenceUpdatePending = true;
}

inline void Presence::wipeOriginRichPresenceString() {
    memset(richPresenceString, 0, 256);
}

inline void Presence::wipeOriginJoinSecret() {
    memset(joinSecret, 0, 256);
}

void Presence::SetTrainingResumeChoice(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_npe_set_training_resume_choice {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    trainingStage = atoi(args.Arg(1));
    updateRichPresence(false);
}

void Presence::updatePlayerCountCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_player_count {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    int old_playerCount = playerCount;
    playerCount = atoi(args.Arg(1));
    if (old_playerCount != playerCount)
    {
        updateRichPresence(false);
        spdlog::get("logger")->debug("bme_update_player_count old:{} new:{}", old_playerCount, playerCount);
    }
}

void Presence::updatePlayerCountGrabCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_player_count_grab {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    int old_playerCount = playerCount;

    typedef __int64 (*FN)(int a1);
    FN GetTeamPlayerCount = (FN)(Util::GetModuleBaseAddress("client.dll") + 0xE8E80); // TODO: cache this address if you feel like so
    auto v1 = GetTeamPlayerCount(2);
    if (!v1) return;
    __int64 a1 = (*(__int64(__fastcall**)(__int64))(*(__int64*)v1 + 1664i64))(v1);
    auto v2 = GetTeamPlayerCount(3);
    if (!v2) return;
    __int64 a2 = (*(__int64(__fastcall**)(__int64))(*(__int64*)v2 + 1664i64))(v2);
    //int local_playerCount = GetTeamPlayerCount(2) + GetTeamPlayerCount(3);
    int local_playerCount = a1 + a2;

    if (local_playerCount && old_playerCount != local_playerCount)
    {
        playerCount = local_playerCount;
        updateRichPresence(false);
        spdlog::get("logger")->debug("bme_update_player_count_grab old:{} new:{}", old_playerCount, playerCount);
    }
}

void Presence::updateGameEndTimeCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_gameendtime {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    int local_gameEndTime;
    if (std::strcmp(args.Arg(1), "null") == 0 || std::strcmp(map, "mp_npe") == 0) // we don't want end time for training
        local_gameEndTime = INT_MIN;
    else
        local_gameEndTime = atoi(args.Arg(1));

    if (local_gameEndTime != INT_MIN)
        local_gameEndTime += timestamp() + 1;

    if (local_gameEndTime != gameEndTime) {
        spdlog::get("logger")->debug("Game end time updated");
        gameEndTime = local_gameEndTime;
        updateRichPresence(false);
    }
}

void Presence::updateGameEndTime2CCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_gameendtime2 {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    float local_gameEndTime2;
    if (std::strcmp(args.Arg(1), "null") == 0 || std::strcmp(map, "mp_npe") == 0) // we don't want end time for training
        local_gameEndTime2 = -FLT_MAX;
    else
        local_gameEndTime2 = std::stof(args.Arg(1));

    if (local_gameEndTime2 != -FLT_MAX)
        local_gameEndTime2 -= *serverClock;

    if (local_gameEndTime2 != gameEndTime) {
        spdlog::get("logger")->debug("Game end time updated");
        if (local_gameEndTime2 == -FLT_MAX)
            gameEndTime2 = INT_MIN;
        else
            gameEndTime2 = std::lround(std::ceilf(local_gameEndTime2)) + timestamp() + 1;
        updateRichPresence(false);
    }
}

void Presence::updateGameStateCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_game_state {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    gameState = atoi(args.Arg(1));
    if (gameState == SwitchingSides)
        switchedSides = true;
    updateRichPresence(false);
}

void Presence::updateIsRoundBasedCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_is_round_based {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    isRoundBased = atoi(args.Arg(1)) ? true : false;
}

void Presence::updateIsSwitchBasedCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_is_switch_sides_based {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    isSwitchSidesBased = atoi(args.Arg(1)) ? true : false;
}

void Presence::updateRoundsPlayedCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_rounds_played {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    int old_roundNumber = roundNumber;
    roundNumber = atoi(args.Arg(1));
    if (old_roundNumber != roundNumber)
        updateRichPresence(false);
}

void Presence::updateRoundsTotalCCommand(const CCommand& args)
{
    spdlog::get("logger")->debug("bme_update_rounds_total {}", args.ArgS());
    if (args.ArgC() <= 1) return;
    maxRounds = atoi(args.Arg(1));
}

HookedFuncStatic<void __fastcall, __int64> _updatePresence2("engine.dll", 0x15C4C0);
HookedFuncStatic<char, char*, signed __int64, const char*, char*, char*> _sub_180473500("engine.dll", 0x473500);
HookedFuncStatic<__int64 __fastcall, __int64> _sub_180022CA0("engine.dll", 0x22CA0);

Presence::Presence(ConCommandManager& conCommandManager)
{
    //auto enginedllBaseAddress = *engineClient->ba;
    auto enginedllBaseAddress = Util::GetModuleBaseAddress(_("engine.dll"));
    auto clientdllBaseAddress = Util::GetModuleBaseAddress(_("client.dll"));
    isConnectedAndInLobby = (int*)(enginedllBaseAddress + 0x79725C);
    teamNum = (int*)(enginedllBaseAddress + 0x797260);
    match_playlist = (const char*)(enginedllBaseAddress + 0x2EDABB0);
    map = (const char*)(enginedllBaseAddress + 0x79719C); // alt, faster (from cl_showfps???)
    map_old = (const char*)(clientdllBaseAddress + 0x16BC7E0);
    joinSecretInGame = (const char*)(enginedllBaseAddress + 0x2ECF03E);
    sessionIDBufferInGame = (char*)(enginedllBaseAddress + 0x2ECF03E); // congrats idiot, you added the same offset 2 times xD
    memset(richPresenceBuffer, 0, 1024);
    memset(presenceBuffer, 0, 1024);
    richPresenceBufferInGame = (char*)(enginedllBaseAddress + 0x2ECF13E);
    serverIPAndPortBufferInGame = (char*)(enginedllBaseAddress + 0x7B0437);
    isPresenceUpdatePending = (__int16*)(enginedllBaseAddress + 0x2ECF03C);
    richPresenceString = (void*)(enginedllBaseAddress + 0x2ECF13E);
    joinSecret = (void*)(enginedllBaseAddress + 0x2ECF03E); // sorry, thrice XD
    maxplayers = (int*)(enginedllBaseAddress + 0x7972A4);
    isPrivateLobby = (bool*)(enginedllBaseAddress + 0x210FE3C);
    serverClock = (float*)(clientdllBaseAddress + 0xF83FBC); // client+0xF83FBC=first to be populated, engine.dll+796D48=fastest, least delayed

    updatePresence2LastInput = 0;
    _updatePresence2.Hook(WRAPPED_MEMBER(Hook_updatePresence2));
    _sub_180473500.Hook(WRAPPED_MEMBER(Hook_sub_180473500));
    //_sub_180022CA0.Hook(WRAPPED_MEMBER(Hook_sub_180022CA0));
    didOriginOfflineKickAlready = false;

    trainingStage = -1;
    playerCount = 1;
    gameEndTime = 0;
    gameEndTime2 = 0;
    gameStartTime = timestamp();
    gameState = 0;

    conCommandManager.RegisterCommand("bme_update_rich_presence", WRAPPED_MEMBER(updateRichPresenceCCommand), "Update rich presence now", 0);
    conCommandManager.RegisterCommand("bme_npe_set_training_stage", WRAPPED_MEMBER(SetTrainingResumeChoice), "Set current training stage (for presence)", 0);
    conCommandManager.RegisterCommand("bme_update_player_count", WRAPPED_MEMBER(updatePlayerCountCCommand), "Set current player count (for presence)", 0);
    conCommandManager.RegisterCommand("bme_update_player_count_grab", WRAPPED_MEMBER(updatePlayerCountGrabCCommand), "Update current player count (for presence)", 0);
    conCommandManager.RegisterCommand("bme_update_gameendtime", WRAPPED_MEMBER(updateGameEndTimeCCommand), "Update game end time (in-game)", 0);
    conCommandManager.RegisterCommand("bme_update_gameendtime2", WRAPPED_MEMBER(updateGameEndTime2CCommand), "Update game start time (lobby countdown), uses server time to calculate", 0);
    conCommandManager.RegisterCommand("bme_update_game_state", WRAPPED_MEMBER(updateGameStateCCommand), "for presence", 0);
    conCommandManager.RegisterCommand("bme_update_is_round_based", WRAPPED_MEMBER(updateIsRoundBasedCCommand), "for presence", 0);
    conCommandManager.RegisterCommand("bme_update_is_switch_sides_based", WRAPPED_MEMBER(updateIsSwitchBasedCCommand), "for presence", 0);
    conCommandManager.RegisterCommand("bme_update_rounds_played", WRAPPED_MEMBER(updateRoundsPlayedCCommand), "for presence", 0);
    conCommandManager.RegisterCommand("bme_update_rounds_total", WRAPPED_MEMBER(updateRoundsTotalCCommand), "for presence", 0);
}

void __fastcall Presence::Hook_updatePresence2(__int64 a1) {

    updatePresence2LastInput = a1;
    strncpy(presenceBuffer, (const char*)a1, 1024);
    //std::cout << "Hi: " << (const char*)a1 << std::endl;

    //CreateThread(0, 0, updatePresence2_thread, 0, 0, NULL);

    //updateRichPresence(!canJoin || !a1);
    updateRichPresence(!a1);

    //if (canJoin) {
        _updatePresence2(a1);
    //}
    //else {
    //    _updatePresence2((__int64)"");
    //}

}

char Presence::Hook_sub_180473500(char* a1, signed __int64 a2, const char* a3, char* a4, char* a5) {
    //return fp_sub_180473500(a1, a2, a3, ...);
    updateRichPresenceLoadingWithMap(a4, true);
    return _sub_180473500(a1, a2, a3, a4, a5);
}

__int64 __fastcall Presence::Hook_sub_180022CA0(__int64 a1) {
    // we don't care what this function is, but it's only being called at the end of challenge response packet, so it's a good hook
    didOriginOfflineKickAlready = false;
    updateRichPresenceLoading(true);
    return _sub_180022CA0(a1);
}
