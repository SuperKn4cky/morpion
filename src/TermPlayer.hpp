#pragma once
#include <optional>
#include <future>
#include "IPlayer.hpp"

class TermPlayer : public IPlayer {
public:
    TermPlayer();
    ~TermPlayer();
    void                        set_win(char player) override;
    void                        set_draw(void) override;
    std::optional<unsigned int> get_move() override;
    void                        set_player_symbol(char player) override;
    void set_board_state(const std::array<char, 9> &board) override;
    bool done() override;
    void ask_for_move(char player) override;
    unsigned int process_events() override;

private:
    std::future<unsigned int> _futureAnswer;
    std::optional<unsigned int> _move_made;
};
