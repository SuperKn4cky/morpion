#include "TermPlayer.hpp"
#include <iostream>

TermPlayer::TermPlayer()
{}

TermPlayer::~TermPlayer()
{}

void TermPlayer::set_win(char player)
{
    std::cout << "Player " << player << " wins!" << std::endl;
}

void TermPlayer::set_draw(void)
{
    std::cout << "No one wins" << std::endl;
}

std::optional<unsigned int> TermPlayer::get_move(char player)
{
    (void) player;
    if (_futureAnswer.valid()) {
        if (_futureAnswer.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                _move_made = _futureAnswer.get();
                return _move_made;
            } catch (const std::future_error &e) {
                done();
            }
        }
    }
    return std::nullopt;
}

void TermPlayer::set_player_symbol(char player)
{
    std::cout << "You are player: " << player << std::endl;
}

void TermPlayer::set_board_state(const std::array<char, 9> &board)
{
    unsigned int rounds{0};
    unsigned int idx{0};

    while (rounds < 3) {
        std::cout << board[idx];
        if (idx == 2 || idx == 5 || idx == 8) {
            rounds += 1;
            std::cout << std::endl;
        }
        idx += 1;
    }
}

unsigned int TermPlayer::process_events(bool turn)
{
    (void) turn;
    return 0;
}

bool TermPlayer::done()
{
    return std::cin.eof();
}

void TermPlayer::ask_for_move(char player)
{
    _move_made.reset();
    if (!_futureAnswer.valid() ) {
        _futureAnswer = std::async(std::launch::async, [player]() {
            std::cout << "Your turn: " << player << std::endl;
            std::cout << "Index between 0~8: ";
            unsigned int answer;
            std::string input;
            if (getline(std::cin, input)) {
                answer = std::stoi(input);
                return answer;
            } else {
                // INFO: the 9 OOB move indicates to arena that move was invalid
                answer = 9;
                std::cin.clear(std::cin.rdstate() & ~std::ios::failbit);
            }
            return answer;
        });
    }
}