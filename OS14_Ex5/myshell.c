/*
 * File Name	:	myshell.c
 * Name	        :	Ron Cohen
 * Student ID	:	301704557
 * Email     	:	cohen.ron1@idc.ac.il
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define COMMAND_PROMPT_STR "> "
#define COMMAND_MAX_LENGTH 255
#define MAX_COMMANDS_COUNT 16
#define NULL_CHAR '\0'
#define AMP_STR "&"
#define AMP_CHAR '&'
#define SPACE_STR " "
#define CD_CMD_STR "cd"
#define EXIT_CMD_STR "exit"
#define GOOD_BYE_MSG "\nThank you for using my shell!\n"

typedef enum {
	false,
	true
} bool;

/**
 * Prints the current folder location and command line prompt.
 */
void show_cmd_prompt() {
	char *curr_dir = getcwd(0, 0);
	printf("%s%s", curr_dir, COMMAND_PROMPT_STR);
}

/**
 * Trims white spaces from the given string and
 * Returns the length of the resulting string.
 */
size_t trim_ws(char *str) {
	char *end;

	/* Trim leading spaces */
	while (isspace(*str)) {
		str++;
	}

	/* Trim trailing space */
	end = str + strlen(str) - 1;
	while (end > str && isspace(*end)) {
		end--;
	}

	/* Write new null terminator */
	*(end + 1) = 0;

	return strlen(str);
}

/**
 * Parses a string into tokens using the given delimiter.
 */
size_t parse_command(char *in_FullCmd, char **output) {
	size_t size = 0;
	char *tempbuff = NULL;

	tempbuff = strtok(in_FullCmd, AMP_STR);
	while (tempbuff != NULL) {

		/* trim white spaces */
		trim_ws(tempbuff);

		/* store each command in its own cell */
		output[size] = malloc(strlen(tempbuff) * sizeof(char));
		strcpy(output[size], tempbuff);
		tempbuff = strtok(NULL, AMP_STR);
		size++;
	}

	/* writing NULL terminator */
	output[size] = NULL_CHAR;

	return size;
}

/**
 * Gets a full-command and returns only the command without its arguments.
 * This function allocates memory for the returned string!
 */
char *get_command(const char *command) {
	char *buff = malloc((strlen(command) + 1) * sizeof(char));

	strcpy(buff, command);
	strtok(buff, SPACE_STR);
	return buff;
}

/**
 * Gets a full command and returns only the arguments without the command.
 * This function allocates memory for the returned string!
 */
char *get_arguments(const char *command) {
	char *buff = malloc((strlen(command) + 1) * sizeof(char));

	strcpy(buff, command);
	while (isspace(*buff)) {
		buff++;
	}
	return buff;
}

/**
 * Checks whether the given PID belongs to the parent or the child process.
 */
bool is_child(pid_t pid) {
	return pid == 0;
}

/**
 * Executes a process, possibly in background.
 */
int execute(char **command, bool background) {
	pid_t pid;
	int status;

	pid = fork();
	if (is_child(pid)) {
		// Child
		execvp(command[0], command);
		exit(EXIT_FAILURE);
	} else if (background) {
		// Parent
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	} else {
		return EXIT_SUCCESS;
	}
}

/**
 * Detects the exit command.
 */
bool is_not_exit(char *cmd) {
	return strcmp(EXIT_CMD_STR, cmd) != 0;
}

/**
 * Detects the 'cd' command
 */
bool is_chdir(char *cmd) {
	return strcmp(CD_CMD_STR, get_command(cmd)) == 0;
}

/**
 * Reads the next command from the command line.
 */
size_t read_cmd(char *buffer) {
	fgets(buffer, COMMAND_MAX_LENGTH, stdin);
	trim_ws(buffer);

	return strlen(buffer);
}

bool has_bg_flag(char *buffer) {

}

/**
 * main function.
 */
int main() {
	bool bgFlag = false;
	size_t cmdsArraySize = 0;
	size_t length = 0;
	char *currentCommand = NULL;
	char fullCmd[COMMAND_MAX_LENGTH] = {NULL_CHAR};
	char *cmdsArray[MAX_COMMANDS_COUNT];
	char *args[COMMAND_MAX_LENGTH];
	int index = 0;

	// main shell loop.
	while (is_not_exit(fullCmd)) {

		// scans the users input
		show_cmd_prompt();

		// reads the command
		read_cmd(fullCmd);

		// checks if the last character is '&'
		bgFlag = has_bg_flag(fullCmd);

		// taking the entire command into an array
		cmdsArraySize = parse_command(fullCmd, cmdsArray);

		// process the parsed input of commands
		for (index = 0; index < cmdsArraySize; ++index) {
			currentCommand = cmdsArray[index];
			length = strlen(currentCommand);

			// in case of change directory is called
			if (is_chdir(currentCommand)) {
				chdir(get_arguments(currentCommand));
			}

			// break the entire command to argv structure.
			parse_command(currentCommand, args);

			// runs the command.
			execute(args, bgFlag);

			// checks if exit received.
			if (!is_not_exit(fullCmd)) {
				exit(EXIT_SUCCESS);
			}
		}
	}
	printf(GOOD_BYE_MSG);

	return EXIT_SUCCESS;
}
