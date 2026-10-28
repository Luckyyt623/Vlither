#ifndef PLAYER_ID_H
#define PLAYER_ID_H

#include <stdbool.h>
#include <stdint.h>

// Same format as the Android build: 16 lowercase hex characters
// (8 random bytes), sent to the server as 16 ASCII bytes after opcode 67.
#define PLAYER_ID_LEN 16
#define PLAYER_ID_PACKET 67

#define PLAYER_ID_LOG_LINES 32
#define PLAYER_ID_LOG_LEN 128

// Load the ID from disk, or generate + save it on first launch.
void player_id_init(void);

// 16 ASCII hex bytes (NOT NUL terminated) - this is what goes on the wire.
const uint8_t* player_id_get(void);

// 16 hex chars + NUL, for copy-paste (give to server host).
void player_id_hex(char out[PLAYER_ID_LEN + 1]);

// True if the ID packet should be sent to this server ("ip:port"):
// it is on the ID server list, or !idforce is on.
bool server_uses_player_id(const char* ipv4);

// Runs one of the Android chat commands: "!id", "!idlist", "!idforce".
// Output goes to the log below. Returns false if input is not an ID command.
bool player_id_handle_command(const char* input);

// !idforce state (dev switch, not saved - same as Android).
bool player_id_force_enabled(void);

// Output of the commands above, oldest line first.
int player_id_log_count(void);
const char* player_id_log_get(int i);
void player_id_log_clear(void);
// True once after a new line was added (lets the UI scroll to the bottom).
bool player_id_log_take_dirty(void);

// Homepage panel visibility (saved in player_id.cfg).
bool player_id_panel_visible(void);
void player_id_set_panel_visible(bool visible);

#endif
