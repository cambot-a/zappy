/*
** EPITECH PROJECT, 2026
** Zappy
** File description:
** RAII owner of a POSIX file descriptor
*/

#ifndef POSIX_FILEDESCRIPTOR_HPP_
    #define POSIX_FILEDESCRIPTOR_HPP_

namespace zappy::posix {

/**
 * @brief Move-only RAII owner of a POSIX file descriptor.
 */
class FileDescriptor {
public:
    /**
     * @brief Build an invalid descriptor.
     */
    FileDescriptor() noexcept;

    /**
     * @brief Take ownership of @p fd.
     *
     * @param fd raw file descriptor to wrap (INVALID_FD for an empty descriptor)
     */
    explicit FileDescriptor(int fd) noexcept;

    /**
     * @brief Close the owned descriptor.
     */
    ~FileDescriptor();

    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;

    /**
     * @brief Steal the descriptor from @p other.
     *
     * @param other source descriptor, left in an invalid state
     */
    FileDescriptor(FileDescriptor &&other) noexcept;

    /**
     * @brief Close the current descriptor, then steal @p other's.
     *
     * @param other source descriptor, left in an invalid state
     * @return FileDescriptor& reference to this
     */
    FileDescriptor &operator=(FileDescriptor &&other) noexcept;

    /**
     * @brief Return the raw descriptor without releasing ownership.
     *
     * @return int the raw file descriptor, or INVALID_FD if empty
     */
    [[nodiscard]] int get() const noexcept;

    /**
     * @brief Release ownership and return the raw descriptor.
     *
     * @return int the raw file descriptor; caller is now responsible for closing it
     */
    [[nodiscard]] int release() noexcept;

    /**
     * @brief Close the descriptor if open; idempotent.
     */
    void close() noexcept;

    /**
     * @brief Tell whether the descriptor is valid.
     *
     * @return true if the descriptor is open
     * @return false if the descriptor is INVALID_FD
     */
    [[nodiscard]] bool is_valid() const noexcept;

    /**
     * @brief Same as is_valid().
     *
     * @return true if the descriptor is open
     * @return false if the descriptor is INVALID_FD
     */
    explicit operator bool() const noexcept;

private:
    int _fd;
};

}

#endif /* !POSIX_FILEDESCRIPTOR_HPP_ */
