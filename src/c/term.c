#include <conf.h>

struct history {
	char       cmd[256];
	int        id;
};

static int             TERMINAL_MODE;
static int             nr_history = 0;
static struct history *history_table;

/*
 * Manage terminal command history
 */
static void
cmd_history_add(char *cmd)
{
	strncpy(history_table[nr_history].cmd, cmd, 255);
	history_table[nr_history].id = nr_history;
	nr_history++;
}

static struct history *
cmd_history_last_cmd(void)
{
	return &history_table[nr_history-1];
}

static void
cmd_history_print(void)
{
	for (int x=0; x<nr_history; x++)
		printf("%s\n", history_table[x].cmd);
}

/*
 * Terminal Loop
 */
void terminal_loop(void)
{
	FILE           *fp;
	struct termios  term;
	struct history *hist      = NULL;
	char            buf[256]  = {0};
	int             n = 0, key, pos = 0, noprompt = 0, cmd_mode = 0;

	history_table = zmalloc(sizeof(struct history)*4096);
	if (!history_table)
		exit(-1);

	tcgetattr(0, &term);
	memcpy(&sterm, &term, sizeof(term));
	term.c_lflag |= IGNPAR;
	term.c_lflag &= ~(ISTRIP | INLCR  | IGNCR | ICRNL | IXON  | IXANY | IXOFF);
	term.c_lflag &=	~(ISIG   | ICANON | ECHO  | ECHOE | ECHOK | ECHONL);
	term.c_lflag &= ~OPOST;
	term.c_cc[VMIN]  = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &term);

	for (;;) {
		term_noecho();
		switch (TERMINAL_MODE) {
			/**********************************
			 * QSHELL scripting command prompt
			 */
			case APC_MODE_WEBSCRIPT:
				printf(BLUE  "<QSH> " RESET);
				fflush(stdout);
				break;
			/*******************************************************
			 * For commands dealing with remotely fetching objects
			 * such as (HTML Tables, PDFs) and processing them
			 * there will be a "set" command for defining variables
			 */
			case APC_MODE_WWW:
				printf(GREEN "<WWW> " RESET);
				fflush(stdout);
				break;
			default:
				printf(BOLDRED "<#> " RESET);
				fflush(stdout);
		}
		noprompt = 0;
		key      = fs_readline_tty(buf+pos, &n);
		if (key == -1) {
			noprompt = 1;
			continue;
		}
		pos = n;
		switch (key) {
			case KEY_NONE: {
				noprompt = 1;
				continue;
			};
			case UP_KEY: {
				if (!nr_history) {
					noprompt = 1;
					continue;
				}
				if (hist) {
					if (!hist->id) {
						noprompt = 1;
						continue;
					}
					term_erase(strlen(hist->cmd));
					hist = &history_table[hist->id-1];
				}
				else
					hist = cmd_history_last_cmd();
				write(1, hist->cmd, strlen(hist->cmd));
				noprompt = 1;
				continue;
			};
			case DOWN_KEY: {
				if (!nr_history || !hist || (hist->id == nr_history)) {
					noprompt = 1;
					continue;
				}
				term_erase(strlen(hist->cmd));
				hist = &history_table[hist->id+1];
				write(1, hist->cmd, strlen(hist->cmd));
				noprompt = 1;
				if (!hist->id)
					hist = NULL;
				continue;
			};
			case BACKSPACE_KEY: {
				buf[pos] = 0;
				noprompt = 1;
				continue;
			};
			case KEY_ENTER: {
				if (!hist)
					break;
				cmd_history_add(hist->cmd);
				buf[n+pos] =0;
				break;
			}
			default:
				cmd_history_add(buf);
				break;
		}

		// can't remember why this is uncommented or what it does
//		tcsetattr(0,TCSAFLUSH,&sterm);
//		tcsetattr(0,TCSADRAIN,&sterm);

		if (hist) {
			apc_exec(hist->cmd);
			hist = NULL;
			continue;
		}

		/* Add Command to History */
		cmd_history_add(buf);

		/* Execute APC|LPC */
		apc_exec(buf);

		/* reset loop */
		pos = 0;
		n   = 0;
		memset(buf, 0, sizeof(buf));
	}
}