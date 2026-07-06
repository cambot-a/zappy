/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** Client implementation
*/

#include "server/client/Client.hpp"
#include <variant>

/**
 * @brief Take ownership of @p fd; initialise state to HANDSHAKE and payload to monostate.
 *
 * @param fd non-blocking file descriptor (already O_NONBLOCK)
 */
zappy::server::client::Client::Client(posix::FileDescriptor fd)
    : _buffer(std::move(fd)),
      _state(ClientState::HANDSHAKE),
      _payload(std::monostate{})
{}

/**
 * @brief Return the raw descriptor forwarded from the internal buffer.
 *
 * @return int file descriptor
 */
int zappy::server::client::Client::fd() const noexcept
{
    return _buffer.fd();
}

/**
 * @brief Return the current lifecycle state.
 *
 * @return ClientState
 */
zappy::server::client::ClientState
zappy::server::client::Client::state() const noexcept
{
    return _state;
}

/**
 * @brief Return a mutable reference to the I/O buffer.
 *
 * @return network::ClientBuffer&
 */
zappy::net::ClientBuffer &
zappy::server::client::Client::buffer() noexcept
{
    return _buffer;
}

/**
 * @brief Return a read-only reference to the I/O buffer.
 *
 * @return const network::ClientBuffer&
 */
const zappy::net::ClientBuffer &
zappy::server::client::Client::buffer() const noexcept
{
    return _buffer;
}

/**
 * @brief Transition from HANDSHAKE to @p newState and initialise the matching payload.
 *
 * @param newState target state; must be AI, GUI, or GUI_ADMIN
 * @throws ClientStateError if current state is not HANDSHAKE or newState is HANDSHAKE
 */
void zappy::server::client::Client::promote(ClientState newState)
{
    if (_state != ClientState::HANDSHAKE)
        throw ClientStateError("promote() called outside HANDSHAKE state");
    if (newState == ClientState::AI)
        _payload = AiData{};
    else if (newState == ClientState::GUI)
        _payload = GuiData{};
    else if (newState == ClientState::GUI_ADMIN)
        _payload = GuiAdminData{};
    else
        throw ClientStateError("promote() target must be AI, GUI or GUI_ADMIN");
    _state = newState;
}

/**
 * @brief Promote a GUI client to GUI_ADMIN; idempotent when already admin.
 *
 * @throws ClientStateError if current state is not GUI or GUI_ADMIN
 */
void zappy::server::client::Client::promoteToAdmin()
{
    if (_state != ClientState::GUI && _state != ClientState::GUI_ADMIN)
        throw ClientStateError("promoteToAdmin() requires GUI or GUI_ADMIN state");
    _payload = GuiAdminData{};
    _state = ClientState::GUI_ADMIN;
}

/**
 * @brief Return a mutable reference to the AiData payload.
 *
 * @return AiData&
 * @throws ClientStateError if state() != AI
 */
zappy::server::client::AiData &
zappy::server::client::Client::aiData()
{
    if (_state != ClientState::AI)
        throw ClientStateError("aiData() requires AI state");
    return std::get<AiData>(_payload);
}

/**
 * @brief Return a read-only reference to the AiData payload.
 *
 * @return const AiData&
 * @throws ClientStateError if state() != AI
 */
const zappy::server::client::AiData &
zappy::server::client::Client::aiData() const
{
    if (_state != ClientState::AI)
        throw ClientStateError("aiData() requires AI state");
    return std::get<AiData>(_payload);
}

/**
 * @brief Return a mutable reference to the GuiData payload.
 *
 * @return GuiData&
 * @throws ClientStateError if state() != GUI
 */
zappy::server::client::GuiData &
zappy::server::client::Client::guiData()
{
    if (_state != ClientState::GUI)
        throw ClientStateError("guiData() requires GUI state");
    return std::get<GuiData>(_payload);
}

/**
 * @brief Return a read-only reference to the GuiData payload.
 *
 * @return const GuiData&
 * @throws ClientStateError if state() != GUI
 */
const zappy::server::client::GuiData &
zappy::server::client::Client::guiData() const
{
    if (_state != ClientState::GUI)
        throw ClientStateError("guiData() requires GUI state");
    return std::get<GuiData>(_payload);
}

/**
 * @brief Return a mutable reference to the GuiAdminData payload.
 *
 * @return GuiAdminData&
 * @throws ClientStateError if state() != GUI_ADMIN
 */
zappy::server::client::GuiAdminData &
zappy::server::client::Client::guiAdminData()
{
    if (_state != ClientState::GUI_ADMIN)
        throw ClientStateError("guiAdminData() requires GUI_ADMIN state");
    return std::get<GuiAdminData>(_payload);
}

/**
 * @brief Return a read-only reference to the GuiAdminData payload.
 *
 * @return const GuiAdminData&
 * @throws ClientStateError if state() != GUI_ADMIN
 */
const zappy::server::client::GuiAdminData &
zappy::server::client::Client::guiAdminData() const
{
    if (_state != ClientState::GUI_ADMIN)
        throw ClientStateError("guiAdminData() requires GUI_ADMIN state");
    return std::get<GuiAdminData>(_payload);
}

/**
 * @brief Flag this client for a deferred drop once its buffer drains.
 */
void zappy::server::client::Client::markForDrop() noexcept
{
    _markedForDrop = true;
}

/**
 * @brief Whether a deferred drop was requested.
 *
 * @return bool true if markForDrop() was called
 */
bool zappy::server::client::Client::isMarkedForDrop() const noexcept
{
    return _markedForDrop;
}
