#include <iostream>
#include <vector>
#include <sstream>
#include <exception>

#include <unordered_map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

const string welcomeMessage = "KindShell created by Baykaov Vladimir\n"
                              "Group M3234\n";

const string helpMessage = "Commands:\n"
                           "Help: Shows help,\n"
                           "Set <var_name>=<var_value>: Creates or rewrites variable,\n"
                           "Remove <var_name>: Removes variable,\n"
                           "Variables: Shows all variables,\n"
                           "Exit: Terminates program.\n";

const string unexpectedSetMessage = "Unexpected input.\n"
                                    "Expected: Set <var_name>=<var_value>\n";

const string unexpectedRemoveMessage = "Unexpected input.\n"
                                       "Expected: Remove <var_name>\n";

const string exitMessage = "Terminate program.";


unordered_map<string, string> environmentVariables;


vector<string> getTokens(stringstream &commandStream) {
    vector<string> result;
    string tmpToken;

    while (!commandStream.eof()) {
        getline(commandStream, tmpToken, ' ');
        result.push_back(tmpToken);
    }

    return result;
}


void printErrorMessage(const string &message) {
    cerr << message << endl;
}


void executeCommand(const vector<string> &tokens) {
    pid_t pid = fork();

    if (pid == -1) {
        printErrorMessage("Error occurred while forking");
    } else {
        if (pid == 0) {
            char *args[tokens.size() + 1];

            for (int i = 0; i < tokens.size(); ++i) {
                args[i] = (char *) tokens[i].c_str();
            }
            args[tokens.size()] = nullptr;

            char *variables[environmentVariables.size() + 1];

            int i = 0;
            for (auto &pair : environmentVariables) {
                variables[i] = (char *) (pair.first + "=" + pair.second).c_str();
                ++i;
            }
            variables[environmentVariables.size()] = nullptr;

            int executeResult = execve(args[0], args, variables);
            if (executeResult == -1) {
                printErrorMessage("Error occurred while executing process.");
                exit(EXIT_FAILURE);
            }

        } else {
            int result;
            pid_t childResult = waitpid(pid, &result, 0);

            if (childResult == -1) {
                printErrorMessage("Error occurred while executing child process.");
            } else {
                cout << "Process result: " << WEXITSTATUS(result) << endl;
            }
        }
    }
}


int main(int argc, char *argv[]) {
    cout << welcomeMessage << helpMessage;

    string command;
    stringstream commandStream;

    cout << ">> ";

    while (getline(cin, command)) {
        cout.flush();
        commandStream.clear();
        commandStream << command;

        vector<string> lineTokens = getTokens(commandStream);

        if (lineTokens.empty()) {
            continue;

        } else if (lineTokens[0] == "Exit" || cin.eof()) {
            cout << exitMessage;
            exit(0);

        } else if (lineTokens[0] == "Help") {
            cout << helpMessage;

        } else if (lineTokens[0] == "Set") {
            if (lineTokens.size() == 2) {
                string varDefinition = lineTokens[1];
                int equalSignPlace = varDefinition.find('=');

                if (equalSignPlace == -1) {
                    cout << unexpectedSetMessage;
                } else {
                    string varName = varDefinition.substr(0, equalSignPlace);
                    string varValue = varDefinition.substr(equalSignPlace + 1,
                                                           varDefinition.size() - equalSignPlace - 1);

                    environmentVariables[varName] = varValue;
                }

            } else {
                cout << unexpectedSetMessage;
            }

        } else if (lineTokens[0] == "Remove") {
            if (lineTokens.size() == 2) {
                string varName = lineTokens[1];

                environmentVariables.erase(varName);
            } else {
                cout << unexpectedRemoveMessage;
            }

        } else if (lineTokens[0] == "Variables") {
            if (environmentVariables.empty()) {
                cout << "There are no environment variables" << endl;
            } else {
                for (auto &variable : environmentVariables) {
                    cout << variable.first << " = " << variable.second << endl;
                }
            }

        } else {
            executeCommand(lineTokens);
        }

        cout << ">> ";
    }
}
