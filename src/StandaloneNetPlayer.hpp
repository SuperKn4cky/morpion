#pragma once
#ifndef STANDALONE_NET_PLAYER_HPP_
#define STANDALONE_NET_PLAYER_HPP_

#include <iostream>
#include <optional>
#include <SFML/Network.hpp>
#include <future>
#include "IPlayer.hpp"

class StandaloneNetPlayer : public IPlayer {
    sf::SocketSelector _stor;
    sf::TcpSocket _sock;
    std::string _player_symbol;
    std::optional<unsigned int> _move_made;
    bool _done{false};
    void send_msg(std::string msg);
    std::string receive_msg();
public:
    StandaloneNetPlayer(int port);
    void set_win(char player) override;
    void set_draw() override;
    std::optional<unsigned int> get_move() override;
    void set_player_symbol(char player) override;
    void set_board_state(const std::array<char, 9>& board) override;
    bool done() override;
    void ask_for_move(char player) override;
};



#endif /* STANDALONE_NET_PLAYER_HPP_ */