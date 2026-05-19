#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <random>

using namespace std;

// ---------------- WORD LIST ----------------
namespace Wordlist
{
    vector<string> words{
        "broccoli", "drift", "phone", "galaxy", "quantum",
        "keyboard", "syntax", "variable", "function",
        "pointer", "compile", "debug"
    };
}

// ---------------- RANDOM ----------------
int getRandomIndex()
{
    static mt19937 mt{ random_device{}() };
    uniform_int_distribution<int> dist(0, Wordlist::words.size() - 1);
    return dist(mt);
}

// ---------------- SESSION CLASS ----------------
class Session
{
private:
    string m_word{ Wordlist::words[getRandomIndex()] };
    vector<char> m_letters{};
    vector<char> m_wrong{};

public:
    void push_letter(char letter)
    {
        m_letters.push_back(letter);
    }

    string get_word() const
    {
        return m_word;
    }

    bool has_already_guessed(char letter) const
    {
        for (char c : m_letters)
        {
            if (c == letter)
            {
                cout << "⚠ Already guessed!\n";
                return true;
            }
        }

        for (char c : m_wrong)
        {
            if (c == letter)
            {
                cout << "⚠ Already guessed!\n";
                return true;
            }
        }

        return false;
    }

    bool is_letter_guessed(char letter) const
    {
        for (char c : m_letters)
        {
            if (c == letter)
                return true;
        }
        return false;
    }

    bool is_letter_in_word(char letter) const
    {
        for (char c : m_word)
        {
            if (c == letter)
                return true;
        }
        return false;
    }

    void push_wrong(char letter)
    {
        m_wrong.push_back(letter);
    }

    vector<char> get_wrong() const
    {
        return m_wrong;
    }
};

// ---------------- INPUT ----------------
char getInput()
{
    while (true)
    {
        string input;
        cout << "Enter a letter: ";
        cin >> input;

        if (input.length() != 1)
        {
            cout << "❌ Enter only ONE letter.\n";
            continue;
        }

        char letter = tolower(static_cast<unsigned char>(input[0]));

        if (isalpha(static_cast<unsigned char>(letter)))
        {
            return letter;
        }
        else
        {
            cout << "❌ Invalid input.\n";
        }
    }
}

// ---------------- DISPLAY ----------------
void printBanner()
{
    cout << "=====================================\n";
    cout << "          🎯 WORD HUNT 🎯\n";
    cout << "=====================================\n";
    cout << "Guess the hidden word letter by letter.\n";
    cout << "Run out of attempts and you lose.\n\n";
}

void displayWord(const Session& session)
{
    cout << "Word: ";
    for (char c : session.get_word())
    {
        if (session.is_letter_guessed(c))
            cout << c << " ";
        else
            cout << "_ ";
    }
    cout << "\n";
}

void displayStatus(const Session& session, int tries)
{
    displayWord(session);

    cout << "Attempts Left: ";
    for (int i = 0; i < tries; i++)
        cout << "+ ";

    cout << "\nWrong Letters: ";
    for (char c : session.get_wrong())
        cout << c << " ";

    cout << "\n-------------------------------------\n";
}

// ---------------- WIN CHECK ----------------
bool checkWin(const Session& session)
{
    for (char c : session.get_word())
    {
        if (!session.is_letter_guessed(c))
            return false;
    }
    return true;
}

// ---------------- GAME ----------------
void startGame()
{
    Session session{};
    int tries = 6;

    cout << "Word to guess: ";
    for (size_t i = 0; i < session.get_word().size(); i++)
        cout << "_ ";
    cout << "\n\n";

    while (tries > 0)
    {
        displayStatus(session, tries);

        char guess = getInput();

        if (session.has_already_guessed(guess))
            continue;

        session.push_letter(guess);

        if (session.is_letter_in_word(guess))
        {
            cout << "✔ Correct!\n";
        }
        else
        {
            cout << "✖ Wrong!\n";
            session.push_wrong(guess);
            tries--;   
        }

        if (checkWin(session))
        {
            cout << "\n=====================================\n";
            cout << "🎉 YOU WON! 🎉\n";
            cout << "Word: " << session.get_word() << "\n";
            cout << "=====================================\n";
            return;
        }

        cout << "\n";
    }

    cout << "\n=====================================\n";
    cout << "💀 GAME OVER 💀\n";
    cout << "Word was: " << session.get_word() << "\n";
    cout << "=====================================\n";
}

// ---------------- MAIN ----------------
int main()
{
    printBanner();
    startGame();
}
