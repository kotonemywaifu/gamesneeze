#include "../../includes.hpp"
#include "hooks.hpp"
#include <cstring>

Hooks::Events::EventListener::EventListener() {
    Interfaces::eventManager->AddListener(this, "player_hurt", false);
    Interfaces::eventManager->AddListener(this, "player_death", false);
    Interfaces::eventManager->AddListener(this, "bullet_impact", false);
    Interfaces::eventManager->AddListener(this, "vote_cast", false);
    Interfaces::eventManager->AddListener(this, "round_start", false);
    Interfaces::eventManager->AddListener(this, "round_freeze_end", false);
}

Hooks::Events::EventListener::~EventListener() {
    Interfaces::eventManager->RemoveListener(this);
}

void Hooks::Events::EventListener::FireGameEvent(IGameEvent *event) {
  auto name = event->GetName();
  if (strstr(name, "player_hurt")) {
    Player *attacker = (Player *)Interfaces::entityList->GetClientEntity(
        Interfaces::engine->GetPlayerForUserID(event->GetInt("attacker")));
    Player *victim = (Player *)Interfaces::entityList->GetClientEntity(
        Interfaces::engine->GetPlayerForUserID(event->GetInt("userid")));
    if (attacker && victim && attacker == Globals::localPlayer) {
      player_info_t info;
      if (!Interfaces::engine->GetPlayerInfo(victim->index(), &info)) {
        return;
      }

      if (CONFIGBOOL("Misc>Misc>Hitmarkers>Hitlogs")) {
        if (CONFIGBOOL("Legit>Backtrack>Backtrack") &&
            Features::Backtrack::lastBacktrack > 4) {
          Features::Notifications::addNotification(
              ImColor(220, 220, 40), "backtracked %s %i ticks for %i health",
              info.name, Features::Backtrack::lastBacktrack,
              event->GetInt("dmg_health"));
        } else {
          Features::Notifications::addNotification(
              ImColor(220, 220, 40), "hit %s for %i health", info.name,
              event->GetInt("dmg_health"));
        }
      }

      if (CONFIGBOOL("Misc>Misc>Hitmarkers>Hitmarkers")) {
        Features::Hitmarkers::drawHitmarkerTill =
            Interfaces::globals->curtime + 0.7f;
      }

      if (CONFIGBOOL("Misc>Misc>Hitmarkers>Hitsound")) {
        Interfaces::engine->ExecuteClientCmd(
            "play buttons/arena_switch_press_02"); // TODO: play sound via a
                                                   // better method
      }

      if (CONFIGBOOL("Misc>Misc>Hitmarkers>Damage Markers")) {
        Features::Hitmarkers::DamageMarker damageMarker;
        damageMarker.drawHitmarkerTill = Interfaces::globals->curtime + 4.f;
        damageMarker.headshot = event->GetInt("hitgroup") == HITGROUP_HEAD;
        damageMarker.damage = event->GetInt("dmg_health");
        switch (event->GetInt("hitgroup")) {
        case HITGROUP_HEAD:
          damageMarker.position = victim->getBonePos(8);
          break;
        case HITGROUP_CHEST:
          damageMarker.position = victim->getBonePos(6);
          break;
        case HITGROUP_STOMACH:
          damageMarker.position = victim->getBonePos(4);
          break;
        default:
          damageMarker.position = victim->getBonePos(3);
          break;
        }
        Features::Hitmarkers::damageMarkers.push_back(damageMarker);
      }
    }
  } else if (strstr(name, "player_death")) {
    Player *attacker = (Player *)Interfaces::entityList->GetClientEntity(
        Interfaces::engine->GetPlayerForUserID(event->GetInt("attacker")));
    Player *victim = (Player *)Interfaces::entityList->GetClientEntity(
        Interfaces::engine->GetPlayerForUserID(event->GetInt("userid")));
    if (attacker && victim && attacker == Globals::localPlayer) {
      player_info_t info;
      if (!Interfaces::engine->GetPlayerInfo(victim->index(), &info)) {
        return;
      }

      Globals::kills++;

      if (CONFIGBOOL("Misc>Misc>Hitmarkers>Hitlogs")) {
        Features::Notifications::addNotification(ImColor(220, 40, 40),
                                                 "killed %s", info.name);
      }

      if (CONFIGBOOL("Misc>Misc>Kill Spam")) {
        std::ostringstream ss;
        ss << "say \"L " << info.name << " imagine getting killed by an open-source cheat. ghub/kotonemywaifu/gamesneeze\""; 
        Interfaces::engine->ExecuteClientCmd(ss.str().c_str());
      }
    }
  } else if (strstr(name, "vote_cast")) {
    Features::VoteRevealer::event(event);
  } else if (strstr(name, "round_start")) {
    Globals::freezeTime = 0.f;
  } else if (strstr(name, "round_freeze_end")) {
    Globals::freezeTime = Interfaces::globals->realtime;
  }
  Features::BulletTracers::event(event);
}

int Hooks::Events::EventListener::GetEventDebugID() {
    return EVENT_DEBUG_ID_INIT;
}