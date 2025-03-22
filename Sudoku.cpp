#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <conio.h>
#include <cstring>

using namespace std;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAX_SAVED_GAMES 100
#define MAX_LEADERBOARD_ENTRIES 100

struct SavedGame {
    char name[50];
    int board[9][9];
    int elapsedTime;
    int remainingTime;
};

struct LeaderboardEntry {
    char username[50];
    int score;
    int time;
    int win = 0;
    int lose = 0;
};

SavedGame savedGames[MAX_SAVED_GAMES];
int savedGamesCount = 0;

LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
int leaderboardCount = 0;

void clearScreen();
void displayMenu(const char*);
void displayDifficultyMenu();
bool isValidMove(const int[][9], int, int, int);
void displaySudoku(const int[][9], int, int, int, int);
void displayPauseScreen();
void displayEndScreen(bool, int, int, int, int);
void playSudoku(int[][9], int, const char*);
void displayLeaderboard();
bool isUsernameUnique(const char* username);
void bubbleSortLeaderboard();

void clearScreen() {
    system("cls");
}

void displayMenu(const char* username) {
    clearScreen();
    cout << GREEN << "In His Sublime Name" << RESET << "\n";
    cout << YELLOW << "Let's Play Sudoku" << RESET << "\n";
    cout << BLUE << "Game Options" << RESET << "\n\n";
    cout << "Welcome, " << username << "!\n";
    cout << RED << "1" << RESET << ": Start a New Game\n";
    cout << RED << "2" << RESET << ": Play a Saved Game\n";
    cout << RED << "3" << RESET << ": View Leaderboard\n";
    cout << RED << "4" << RESET << ": Exit\n";
    cout << "Choose an option: ";
}

void displayDifficultyMenu() {
    clearScreen();
    cout << "Select Difficulty Level:\n";
    cout << RED << "1" << RESET << ": Easy (5 minutes)\n";
    cout << RED << "2" << RESET << ": Medium (3 minutes)\n";
    cout << RED << "3" << RESET << ": Hard (2 minute)\n";
    cout << "Choose an option: ";
}

bool isValidMove(const int board[][9], int row, int col, int value) {
    for (int i = 0; i < 9; ++i) {
        if (board[row][i] == value || board[i][col] == value) return false;
    }

    int startRow = (row / 3) * 3;
    int startCol = (col / 3) * 3;
    for (int i = startRow; i < startRow + 3; ++i) {
        for (int j = startCol; j < startCol + 3; ++j) {
            if (board[i][j] == value) return false;
        }
    }

    return true;
}

void displaySudoku(const int board[][9], int cursorX, int cursorY, int elapsedTime, int totalTime) {
    clearScreen();
    cout << "Elapsed Time: " << elapsedTime / 60 << "m " << elapsedTime % 60 << "s | Total Time: " << totalTime / 60 << "m " << totalTime % 60 << "s\n\n";
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            if (i == cursorY && j == cursorX) {
                cout << "[";
            } else {
                cout << " ";
            }

            if (board[i][j] == 0)
                cout << ".";
            else
                cout << board[i][j];

            if (i == cursorY && j == cursorX) {
                cout << "]";
            } else {
                cout << " ";
            }

            if (j == 2 || j == 5) cout << "|";
        }
        cout << "\n";
        if (i == 2 || i == 5) cout << "---------+---------+---------\n";
    }
}

void displayPauseScreen() {
    clearScreen();
    cout << "Game Paused. Press ESC to resume, S to save the game, or Q to quit.\n";
}

void displayEndScreen(bool won, int correctCells, int incorrectCells, int elapsedTime, int score) {
    clearScreen();
    if (won) {
        cout << GREEN << "Congratulations! You won the game!" << RESET << "\n";
    } else {
        cout << RED << "Game Over. You lost." << RESET << "\n";
    }

    cout << "Correct Cells: " << correctCells << "\n";
    cout << "Incorrect Cells: " << incorrectCells << "\n";
    cout << "Time Spent: " << elapsedTime / 60 << "m " << elapsedTime % 60 << "s\n";
    cout << "Score: " << score << "\n";
    cout << "Press M to return to the menu.\n";

    while (true) {
        char key = _getch();
        if (key == 'M' || key == 'm') {
            return;
        }
    }
}

void displayLeaderboard() {
    clearScreen();
    cout << "Leaderboard:\n";
    
    if (leaderboardCount == 0) {
        cout << "No entries yet.\n";
        cout << "Press M to return to the menu.\n";
        while (true) {
            char key = _getch();
            if (key == 'M' || key == 'm') return;
        }
    }

    bubbleSortLeaderboard();

    cout << "Rank | Username | Score | win | lose | Time \n";
    cout << "-----+----------+-------+-----+------+------------\n";
    for (int i = 0; i < leaderboardCount; ++i) {
        cout << (i + 1) << "    | "
             << leaderboard[i].username << " |  "
             << leaderboard[i].score << "   |  "
             << leaderboard[i].win << "  |  "
             << leaderboard[i].lose << "   | "
             << leaderboard[i].time / 60 << "m "
             << leaderboard[i].time % 60 << "s\n";

    }

    cout << "Press M to return to the menu.\n";
    while (true) {
        char key = _getch();
        if (key == 'M' || key == 'm') return;
    }
}

void playSudoku(int board[][9], int totalTime, const char* username) {
    int cursorX = 0, cursorY = 0;
    int mistakes = 0, correctCells = 0, incorrectCells = 0;
    int positiveScore = 0, negativeScore = 0;
    auto startTime = chrono::steady_clock::now();
    bool isPaused = false;

    while (true) {
        if (!isPaused) {
            auto currentTime = chrono::steady_clock::now();
            int elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

            if (elapsedTime >= totalTime) {
                int finalScore = positiveScore - negativeScore;
                
                
                bool found = false;
                for (int i = 0; i < leaderboardCount; ++i) {
                    if (strcmp(leaderboard[i].username, username) == 0) {
                        if (finalScore > leaderboard[i].score ||
                           (finalScore == leaderboard[i].score && elapsedTime < leaderboard[i].time)) {
                            leaderboard[i].score = finalScore;
                            leaderboard[i].time = elapsedTime;
                            leaderboard[i].lose += 1 ;
                        }
                        found = true;
                        break;
                    }
                }
                if (!found && leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
                    strcpy(leaderboard[leaderboardCount].username, username);
                    leaderboard[leaderboardCount].score = finalScore;
                    leaderboard[leaderboardCount].time = elapsedTime;
                    leaderboard[leaderboardCount].lose = 1;
                    leaderboardCount++;
                }

                displayEndScreen(false, correctCells, incorrectCells, elapsedTime, finalScore);
                return;
            }

            displaySudoku(board, cursorX, cursorY, elapsedTime, totalTime);

            cout << "\nUse arrow keys to move, numbers to fill, ESC to pause, Q to quit.\n";
        } else {
            displayPauseScreen();
        }

        char key = _getch();
        if (key == 27) {
            isPaused = !isPaused;
        } else if (isPaused && (key == 'S' || key == 's')) {
            char saveName[50];
            cout << "Enter a name to save the game: ";
            cin.getline(saveName, 50);

            bool exists = false;
            for (int i = 0; i < savedGamesCount; ++i) {

                if (strcmp(savedGames[i].name, saveName) == 0) {
                    exists = true;
                    break;
                }
            }

            if (exists) {
                cout << RED << "Error: A save with this name already exists!" << RESET << "\n";
                this_thread::sleep_for(chrono::seconds(2));
            } else if (savedGamesCount < MAX_SAVED_GAMES) {
                int elapsedTime = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
                SavedGame sg;
                strcpy(sg.name, saveName);
                memcpy(sg.board, board, sizeof(sg.board));
                sg.elapsedTime = elapsedTime;
                sg.remainingTime = totalTime - elapsedTime;
                savedGames[savedGamesCount++] = sg;
                cout << GREEN << "Game saved successfully! Returning to main menu..." << RESET << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                return;
            } else {
                cout << RED << "Error: Maximum saved games reached!" << RESET << "\n";
                this_thread::sleep_for(chrono::seconds(2));
            }
        } else if (isPaused && (key == 'Q' || key == 'q')) {
            int finalScore = positiveScore - negativeScore;
            int elapsedTime = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
            
            bool found = false;
            for (int i = 0; i < leaderboardCount; ++i) {
                if (strcmp(leaderboard[i].username, username) == 0) {
                    if (finalScore > leaderboard[i].score ||
                       (finalScore == leaderboard[i].score && elapsedTime < leaderboard[i].time)) {
                        leaderboard[i].score = finalScore;
                        leaderboard[i].time = elapsedTime;
                        leaderboard[i].lose += 1;
                    }
                    found = true;
                    break;
                }
            }
            if (!found && leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
                strcpy(leaderboard[leaderboardCount].username, username);
                leaderboard[leaderboardCount].score = finalScore;
                leaderboard[leaderboardCount].time = elapsedTime;
                leaderboard[leaderboardCount].lose = 1;
				leaderboardCount++;
            }

            displayEndScreen(false, correctCells, incorrectCells, elapsedTime, finalScore);
            return;
        } else if (!isPaused) {
            if (key == -32) {
                key = _getch();
                if (key == 72 && cursorY > 0) cursorY--;
                if (key == 80 && cursorY < 8) cursorY++;
                if (key == 75 && cursorX > 0) cursorX--;
                if (key == 77 && cursorX < 8) cursorX++;
            } else if (key >= '1' && key <= '9') {
                int value = key - '0';
                if (board[cursorY][cursorX] != 0) {
                    cout << RED << "This cell is already filled. Choose another." << RESET << "\n";
                    this_thread::sleep_for(chrono::seconds(1));
                } else if (!isValidMove(board, cursorY, cursorX, value)) {
                    mistakes++;
                    incorrectCells++;
                    negativeScore++;
                    cout << RED << "Incorrect move! " << mistakes << " mistake(s) out of 3 allowed." << RESET << "\n";
                    this_thread::sleep_for(chrono::seconds(1));
                    if (mistakes == 3) {
                        int finalScore = positiveScore - negativeScore;
                        int elapsedTime = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
                        
                        bool found = false;
                        for (int i = 0; i < leaderboardCount; ++i) {
                            if (strcmp(leaderboard[i].username, username) == 0) {
                                if (finalScore > leaderboard[i].score ||
                                   (finalScore == leaderboard[i].score && elapsedTime < leaderboard[i].time)) {
                                    leaderboard[i].score = finalScore;
                                    leaderboard[i].time = elapsedTime;
                                    leaderboard[i].lose += 1;
                                }
                                found = true;
                                break;
                            }
                        }
                        if (!found && leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
                            strcpy(leaderboard[leaderboardCount].username, username);
                            leaderboard[leaderboardCount].score = finalScore;
                            leaderboard[leaderboardCount].time = elapsedTime;
                            leaderboard[leaderboardCount].lose = 1;
                            leaderboardCount++;
                        }

                        displayEndScreen(false, correctCells, incorrectCells, elapsedTime, finalScore);
                        return;
                    }
                } else {
                    board[cursorY][cursorX] = value;
                    correctCells++;
                    positiveScore++;
                }
            } else if (key == 'Q' || key == 'q') {
                int finalScore = positiveScore - negativeScore;
                int elapsedTime = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
                
                bool found = false;
                for (int i = 0; i < leaderboardCount; ++i) {
                    if (strcmp(leaderboard[i].username, username) == 0) {
                        if (finalScore > leaderboard[i].score ||
                           (finalScore == leaderboard[i].score && elapsedTime < leaderboard[i].time)) {
                            leaderboard[i].score = finalScore;
                            leaderboard[i].time = elapsedTime;
                            leaderboard[i].lose += 1 ;
                        }
                        found = true;
                        break;
                    }
                }
                if (!found && leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
                    strcpy(leaderboard[leaderboardCount].username, username);
                    leaderboard[leaderboardCount].score = finalScore;
                    leaderboard[leaderboardCount].time = elapsedTime;
                    leaderboard[leaderboardCount].lose = 1;
                    leaderboardCount++;
                }

                displayEndScreen(false, correctCells, incorrectCells, elapsedTime, finalScore);
                return;
            }
        }


        bool isComplete = true;
        for (int i = 0; i < 9 && isComplete; ++i) {
            for (int j = 0; j < 9 && isComplete; ++j) {
                if (board[i][j] == 0) isComplete = false;
            }
        }

        if (isComplete) {
            int finalScore = positiveScore - negativeScore;
            int elapsedTime = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - startTime).count();
            
            bool found = false;
            for (int i = 0; i < leaderboardCount; ++i) {
                if (strcmp(leaderboard[i].username, username) == 0) {
                    if (finalScore > leaderboard[i].score ||
                       (finalScore == leaderboard[i].score && elapsedTime < leaderboard[i].time)) {
                        leaderboard[i].score = finalScore;
                        leaderboard[i].time = elapsedTime;
                        leaderboard[i].win += 1;
                    }
                    found = true;
                    break;
                }
            }
            if (!found && leaderboardCount < MAX_LEADERBOARD_ENTRIES) {
                strcpy(leaderboard[leaderboardCount].username, username);
                leaderboard[leaderboardCount].score = finalScore;
                leaderboard[leaderboardCount].time = elapsedTime;
                leaderboard[leaderboardCount].win = 1;
                leaderboardCount++;
            }

            displayEndScreen(true, correctCells, incorrectCells, elapsedTime, finalScore);
            return;
        }
    }
}

bool isUsernameUnique(const char* username) {
    for (int i = 0; i < leaderboardCount; ++i) {
        if (strcmp(leaderboard[i].username, username) == 0) {
            return false;
        }
    }
    return true;
}

void bubbleSortLeaderboard() {
    for (int i = 0; i < leaderboardCount - 1; ++i) {
        for (int j = 0; j < leaderboardCount - i - 1; ++j) {
            bool swapNeeded = false;
            if (leaderboard[j].score < leaderboard[j + 1].score) {
                swapNeeded = true;
            } else if (leaderboard[j].score == leaderboard[j + 1].score) {
                if (leaderboard[j].time > leaderboard[j + 1].time) {
                    swapNeeded = true;
                }
            }

            if (swapNeeded) {
                LeaderboardEntry temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}

int main() {
    char username[50];
    
    while (true) {
        do {
            clearScreen();
            cout << "Enter your username: ";
            cin.getline(username, 50);
            if (!isUsernameUnique(username)) {
                cout << RED << "Error: This username already exists! Please choose another." << RESET << "\n";
                this_thread::sleep_for(chrono::seconds(2));
                username[0] = '\0';
            }
        } while (username[0] == '\0');

        int choice;
        do {
            displayMenu(username);
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1: {
                    int board[9][9] = {
                        {5, 3, 0, 0, 7, 0, 0, 0, 0},
                        {6, 0, 0, 1, 9, 5, 0, 0, 0},
                        {0, 9, 8, 0, 0, 0, 0, 6, 0},
                        {8, 0, 0, 0, 6, 0, 0, 0, 3},
                        {4, 0, 0, 8, 0, 3, 0, 0, 1},
                        {7, 0, 0, 0, 2, 0, 0, 0, 6},
                        {0, 6, 0, 0, 0, 0, 2, 8, 0},
                        {0, 0, 0, 4, 1, 9, 0, 0, 5},
                        {0, 0, 0, 0, 8, 0, 0, 7, 9}
                    };

                    displayDifficultyMenu();
                    int difficulty;
                    cin >> difficulty;
                    cin.ignore();
                    int totalTime = (difficulty == 1 ? 300 : (difficulty == 2 ? 180 : 120));

                    playSudoku(board, totalTime, username);
                    break;
                }
                case 2: {
                    if (savedGamesCount == 0) {
                        cout << RED << "No saved games available." << RESET << "\n";
                        this_thread::sleep_for(chrono::seconds(2));
                    } else {
                        cout << "Saved Games:\n";
                        for (int i = 0; i < savedGamesCount; ++i) {
                            cout << "- " << savedGames[i].name 
                                 << " (Elapsed: " << savedGames[i].elapsedTime / 60 << "m " 
                                 << savedGames[i].elapsedTime % 60 << "s, Remaining: " 
                                 << savedGames[i].remainingTime / 60 << "m " 
                                 << savedGames[i].remainingTime % 60 << "s)\n";
                        }
                        cout << "Enter the name of the save file to load: ";
                        char saveName[50];
                        cin.getline(saveName, 50);

                        bool found = false;
                        for (int i = 0; i < savedGamesCount; ++i) {
                            if (strcmp(savedGames[i].name, saveName) == 0) {
                                int loadedBoard[9][9];
                                memcpy(loadedBoard, savedGames[i].board, sizeof(loadedBoard));
                                playSudoku(loadedBoard, savedGames[i].remainingTime, username);
                                found = true;
                                break;
                                
                            }
                        }

                        if (!found) {
                            cout << RED << "Save not found." << RESET << "\n";
                            this_thread::sleep_for(chrono::seconds(2));
                        }
                    }
                    break;
                }
                case 3:
                    displayLeaderboard();
                    break;
                case 4:
                    clearScreen();
                    cout << "Returning to username input...\n";
                    this_thread::sleep_for(chrono::seconds(1));
                    username[0] = '\0'; 
                    break;
                default:
                    cout << RED << "Invalid choice. Try again." << RESET << "\n";
                    break;
            }
        } while (choice != 4);
    }

    return 0;
}