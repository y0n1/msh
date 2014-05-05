/* File Name	:	myshell.c
 * Student IDs	:	318143120, 123456789
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define COMMAND_MAX_LENGTH 256
#define MAX_COMMANDS_COUNT 16
#define NULL_CHAR '\0'
#define AMP_CHAR '&'
#define AMP_STR "&"
#define SPACE_STR " "
#define CD_CMD_STR "cd"
#define EXIT_CMD_STR "exit"
#define COMMAND_PROMPT_STR "> "
#define GOOD_BYE_MSG "\nThank you for using my shell!\n"

typedef enum {
	false,
	true
} bool;

/**
 * Prints the current folder location and command line prompt.
 */
void show_cmd_prompt() {
	char *currDir = getcwd(0, 0);
	printf("%s%s", currDir, COMMAND_PROMPT_STR);
}

/**
 * Trims white spaces from the given string and
 * Returns the length of the resulting string.
 */
size_t trim_ws(char *i_String) {
	char *end;

	/* Trim leading spaces */
	while (isspace(*i_String)) {
		i_String++;
	}

	/* Trim trailing space */
	end = i_String + strlen(i_String) - 1;
	while (end > i_String && isspace(*end)) {
		end--;
	}

	/* Write new null terminator */
	*(end + 1) = 0;

	return strlen(i_String);
}

/**
 * Checks if the given string is not the empty string.
 */
bool is_not_empty_string(char *i_String) {
	return strcmp(i_String, SPACE_STR) != 0;
}

/**
 * Parses a string into tokens using the given delimiter.
 */
size_t get_tokens(char *i_FullCmd, char **o_CmdsArray) {
	char *token = NULL;
	size_t size = 0;

	token = strtok(i_FullCmd, AMP_STR);
	while (token != NULL) {
		trim_ws(token);
		if (is_not_empty_string(token)) {
			/* store each command in its own cell */
			o_CmdsArray[size] = malloc((strlen(token) + 1) * sizeof (char));
			strcpy(o_CmdsArray[size++], token);
		}
		/* retrieve the next token */
		token = strtok(NULL, AMP_STR);
	}

	/* append string terminator */
	o_CmdsArray[size] = NULL_CHAR;

	return size;
}

/**
 * Gets a full-command and returns only the command without its arguments.
 * This function allocates memory for the returned string!
 */
char *get_command(const char *c_Cmd) {
	char *buffer = malloc((strlen(c_Cmd) + 1) * sizeof(char));

	strcpy(buffer, c_Cmd);
	strtok(buffer, SPACE_STR);

	return buffer;
}

/**
 * Gets a full command and returns only the arguments without the command.
 * This function allocates memory for the returned string!
 */
char *get_arguments(const char *c_Cmd) {
	char *buffer = malloc((strlen(c_Cmd) + 1) * sizeof(char));

	strcpy(buffer, c_Cmd);
	while (isspace(*buffer)) {
		buffer++;
	}

	return buffer;
}

/**
 * Checks whether the given PID belongs to the parent or the child process.
 */
bool is_child(pid_t i_Pid) {
	return i_Pid == 0;
}

/**
 * Executes a process, possibly in background.
 */
int execute(const char **i_CmdsArray, bool i_Background) {
	pid_t pid;
	int status;

	pid = fork();
	if (is_child(pid)) {
		execvp(i_CmdsArray[0], i_CmdsArray);
		exit(EXIT_FAILURE);
	} else if (i_Background) {
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	} else {
		return EXIT_SUCCESS;
	}
}

/**
 * Detects the exit command.
 */
bool is_not_exit(const char *i_Cmd) {
	return strcmp(EXIT_CMD_STR, i_Cmd) != 0;
}

/**
 * Detects the 'cd' command
 */
bool is_chdir(const char *i_Cmd) {
	return strcmp(CD_CMD_STR, get_command(i_Cmd)) == 0;
}

/**
 * Reads the next command from the command line.
 */
size_t read_cmd(char *io_Buffer, bool *o_BgFlagStatus) {
	fgets(io_Buffer, COMMAND_MAX_LENGTH, stdin);
	trim_ws(io_Buffer);
	o_BgFlagStatus = has_bg_flag(io_Buffer);

	return strlen(io_Buffer);
}

/**
 * Checks if the last character in i_CmdBuffer is '&'. This function assumes
 * that the string i_CmdBuffer has been trimmed properly already.
 */
bool has_bg_flag(const char *i_CmdBuffer) {
	size_t length = strlen(i_CmdBuffer);
	char lastChar = *(i_CmdBuffer + length - 1);

	return strcmp(lastChar, AMP_CHAR) == 0;
}

/**
 * main function.
 */
int main() {
	bool bgFlag = false;
	size_t cmdsArraySize = 0;
	char *currentCommand = NULL;
	char fullCmd[COMMAND_MAX_LENGTH] = {NULL_CHAR};
	char *cmdsArray[MAX_COMMANDS_COUNT];
	char *args[COMMAND_MAX_LENGTH];
	int index = 0;

	// main shell loop.
	while (is_not_exit(fullCmd)) {

		// scans the users input
		show_cmd_prompt();

		// reads the command and sets the background flag
		read_cmd(fullCmd, &bgFlag);

		// taking the entire command into an array
		cmdsArraySize = get_tokens(fullCmd, cmdsArray);

		// process the parsed input of commands
		for (index = 0; index < cmdsArraySize; ++index) {
			currentCommand = cmdsArray[index];

			// checks if exit received.
			if (!is_not_exit(currentCommand)) {
				fullCmd = currentCommand;
				break;
			}

			// in case of change directory is called
			if (is_chdir(currentCommand)) {
				chdir(get_arguments(currentCommand));
			}

			// break the entire command to argv structure.
			get_tokens(currentCommand, args);

			// runs the command.
			execute(args, bgFlag);
		}
	}
	printf(GOOD_BYE_MSG);

	return EXIT_SUCCESS;
}
