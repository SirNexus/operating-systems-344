#include "smallsh.h"

// Main function that runs functionality of code
// @return: exit status of function
int main() {
	char *command;
	char *arguments[512];
	int numArgs;
	int i;
	int prevStatus = -1;
	int spawnPID = -1;
	int spawnPIDs[128];
	int numPIDs = 0;

	// Set up signal handlers for both SIGINT and SIGTSTP
	// SIGINT is ignored for parent and SIGTSTP disables
	// Background processes
	struct sigaction SIGINT_action = {0};
	struct sigaction SIGTSTP_action = {0};
	SIGINT_action.sa_handler = catchSIGINT;
	SIGTSTP_action.sa_handler = catchSIGTSTP;

	// Add handling of SIGINT
	sigemptyset(&SIGINT_action.sa_mask);
	sigaddset(&SIGINT_action.sa_mask, SIGINT);
	sigaction(SIGINT, &SIGINT_action, NULL);

	// Add handling of SIGTSTP
	sigemptyset(&SIGTSTP_action.sa_mask);
	sigaddset(&SIGTSTP_action.sa_mask, SIGTSTP);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// Bulk of program. Loop indefinitely until the program exits
	while (1){
		command = (char *) malloc (sizeof(char) * 2048);
		// length gets number of characters received from stdin
		getLine(&command);
		// If first character is a pound sign, ignore rest of functionality
		// for this command
		if (command[0] != '#'){
			expandCommand(&command);

			// Tokenize the input from the user into separate arguments compatible with
			// execvp() command
			numArgs = tokenizeCommand(command, arguments);

			if (arguments[0] != NULL){
				if (!builtInCommands(arguments, &prevStatus, numArgs, spawnPIDs, numPIDs)){
					// If foreground argument, spawn process and wait for it to be done
					if (strcmp(arguments[numArgs - 1], "&") != 0 || foregroundOnly){
						// If running in foreground-only mode and '&' character is present,
						// Remove '&' character
						if (strcmp(arguments[numArgs - 1], "&") == 0 && foregroundOnly){
							arguments[numArgs - 1] = NULL;
							numArgs--;
						}
						// Fork child process
						forkProcess(arguments, &prevStatus, numArgs, spawnPIDs, &numPIDs, false);
					}
					else {
						// If background argument call, spawn process in the background
						// And remove trailing '&' character
						arguments[numArgs - 1] = NULL;
						numArgs--;
						forkProcess(arguments, &prevStatus, numArgs, spawnPIDs, &numPIDs, true);
					}
				}
			}
		}

		// Check background processes to see if they've exited yet
		checkBackground(spawnPIDs, &numPIDs, &prevStatus);
		// Free *command
		free(command);
	}
	// Lost 10 points for this last time! Ain't happenin again
	return 0;
}

// Function for catching SIGINT
// @return: none
void catchSIGINT(int signo){
	char *message = "\n";
	write(STDOUT_FILENO, message, 1);
}

// Function for catching SIGTSP
// @return: none
void catchSIGTSTP(int signo){
	if (foregroundOnly){
		// If running in foreground only mody, print that
		// program is switching to foreground-only
		char *message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
	}
	else {
		// If not running in foreground-only mode, print that program
		// is switching to foreground-only mode
		char *message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
	}
	foregroundOnly = !foregroundOnly;
}
// Gets user input, then removes the newline at the end
// @return: number of characters inputted
int getLine(char **command){
	size_t len = 0;
	while (1){
		printf(": ");
		fflush(stdout);
		// Get user input
		len = getline(command, &len, stdin);
		if (len == -1){
			clearerr(stdin);
		}
		else
			break;
	}
	// Replace end newline with space to be removed by tokenizer
	(*command)[strcspn((*command), "\n")] = ' ';
	return len;
}

// Expand input string to include the process ID anywhere
// it has "$$"
// @return: none
void expandCommand(char **command){
	char *tempStr = (char *) malloc ((strlen(*command) + 60) * sizeof(char));
	char strPID[20];
	int i = 0;
	int k = 0;

	// convert current pid to string
	sprintf(strPID, "%d", getpid());
	for (i=0; i<strlen(*command); i++){
		// If "$$" substring is found, add it to parent string
		if ((*command)[i] == '$' && (*command)[i + 1] == '$'){
			strcat(tempStr, strPID);
			k += (strlen(strPID) - 1);
			i++;
		}
		else
			// Else copy over string letter by letter
			tempStr[k] = (*command)[i];
		k++;
	}
	// Free old command and replace with new string
	free(*command);
	*command = tempStr;
}

// Tokenize input deliminated by spaces
// @return: number of elements in array
int tokenizeCommand(char *command, char **arguments){
	int i = 0;
	arguments[i] = strtok(command, " ");
	while (arguments[i] != NULL){
		i++;
		arguments[i] = strtok(NULL, " ");
	}
	return i;
}

// Built-in function to exit the current program
// Kills all processes that are currently running in the background
// @return: none
void exitProgram(int *spawnPIDs, int numPIDs){
	int i;
	for (i=0; i<numPIDs; i++){
		kill(spawnPIDs[i], SIGKILL);
	}
	exit(0);
}

// Built-in function to change the current directory
// @return: none
int changeDirectory(char *argument, int numArgs){
	int status;
	if (numArgs == 1){
		// If no argument supplied, change to HOME directory
		status = chdir(getenv("HOME"));
	}
	else
		// Else change to specified argument directory
		status = chdir(argument);
	return status * -1;
}

// Built-in command to print status of last command
// Exit value if 0 or 1, signal if other
// @return: none
void printStatus(int prevStatus){
	if (prevStatus == 0 || prevStatus == 1){
		printf("exit value %i\n", prevStatus);
	}
	else
		printf("terminated by signal %i\n", prevStatus);
}

// Built-in command function that calls all built-in commands
// @return: true if built-in command was run, false otherwise
bool builtInCommands(char *arguments[512], int *prevStatus, int numArgs, int *spawnPIDs, int numPIDs){
	if (strcmp(arguments[0], "exit") == 0){
		// Call exit built-in command
		exitProgram(spawnPIDs, numPIDs);
	} 
	else if (strcmp(arguments[0], "cd") == 0){
		// Call change directory built-in command
		*prevStatus = changeDirectory(arguments[1], numArgs);
	}
	else if (strcmp(arguments[0], "status") == 0){
		// Call status built in command
		printStatus(*prevStatus);
	}
	else
		return false;
	return true;
}

// Function for forking child processes
// @return: none
void forkProcess(char *arguments[512], int *prevStatus, int numArgs, int *spawnPIDs, int *numPIDs, bool isBackground){
	int spawnPID;
	spawnPID = fork();
	// Switch statement that checks to see whether the spawned PID is
	// the child, parent, or error
	switch (spawnPID)
	{
	case -1:
		perror("Error Spawning Child!\n");
		break;
	case 0:
		// If child process, execute the command specified by the argument array
		executeCommand(arguments, prevStatus, numArgs, isBackground);
		break;
	default:
		// If parent process and child is not in the background, wait until it exits
		if (!isBackground){
			// Fork process, wait for process to exit
			while (waitpid(spawnPID, prevStatus, WNOHANG) == 0);
			// Get status of last run child process
			getStatus(prevStatus);
			if (*prevStatus > 1){
				printf("terminated by signal %i\n", *prevStatus);
			}
		}
		else {
			// Fork process, store process ID for waiting later
			printf("Background process ID: %i\n", spawnPID);
			spawnPIDs[(*numPIDs)] = spawnPID;
			(*numPIDs)++;
			break;
		}
		break;
	}
}

// Function for checking on the current background processes
// @return: none
void checkBackground(int *spawnPIDs, int *numPIDs, int *status){
	int i;
	int childPID;

	// For all current processes in the background
	for (i=0; i<(*numPIDs); i++){
		childPID = waitpid(spawnPIDs[i], status, WNOHANG);
		if (childPID == -1){
			// If return of waitpid() is -1, return error
			perror("Background process waitpid error");
			exit(EXIT_FAILURE);
		}
		if (childPID != 0){
			// If return of waitpid() is nonzero, process has exited
			// Get status of last exited process
			getStatus(status);
			printf("Process %d exited: ", childPID);
			printStatus(*status);
			removeElem(spawnPIDs, &i, numPIDs);
		}
	}
}

// Function for executing commands based off of arguments array
// @return: none
void executeCommand(char *arguments[512], int *prevStatus, int numArgs, bool isBackground){
	
	// Redirect input and output based on last arguments of arguments array
	redirectIO(arguments, isBackground);
	*prevStatus = execvp(arguments[0], arguments);
	// Rest of case only runs if exec fails to execute a command correctly
	perror("Error");
	exit(EXIT_FAILURE);
}

// Function to redirect input and output based on last arguments
// of arguments array
// @return: none
void redirectIO(char *arguments[512], bool isBackground){
	int input, output, result;
	int i = 0;

	// If running in the background, set default input and output
	// to /dev/null
	if (isBackground){
		signal(SIGINT, SIG_IGN);
		dup2(open("/dev/null", O_RDONLY), 0);
		dup2(open("/dev/null", O_WRONLY), 1);
	}
	// while the current argument is not null, check if the argument
	// is either '<' or '>'
	while (arguments[i] != NULL){
		if (strcmp(arguments[i], "<") == 0){
			// If found argument is matching with '<'
			// redirect stdin to argument after i
			input = open(arguments[i + 1], O_RDONLY);
			if (input == -1)
			{
				// Return error if file was not opened correctly
				perror("Opening input file");
				exit(1);
			}
			// Redirect input
			result = dup2(input, 0);
			if (result == -1)
			{
				// Return error if input was not redirected correctly
				perror("Redirecting input");
				exit(2);
			}
			arguments[i] = NULL;
		}
		else if (strcmp(arguments[i], ">") == 0){
			// If found argument is matching with '>'
			// redirect stdout to argument after i
			output = open(arguments[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (output == -1)
			{
				// Return error if file was not opened correctly
				perror("Opening output file");
				exit(1);
			}
			// Redirect output
			result = dup2(output, 1);
			if (result == -1)
			{
				// Return error if output was not redirected correctly
				perror("Redirecting output");
				exit(2);
			}
			arguments[i] = NULL;
		}
		i++;
	}
}

// Function to get the return code of the last executed process
// @return: none
void getStatus(int *status){
	if (WIFEXITED(*status) != 0){
		// If process exited normally, set exit status to
		// 0 or 1
		*status = WEXITSTATUS(*status);
	}
	else if (WIFSIGNALED(*status) != 0){
		// If process exited abnormally, set exit status to
		// Whatever signals terminated it
		*status = WTERMSIG(*status);
	}
}

// Function to remove an element of spawnPIDs array
// Used for removing processes in the background if they've
// terminated
// @return: none
void removeElem(int *spawnPIDs, int *remove, int *numPIDs){
	spawnPIDs[*remove] = spawnPIDs[(*numPIDs) - 1];
	(*numPIDs)--;
	(*remove)--;
}