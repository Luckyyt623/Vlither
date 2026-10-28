#include "player_id.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../external/mongoose.h"

#define PLAYER_ID_FILE "player_id.dat"
#define PLAYER_ID_CFG_FILE "player_id.cfg"

// Servers that receive the player ID packet.
// Same list as the Android build, plus the PC default event server.
static const char* const id_servers[] = {
    "15.204.212.200:444",
    "81.169.165.225:444",
    "81.169.165.225:446",
    "45.77.5.207:444",
    "45.77.5.207:443",
    "139.84.170.60:444",
    "139.84.170.60:443",
    "45.76.102.107:444",
    "45.76.102.107:443",
    "216.128.169.33:444",
    "216.128.169.33:443",
};
#define ID_SERVER_COUNT (int)(sizeof(id_servers) / sizeof(id_servers[0]))

static char player_id[PLAYER_ID_LEN + 1];
static bool id_force = false;
static bool panel_visible = true;

static char log_lines[PLAYER_ID_LOG_LINES][PLAYER_ID_LOG_LEN];
static int log_count = 0;
static bool log_dirty = false;

static bool id_is_valid(const char* id) {
  if (!id || strlen(id) != PLAYER_ID_LEN) return false;
  for (int i = 0; i < PLAYER_ID_LEN; i++) {
    char c = id[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
  }
  return true;
}

static void generate(void) {
  unsigned char r[8];
  if (!mg_random(r, sizeof r)) {
    // fallback if OS entropy is unavailable
    for (int i = 0; i < 8; i++)
      r[i] = (unsigned char)(rand() ^ (time(NULL) >> (i % 8)));
  }
  snprintf(player_id, sizeof player_id, "%02x%02x%02x%02x%02x%02x%02x%02x",
           r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
}

static void load_cfg(void) {
  panel_visible = true;
  FILE* f = fopen(PLAYER_ID_CFG_FILE, "rb");
  if (!f) return;
  int c = fgetc(f);
  fclose(f);
  if (c == '0') panel_visible = false;
}

static void save_cfg(void) {
  FILE* f = fopen(PLAYER_ID_CFG_FILE, "wb");
  if (!f) return;
  fputc(panel_visible ? '1' : '0', f);
  fclose(f);
}

void player_id_init(void) {
  load_cfg();

  player_id[0] = 0;
  FILE* f = fopen(PLAYER_ID_FILE, "rb");
  if (f != NULL) {
    char buf[PLAYER_ID_LEN + 1] = {0};
    size_t n = fread(buf, 1, PLAYER_ID_LEN, f);
    fclose(f);
    // Old PC builds stored 16 raw random bytes here; those fail this check
    // and a new Android-style ID is generated below.
    if (n == PLAYER_ID_LEN && id_is_valid(buf)) {
      memcpy(player_id, buf, sizeof player_id);
      return;
    }
  }

  generate();
  f = fopen(PLAYER_ID_FILE, "wb");
  if (f != NULL) {
    fwrite(player_id, 1, PLAYER_ID_LEN, f);
    fclose(f);
  }
}

const uint8_t* player_id_get(void) { return (const uint8_t*)player_id; }

void player_id_hex(char out[PLAYER_ID_LEN + 1]) {
  memcpy(out, player_id, PLAYER_ID_LEN);
  out[PLAYER_ID_LEN] = '\0';
}

static bool server_in_list(const char* ipv4) {
  if (!ipv4) return false;
  for (int i = 0; i < ID_SERVER_COUNT; i++)
    if (strcmp(ipv4, id_servers[i]) == 0) return true;
  return false;
}

bool server_uses_player_id(const char* ipv4) {
  return id_force || server_in_list(ipv4);
}

bool player_id_force_enabled(void) { return id_force; }

static void log_add(const char* text) {
  if (!text || !text[0]) return;
  if (log_count == PLAYER_ID_LOG_LINES) {
    memmove(log_lines[0], log_lines[1],
            (PLAYER_ID_LOG_LINES - 1) * PLAYER_ID_LOG_LEN);
    log_count--;
  }
  snprintf(log_lines[log_count++], PLAYER_ID_LOG_LEN, "%s", text);
  log_dirty = true;
}

int player_id_log_count(void) { return log_count; }

const char* player_id_log_get(int i) {
  return (i >= 0 && i < log_count) ? log_lines[i] : "";
}

void player_id_log_clear(void) {
  log_count = 0;
  log_dirty = false;
}

bool player_id_log_take_dirty(void) {
  bool d = log_dirty;
  log_dirty = false;
  return d;
}

static bool is_cmd(const char* input, const char* cmd, size_t len) {
  return !strncmp(input, cmd, len) &&
         (!input[len] || isspace((unsigned char)input[len]));
}

bool player_id_handle_command(const char* input) {
  if (!input) return false;
  while (isspace((unsigned char)*input)) ++input;

  if (is_cmd(input, "!idlist", 7)) {
    char msg[PLAYER_ID_LOG_LEN];
    snprintf(msg, sizeof msg, "Player-ID servers (%s):",
             id_force ? "idforce is ON, sending to all"
                      : "sending to these only");
    log_add(msg);
    for (int i = 0; i < ID_SERVER_COUNT; i++) log_add(id_servers[i]);
    return true;
  }

  if (is_cmd(input, "!idforce", 8)) {
    id_force = !id_force;
    log_add(id_force
                ? "Player-ID packet: forced on for every server (dev mode, "
                  "not saved)."
                : "Player-ID packet: back to using the server list only.");
    return true;
  }

  if (is_cmd(input, "!id", 3)) {
    char msg[PLAYER_ID_LOG_LEN];
    snprintf(msg, sizeof msg, "Your player ID: %s", player_id);
    log_add(msg);
    return true;
  }

  return false;
}

bool player_id_panel_visible(void) { return panel_visible; }

void player_id_set_panel_visible(bool visible) {
  if (panel_visible == visible) return;
  panel_visible = visible;
  save_cfg();
}
