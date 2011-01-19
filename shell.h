
#ifndef SHELL_H
#define SHELL_H

#define SHELL_FAILURE	(-1)
#define SHELL_SUCCESS	(1)

#define SHELL_NOT_FOUND		(-100)
#define SHELL_NO_MEM		(-101)

struct shell;

typedef int shell_cmd(struct shell *sh, int ac, char *av[]);

#ifdef __cplusplus
extern "C" {
#endif

struct shell *shell_create(int x, int y, int w, int h);
void shell_destroy(struct shell *sh);

int shell_get_width(struct shell *sh);
int shell_get_height(struct shell *sh);

int shell_add_cmd(struct shell *sh, const char *name, shell_cmd *fn);
int shell_remove_cmd(struct shell *sh, const char *name);

int shell_flush(struct shell *sh);
int shell_puts(struct shell *sh, const char *buf);
int shell_printf(struct shell *sh, const char *buf, ...);
int shell_gets(struct shell *sh, char *buf, int siz);

int shell_add_default_cmds(struct shell *sh);
int shell_exec(struct shell *sh, const char *name, int ac, char *av[]);
int shell_exec_line(struct shell *sh, const char *line);
int shell_exec_linef(struct shell *sh, const char *line, ...);
int shell_enter(struct shell *sh);

#ifdef __cplusplus
}
#endif

#endif
