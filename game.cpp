#include <iostream>      // for input/output
#include <vector>        // for dynamic arrays (used to store piles)
#include <climits>       // for INT_MIN and INT_MAX constants
#include <algorithm>     // for sort()
#include <unordered_map> // for memoization (fast lookups)
#include <utility>       // for std::pair and std::make_pair
#include <string>
using namespace std;

/*

   STRUCT: Node
   
   Each Node represents one "state" of the game.

   Example: [3,4,5] means 3 piles with 3, 4, and 5 objects respectively.
   - player = 1 → human player’s turn
   - player = 2 → AI’s turn
   - score → minimax evaluation score for this state
   - children → future possible game states reachable from this one
*/
struct Node {
    vector<int> piles;
    int player;
    int score;
    vector<Node*> children;

    // Constructor: creates a new game state
    Node(const vector<int>& p, int pl)
        : piles(p), player(pl), score(0) {}
};

/*
   
   FUNCTION: print_board()
  
   Prints the current number of objects left in each pile.
*/
void print_board(const vector<int>& piles) {
    cout << "\nCurrent piles:\n" << flush;
    for (size_t i = 0; i < piles.size(); ++i) {
        cout << "Pile " << i + 1 << ": " << piles[i] << "\n" << flush;
    }
}

/*
   
   FUNCTION: is_terminal()
   -
   Returns true if all piles are empty (i.e., no moves left).
   Used to check if the game has ended.
*/
bool is_terminal(const vector<int>& piles) {
    for (size_t i = 0; i < piles.size(); ++i)
        if (piles[i] != 0)
            return false;
    return true;
}

/*
   FUNCTION: key_of()
   
   Creates a unique key string for each game state.
   This helps store previously evaluated states in a hash map (memoization).
*/
string key_of(const vector<int>& piles, int player) {
    vector<int> s = piles;
    sort(s.begin(), s.end()); // sorting avoids duplicates like [3,4,5] vs [5,3,4]
    string k;
    for (size_t i = 0; i < s.size(); ++i) k += to_string(s[i]) + ",";
    k += "|P" + to_string(player);
    return k;
}

/*

   FUNCTION: generate_children()
  
   Generates all possible next game states for the current player.
   For each pile, try removing 1, 2, ..., n objects and create a new Node.
*/
void generate_children(Node* node) {
    const vector<int>& p = node->piles;
    for (size_t i = 0; i < p.size(); ++i) {
        if (p[i] == 0) continue; // skip empty piles
        for (int take = 1; take <= p[i]; ++take) {
            vector<int> next = p;
            next[i] -= take; // simulate removing objects
            Node* child = new Node(next, node->player == 1 ? 2 : 1);
            node->children.push_back(child);
        }
    }
}

/*
   
   FUNCTION: terminal_score()
   
   Evaluates a finished game:
   - If it's AI's turn and board is empty → AI lost (score = -1)
   - If it's human's turn and board is empty → AI won (score = +1)
*/
int terminal_score(int player_to_move) {
    return (player_to_move == 2) ? -1 : +1;
}

/*
 
   FUNCTION: minimax()
  
   Classic Minimax recursive algorithm with memoization.
   - AI (player 2) tries to MAXIMIZE the score.
   - Human (player 1) tries to MINIMIZE the score.
   - Memoization avoids recalculating known states.
*/
int minimax(Node* node, unordered_map<string, int>& memo) {
    // Base case: if game is over, return +1 or -1 depending on who lost.
    if (is_terminal(node->piles)) {
        node->score = terminal_score(node->player);
        return node->score;
    }

    // Use stored result if this state was already calculated
    string K = key_of(node->piles, node->player);
    if (memo.find(K) != memo.end())
        return memo[K];

    // Generate all possible moves from this state
    generate_children(node);

    // If it's AI's turn (maximize)
    if (node->player == 2) {
        int best = INT_MIN;
        for (size_t j = 0; j < node->children.size(); ++j)
            best = max(best, minimax(node->children[j], memo));
        node->score = best;
    } else {
        // Human's turn (minimize)
        int best = INT_MAX;
        for (size_t j = 0; j < node->children.size(); ++j)
            best = min(best, minimax(node->children[j], memo));
        node->score = best;
    }

    // Store result for future use
    memo[K] = node->score;
    return node->score;
}

/*
   
   FUNCTION: best_ai_move()
   
   Finds the best move for AI using the Minimax algorithm.
   It simulates every possible move and picks the one with the highest score.
*/
pair<int, int> best_ai_move(const vector<int>& piles) {
    Node root(piles, 2); // Start from AI's perspective
    unordered_map<string, int> memo;
    minimax(&root, memo);

    int bestScore = INT_MIN;
    pair<int, int> bestMove(-1, -1); // (pileIndex, objectsRemoved)

    // Explore all possible children to find the optimal move
    root.children.clear();
    generate_children(&root);

    for (size_t j = 0; j < root.children.size(); ++j) {
        Node* c = root.children[j];
        int val = minimax(c, memo);
        if (val > bestScore) {
            bestScore = val;
            // Find which pile changed to identify the move
            for (size_t i = 0; i < piles.size(); ++i) {
                if (c->piles[i] != piles[i]) {
                    bestMove = std::make_pair(static_cast<int>(i),
                                              piles[i] - c->piles[i]);
                    break;
                }
            }
        }
    }
    return bestMove;
}

/*
  
   FUNCTION: main()
   
   Entry point of the game. Handles user input, game flow,
   and alternates turns between the player and AI.
*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Welcome to the Nim Game!\n" << flush << endl;
    cout << "Enter number of piles: " << flush;
    int n;
    cin >> n;

    vector<int> piles(n);
    cout << "Enter pile sizes:\n" << flush;
    for (int i = 0; i < n; ++i) cin >> piles[i];

    cout << "Who goes first? (1 = You, 2 = AI): " << flush;
    int turn;
    cin >> turn;

    // Main game loop
    while (true) {
        print_board(piles);

        // Check for winning condition
        if (is_terminal(piles)) {
            cout << "\nGame over. Winner is "
                 << (turn == 1 ? "AI" : "You") << "!\n" << flush;
            break;
        }

        // Human player turn
        if (turn == 1) {
            cout << "\nYour move (pile number & how many to remove): " << flush;
            int pidx, amt;
            cin >> pidx >> amt;
            pidx--; // adjust index for 0-based array
            if (pidx < 0 || pidx >= n || amt <= 0 || amt > piles[pidx]) {
                cout << "Invalid move. Try again.\n" << flush;
                continue;
            }
            piles[pidx] -= amt;
            turn = 2; // switch to AI
        }
        // AI turn
        else {
            cout << "\nAI is thinking...\n" << flush;
            pair<int, int> mv = best_ai_move(piles);
            cout << "AI removes " << mv.second
                 << " from pile " << mv.first + 1 << "\n" << flush;
            piles[mv.first] -= mv.second;
            turn = 1; // switch to human3
        }
    }

    return 0;
}
