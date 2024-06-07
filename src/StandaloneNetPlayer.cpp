#include "StandaloneNetPlayer.hpp"

StandaloneNetPlayer::StandaloneNetPlayer(int port)
{
    sf::TcpListener lstnr;

    lstnr.listen(port);
    lstnr.accept(_sock);
    _stor.add(lstnr);
    _stor.add(_sock);
}

void StandaloneNetPlayer::send_msg(std::string msg)
{
    _sock.send(msg.c_str(), msg.size());
}

std::string StandaloneNetPlayer::receive_msg()
{
    char *c_str;
    std::size_t bytes_read;

    c_str = new char[100];
    if (_sock.receive(c_str, 99, bytes_read) != sf::Socket::Done)
        throw std::runtime_error("Failed to receive data from server.");
    c_str[bytes_read] = '\0';
    std::string msg(c_str);
    delete[] c_str;
    return msg;
}

void StandaloneNetPlayer::set_win(char player)
{
    send_msg("WIN " + std::string(1, player) + "\n");
}

void StandaloneNetPlayer::set_draw()
{
    send_msg("DRAW\n");
}

std::optional<unsigned int> StandaloneNetPlayer::get_move()
{
    if (_move_made)
        return _move_made;

    std::string msg = receive_msg();
    if (msg.starts_with("MOVE") == true) {
        int move = std::stoi(msg.substr(4));
        _move_made = move;
        return move;
    } else if (msg == "QUIT") {
        _done = true;
    }
    return std::nullopt;
}

void StandaloneNetPlayer::set_player_symbol(char player)
{
    _player_symbol = player;
    send_msg("SYMBOL " + std::string(1, player) + "\n");
}

void StandaloneNetPlayer::set_board_state(const std::array<char, 9>& board)
{
    std::string board_state = "BOARD\n";
    int counter = 0;
    for (char cell : board) {
        board_state += cell;
        counter += 1;
        if (counter % 3 == 0)
            board_state += "\n";
    }
    board_state += "\n";
    send_msg(board_state);
}

bool StandaloneNetPlayer::ask_end_game()
{
    return _done;
}

bool StandaloneNetPlayer::done()
{
    return _done;
}

void StandaloneNetPlayer::ask_for_move(char player)
{
    send_msg("ASK_MOVE " + std::string(1, player) + "\n");
    _move_made.reset();
}