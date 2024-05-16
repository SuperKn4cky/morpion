#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include "GfxPlayer.hpp"
#include "IPlayer.hpp"
#include "MorpionGame.hpp"
#include "TermPlayer.hpp"

using func_on_2_players = std::function<void(IPlayer &, IPlayer &)>;

const std::unordered_map<MorpionGame::Status, func_on_2_players> STATUS_MAP = {
    {MorpionGame::Status::Draw,
     [](IPlayer &x, IPlayer &o) {
         x.set_draw();
         o.set_draw();
     }},
    {MorpionGame::Status::PXWin,
     [](IPlayer &x, IPlayer &o) {
         x.set_win('x');
         o.set_win('x');
     }},
    {MorpionGame::Status::POWin,
     [](IPlayer &x, IPlayer &o) {
         x.set_win('o');
         o.set_win('o');
     }},
};

void report_win(MorpionGame &game, IPlayer &x, IPlayer &o)
{
    auto status{game.status()};
    auto found_iter{STATUS_MAP.find(status)};

    if (found_iter != STATUS_MAP.end()) {
        found_iter->second(x, o);
    } else {
        std::cerr << "the game hasn't ennded properly\n";
    }
}

void make_them_play(MorpionGame &game, IPlayer &player, char sym)
{
    std::cout << "Player " << sym << " turn" << std::endl;
    player.ask_for_move(sym);
    std::cout << "Player " << sym << " asked for move" << std::endl;

    bool played = false;
    while (!played && !player.done()) {
        //std::cout << "Player " << sym << " checking move" << std::endl;
        auto move = player.check_move();
        if (move) {
            played = game.play(sym, *move);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!played) {
        std::cerr << "Player " << sym
                  << " failed to make a move or game was interrupted."
                  << std::endl;
    }
}

using player_ptr = std::unique_ptr<IPlayer>;

int main(void)
{
    MorpionGame               game;
    std::array<player_ptr, 2> players{player_ptr(new TermPlayer),
                                      player_ptr(new GfxPlayer)};
    unsigned int              current_player{0};

    players[0]->set_player_symbol('x');
    players[0]->set_board_state(game.array());
    players[1]->set_player_symbol('o');
    players[1]->set_board_state(game.array());
    while (!game.done() && !players[0]->done() && !players[1]->done()) {
        make_them_play(game, *players[current_player],
                       (current_player) ? 'o' : 'x');
        current_player = !current_player;
    }
    if (!players[0]->done() && !players[1]->done())
        report_win(game, *players[0], *players[1]);
    else
        std::cerr << "Game interrupted." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
