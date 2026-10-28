#include "title_screen.h"

#include "../network/player_id.h"
#include "../network/server.h"
#include "../user.h"

void ui_title_screen_init(tenv* env) {}

// Homepage panel with the Android chat commands (!id, !idlist, !idforce).
// PC has no chat box, so each command is a button. Can be hidden with
// "Hide"; it comes back from Settings > "Player ID panel on homepage".
static void title_player_id_panel(tenv* env) {
  tuser_data* usr = env->usr;
  tcontext* ctx = env->ctx;
  ImGuiStyle* style = igGetStyle();

  if (!player_id_panel_visible()) return;

  float frame_height = igGetFrameHeight();
  float w = 380;
  float log_h = frame_height * 6;
  float h = style->WindowPadding.y * 2 + frame_height * 4 +
            style->ItemSpacing.y * 6 + log_h;
  float pad = style->WindowPadding.x;

  igSetCursorPos((ImVec2){pad, ctx->size[1] - h - pad});
  if (igBeginChild_Str("##player_id_panel", (ImVec2){w, h},
                       ImGuiChildFlags_Borders, ImGuiWindowFlags_None)) {
    float hide_w = 60;
    igSeparatorText("Player ID");
    igSameLine(w - hide_w - style->WindowPadding.x, 0);
    if (igButton("Hide", (ImVec2){hide_w, 0})) player_id_set_panel_visible(false);

    char pid_hex[PLAYER_ID_LEN + 1];
    player_id_hex(pid_hex);
    igSetNextItemWidth(-(60 + style->ItemSpacing.x));
    igInputText("##home player id", pid_hex, sizeof(pid_hex),
                ImGuiInputTextFlags_ReadOnly, NULL, NULL);
    igSameLine(0, -1);
    if (igButton("Copy", (ImVec2){60, 0})) igSetClipboardText(pid_hex);

    float bw = (w - style->WindowPadding.x * 2 - style->ItemSpacing.x * 2) / 3;
    if (igButton("!id", (ImVec2){bw, 0})) player_id_handle_command("!id");
    igSameLine(0, -1);
    if (igButton("!idlist", (ImVec2){bw, 0}))
      player_id_handle_command("!idlist");
    igSameLine(0, -1);
    char force_label[24];
    snprintf(force_label, sizeof force_label, "!idforce: %s",
             player_id_force_enabled() ? "ON" : "OFF");
    if (igButton(force_label, (ImVec2){bw, 0}))
      player_id_handle_command("!idforce");

    if (igBeginChild_Str("##player_id_log", (ImVec2){-1, log_h},
                         ImGuiChildFlags_Borders, ImGuiWindowFlags_None)) {
      igPushTextWrapPos(0);
      for (int i = 0; i < player_id_log_count(); i++)
        igTextColored((ImVec4){1, 1, 1, 0.75f}, "%s", player_id_log_get(i));
      igPopTextWrapPos();
      if (player_id_log_take_dirty()) igSetScrollHereY(1.0f);
    }
    igEndChild();
  }
  igEndChild();
}

void ui_title_screen(tenv* env) {
  tuser_data* usr = env->usr;
  tcontext* ctx = env->ctx;
  user_settings* usrs = &usr->usrs;
  ImGuiStyle* style = igGetStyle();
  ImGuiIO* io = igGetIO_Nil();
  game_data* gdata = &usr->gdata;
  
  // version
  char version_str[16] = {0};
  sprintf(version_str, "v%s", APP_VERSION);
  ImVec2 vtxtsz; igCalcTextSize(&vtxtsz, version_str, NULL, false, -1);
  igSetCursorPosX(ctx->size[0] - vtxtsz.x - style->WindowPadding.x);
  igPushFont(usr->imgui_data.regular_font[FONT_SIZE_SMALL],
             usr->imgui_data.regular_font[FONT_SIZE_SMALL]->LegacySize);
  igTextColored((ImVec4){0.168f, 0.668f, 0.375f, 1}, version_str);
  igPopFont();

  igPushFont(usr->imgui_data.regular_font[usrs->ui_font_size],
             usr->imgui_data.regular_font[usrs->ui_font_size]->LegacySize);

  usr->r->global.bg_opacity = 0;
  usr->r->global.bd_opacity = 0;
  usr->r->global.minimap_opacity = 0;

  float frame_height = igGetFrameHeight();

  float logo_size = 400;
  float logo_gap = 5;

  igPushFont(usr->imgui_data.mono_font[usrs->ui_font_size],
             usr->imgui_data.mono_font[usrs->ui_font_size]->LegacySize);
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f -
                  (igGetFrameHeight() - style->ItemSpacing.x) * 3);
  int tot_sec = (int)usrs->play_time;
  int hours = tot_sec / 3600;
  int minutes = (tot_sec % 3600) / 60;
  int seconds = tot_sec % 60;
  igTextColored((ImVec4){1, 1, 1, 0.5f}, "\ue99e");
  igSameLine(0, -1);
  igTextColored((ImVec4){1, 1, 1, 0.5f}, "%d", usrs->score);
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f -
                  (igGetFrameHeight() - style->ItemSpacing.x) * 2);
  igTextColored((ImVec4){1, 1, 1, 0.5f}, "\ueaeb");
  igSameLine(0, -1);
  igTextColored((ImVec4){1, 1, 1, 0.5f}, "%d", usrs->kills);
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f -
                  (igGetFrameHeight() - style->ItemSpacing.x));
  igTextColored((ImVec4){1, 1, 1, 0.6}, "\ue952");
  igSameLine(0, -1);
  igTextColored((ImVec4){1, 1, 1, 0.6}, "%02d:%02d:%02d", hours, minutes,
                seconds);
  igPopFont();

  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f + style->ItemSpacing.y);
  igPushItemWidth(logo_size);
  igPushStyleColor_Vec4(ImGuiCol_FrameBg,
                        (ImVec4){0.297f, 0.265f, 0.484f, 1.0f});
  igInputTextWithHint("##nickname_input", "Nickname", usrs->nickname,
                      MAX_NICKNAME_LEN + 1, ImGuiInputTextFlags_None, NULL,
                      NULL);
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f + style->ItemSpacing.y * 2 +
                  frame_height);
  igInputTextWithHint("##ipv4_input", "IPv4:Port", usrs->ipv4, MAX_IPV4_LEN + 1,
                      ImGuiInputTextFlags_None, NULL, NULL);
  igPopStyleColor(1);
  igPopItemWidth();
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f + style->ItemSpacing.y * 3 +
                  frame_height * 2);

  if (igButton("\uea1c Play", (ImVec2){logo_size})) {
    usr->gdata.conn = CONNECTING;
    usr->gdata.curr_screen = PLAYING;
    glfwSetTime(0);
    server_connect(env);
  }
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f + style->ItemSpacing.y * 4 +
                  frame_height * 3);
  if (igButton("\ue90c Skin editor",
               (ImVec2){logo_size / 2 - style->ItemSpacing.x / 2}))
    usr->gdata.curr_screen = SKIN_EDITOR;
  igSameLine(0, -1);
  if (igButton("\ue991 Settings",
               (ImVec2){logo_size / 2 - style->ItemSpacing.x / 2})) {
    usr->gdata.curr_screen = SETTINGS;
  }
  igSetCursorPosX(ctx->size[0] / 2.0f - logo_size / 2);
  igSetCursorPosY(ctx->size[1] / 2.0f + style->ItemSpacing.y * 5 +
                  frame_height * 4);
  if (igButton("\ue9b6 Quit", (ImVec2){logo_size})) {
    env->config.running = false;
    save_user_settings(usrs);
  }

  title_player_id_panel(env);

  igPopFont();
}

void ui_title_screen_destroy(tenv* env) {}
