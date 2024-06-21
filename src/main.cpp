#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <optional>
#include <SFML/Network.hpp>
#include <cstring>
#include "GfxPlayer.hpp"
#include "IPlayer.hpp"
#include "MorpionGame.hpp"
#include "TermPlayer.hpp"
#include "StandaloneNetPlayer.hpp"

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

static void move(GfxPlayer &gfx_player, sf::TcpSocket &sock, bool &move_left) {
    auto move = gfx_player.get_move();
    std::size_t bytes_sent;
    if (move) {
        std::string move_msg = "MOVE " + std::to_string(*move);
        sock.send(move_msg.c_str(), move_msg.size(), bytes_sent);
        move_left = false;
    }
}

static void actions(std::string &msg, GfxPlayer &gfx_player, sf::TcpSocket &sock, bool &move_left) {
    if (msg.starts_with("ASK_MOVE") == true) {
            char sym = msg.back();
            gfx_player.ask_for_move(sym);
            move_left = true;
            move(gfx_player, sock, move_left);
        } else if (msg.starts_with("SYMBOL") == true) {
            char sym = msg[7];
            gfx_player.set_player_symbol(sym);
        } else if (msg.starts_with("BOARD") == true) {
            std::array<char, 9> board;
            for (int i = 0; i < 9; i += 1)
                board[i] = msg[i + 5];
            gfx_player.set_board_state(board);
        } else if (msg.starts_with("WIN") == true) {
            gfx_player.set_win(msg.back());
            sock.disconnect();
        } else if (msg.starts_with("DRAW") == true) {
            gfx_player.set_draw();
            sock.disconnect();
        } else if (msg.starts_with("QUIT") == true) {
            std::cerr << "Server has quit the game." << std::endl;
            sock.disconnect();
        }
}

void client(sf::TcpSocket &sock)
{
    GfxPlayer gfx_player;
    std::string msg;
    char data[100];
    std::size_t received{0};
    bool move_left = false;

    sock.setBlocking(false);
    while (sock.getRemoteAddress() != sf::IpAddress::None && !gfx_player.done()) {
        if (received == 0 && sock.receive(data, sizeof(data), received) == sf::Socket::Error) {
            std::cerr << "Failed to receive data from server." << std::endl;
            sock.disconnect();
            break;
        }
        msg += data;
        std::memset(data, 0 , sizeof(data));
        if (received > 0) {
            actions(msg, gfx_player, sock, move_left);
        }
        if (move_left == true) {
            move(gfx_player, sock, move_left);
        }
        gfx_player.process_events();
        msg = &msg[msg.find('\n') + 1];
        received = msg.length();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (gfx_player.done()) {
        std::cerr << "Game interrupted." << std::endl;
        std::size_t bytes_sent;
        sock.send("QUIT\n", 5, bytes_sent);
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

using player_ptr = std::unique_ptr<IPlayer>;

void server(StandaloneNetPlayer &NetPlayer)
{
    MorpionGame               game;
    std::array<player_ptr, 2> players{player_ptr(&NetPlayer),
                                      player_ptr(new GfxPlayer)};
    std::array<char, 2>       symbols{'x', 'o'};
    unsigned int              current_player{0};
    std::optional<unsigned int> move;
    bool played = false;

    players[0]->set_player_symbol(symbols[0]);
    players[0]->set_board_state(game.array());
    players[1]->set_player_symbol(symbols[1]);
    players[1]->set_board_state(game.array());
    players[current_player]->ask_for_move(symbols[current_player]);
    while (!game.done() && !players[0]->done() && !players[1]->done()) {
        try {
            move = players[current_player]->get_move();
        }
        catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return;
        }
        if (move) {
            played = game.play(symbols[current_player], *move);
            players[0]->set_board_state(game.array());
            players[1]->set_board_state(game.array());
        }
        if (played) {
            current_player = !current_player;
            played = false;
            players[current_player]->ask_for_move(symbols[current_player]);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!players[0]->done() && !players[1]->done())
        report_win(game, *players[0], *players[1]);
    else
        std::cerr << "Game interrupted." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}


int main(int ac, char **av)
{
    if (ac < 2) {
        std::cerr << "Usage: " << av[0] << " <-s> or <-c> <ip> <port>" << std::endl;
        return 1;
    }
    if (std::string(av[1]) == "-s") {
        std::cout << "Starting server on port " << av[3] << std::endl;
        StandaloneNetPlayer player(std::stoi(av[2]));
        server(player);
    } else if (std::string(av[1]) == "-c") {
        sf::TcpSocket sock;
        std::cout << "Connecting to " << av[2] << ":" << av[3] << std::endl;
        sock.connect(sf::IpAddress(av[2]), std::stoi(av[3]));
        client(sock);
    }
    return 0;
}
