#pragma once
#include <optional>
#include "IPlayer.hpp"

class TermPlayer : public IPlayer {
public:
    TermPlayer();
    ~TermPlayer();
    void                        set_win(char player) override;
    void                        set_draw(void) override;
    std::optional<unsigned int> get_move(char player) override;
    void                        set_player_symbol(char player) override;
    void set_board_state(const std::array<char, 9> &board) override;
    bool done() override;
    void ask_for_move(char player) override;
    std::optional<unsigned int> check_move() override;

private:
    std::optional<unsigned int> _move_made;
};
