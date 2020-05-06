#include "resolverNimbus.h"

#include "../Utils/entity.h"
#include "../Utils/math.h"
#include "../Utils/xorstring.h"
#include "../interfaces.h"
#include "../settings.h"
#include "antiaim.h"

std::vector<int64_t> ResolverNimbus::Players = {};
std::vector<std::pair<C_BasePlayer *, QAngle>> player_data_nimbus;

static float NormalizeAsYaw(float flAngle)
{
	if (flAngle > 180.f || flAngle < -180.f)
	{
		auto revolutions = round(abs(flAngle / 360.f));

		if (flAngle < 0.f)
			flAngle += 360.f * revolutions;
		else
			flAngle -= 360.f * revolutions;
	}

	return flAngle;
}

void ResolverNimbus::FrameStageNotify(ClientFrameStage_t stage)
{
	if (!engine->IsInGame())
		return;

	C_BasePlayer *localplayer = (C_BasePlayer *)entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer)
		return;

	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < engine->GetMaxClients(); ++i)
		{
			C_BasePlayer *player = (C_BasePlayer *)entityList->GetClientEntity(i);

			if (!player 
			|| player == localplayer 
			|| player->GetDormant() 
			|| !player->GetAlive() 
			|| player->GetImmune() 
			|| Entity::IsTeamMate(player, localplayer))
				continue;

			IEngineClient::player_info_t entityInformation;
			engine->GetPlayerInfo(i, &entityInformation);

			if (Entity::IsTeamMate(player, localplayer))
				continue;

			player_data_nimbus.push_back(std::pair<C_BasePlayer *, QAngle>(player, *player->GetEyeAngles()));

			/*
			cvar->ConsoleColorPrintf(ColorRGBA(64, 0, 255, 255), XORSTR("\n[Nimbus] "));
			cvar->ConsoleDPrintf("Debug log here!");
			*/

			// Tanner is a sex bomb, also thank you Stacker for helping us out!
			float lbyDelta = fabsf(NormalizeAsYaw(*player->GetLowerBodyYawTarget() - player->GetEyeAngles()->y));

			if (lbyDelta < 35)
				return;

			player->GetEyeAngles()->y += *player->GetLowerBodyYawTarget();
		}
	}
	else if (stage == ClientFrameStage_t::FRAME_RENDER_END)
	{
		for (unsigned long i = 0; i < player_data_nimbus.size(); i++)
		{
			std::pair<C_BasePlayer *, QAngle> player_aa_data = player_data_nimbus[i];
			*player_aa_data.first->GetEyeAngles() = player_aa_data.second;
		}

		player_data_nimbus.clear();
	}
}

void ResolverNimbus::FireGameEvent(IGameEvent *event)
{
	if (!event)
		return;

	if (strcmp(event->GetName(), XORSTR("player_connect_full")) != 0 && strcmp(event->GetName(), XORSTR("cs_game_disconnected")) != 0)
		return;

	if (event->GetInt(XORSTR("userid")) && engine->GetPlayerForUserID(event->GetInt(XORSTR("userid"))) != engine->GetLocalPlayer())
		return;

	ResolverNimbus::Players.clear();
}