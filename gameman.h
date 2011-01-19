
#ifndef GAMEMAN_H
#define GAMEMAN_H

#define GM_PLAY_WIDTH	40
#define GM_PLAY_HEIGHT	24

struct shell;

int gm_init(void);
void gm_exit(void);

void gm_connect_shell(struct shell *sh);

#endif
