#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

class Connection
    : public std::enable_shared_from_this<Connection>
{
  private:
    boost::asio::ip::tcp::socket m_socket;
    boost::asio::streambuf m_buffer{};
    std::regex m_pattern_insert{};
    std::regex m_pattern_truncate{};
    std::regex m_pattern_intersection{};
    std::regex m_pattern_symmetric_difference{};

    std::map<int, std::string> A{};
    std::map<int, std::string> B{};

    size_t
    get_max_length(const std::map<int, std::string> &collection)
    {
        size_t max_length = 0;
        for (const auto &[key, value] : collection)
        {
            max_length = std::max(max_length, value.length());
        }
        return max_length;
    }

    void
    print_intersection()
    {
        std::stringstream buffer{};
        size_t max_length_A = get_max_length(A);
        size_t max_length_B = get_max_length(B);

        buffer << "id | A" << std::string(max_length_A, ' ') << "| B" << std::string(max_length_A, ' ') << "\n";
        buffer << "---+-" << std::string(max_length_A + 1, '-') << "+-" << std::string(max_length_B + 1, '-') << "\n";
        for (const auto &[key, valueA] : A)
        {
            auto itB = B.find(key);
            if (itB != B.end())
            {
                buffer << std::left << std::setw(2) << key << " | "
                       << std::setw(max_length_A) << valueA << " | "
                       << std::setw(max_length_B) << itB->second << "\n";
            }
        }
        boost::asio::write(m_socket, boost::asio::buffer(buffer.str()));
    }

    void
    print_symmetric_difference()
    {
        std::stringstream buffer{};
        size_t max_length_A = get_max_length(A);
        size_t max_length_B = get_max_length(B);

        buffer << "id | A" << std::string(max_length_A, ' ') << "| B" << std::string(max_length_A, ' ') << "\n";
        buffer << "---+-" << std::string(max_length_A + 1, '-') << "+-" << std::string(max_length_B + 1, '-') << "\n";

        // Сначала выводим элементы из A, которых нет в B
        for (const auto &[key, valueA] : A)
        {
            if (B.find(key) == B.end())
            {
                buffer << std::left << std::setw(2) << key << " | "
                       << std::setw(max_length_A) << valueA << " | "
                       << std::setw(max_length_B) << " "
                       << "\n";
            }
        }

        // Затем выводим элементы из B, которых нет в A
        for (const auto &[key, valueB] : B)
        {
            if (A.find(key) == A.end())
            {
                buffer << std::left << std::setw(2) << key << " | "
                       << std::setw(max_length_A) << " "
                       << " | "
                       << std::setw(max_length_B) << valueB << "\n";
            }
        }
        boost::asio::write(m_socket, boost::asio::buffer(buffer.str()));
    }

    void
    print_ok()
    {
        boost::asio::write(m_socket, boost::asio::buffer("OK\n"));
    }

    void
    print_error(const std::exception &err)
    {
        std::string e = err.what();
        boost::asio::write(m_socket, boost::asio::buffer(e + "\n"));
    }

    void
    do_read()
    {
        boost::asio::async_read_until(
            m_socket,
            m_buffer,
            '\n',
            [self = shared_from_this()](
                const boost::system::error_code &ec,
                std::size_t n)
            {
                if (!ec)
                {
                    std::istream is(&self->m_buffer);
                    std::string line;
                    std::getline(is, line);
                    if (!line.empty())
                    {
                        try
                        {
                            std::smatch match_insert;
                            if (std::regex_search(line, self->m_pattern_intersection)) // intersection
                            {
                                self->print_intersection();
                            }
                            else if (std::regex_search(line, self->m_pattern_symmetric_difference)) // symmetric_difference
                            {
                                self->print_symmetric_difference();
                            }
                            else if (std::regex_search(line, match_insert, self->m_pattern_insert)) // insert
                            {
                                int number = std::stoi(match_insert.str(2));
                                if (match_insert.str(1) == "A")
                                {
                                    self->A.emplace(number, match_insert.str(3));
                                    self->print_ok();
                                }
                                else if (match_insert.str(1) == "B")
                                {
                                    self->B.emplace(number, match_insert.str(3));
                                    self->print_ok();
                                }
                                else
                                {
                                    self->print_error(std::runtime_error("command :" + line + " is not support"));
                                }
                            }
                            else if (std::regex_search(line, match_insert, self->m_pattern_truncate)) // truncate
                            {
                                if (match_insert.str(1) == "A")
                                {
                                    self->A.clear();
                                    self->print_ok();
                                }
                                else if (match_insert.str(1) == "B")
                                {
                                    self->B.clear();
                                    self->print_ok();
                                }
                                else
                                {
                                    self->print_error(std::runtime_error("command :" + line + " is not support"));
                                }
                            }
                            else
                            {
                                self->print_error(std::runtime_error("command :" + line + " is not support"));
                            }
                        }
                        catch (const std::exception &err)
                        {
                            self->print_error(err);
                        }
                    }
                    self->do_read();
                }
                else
                {
                    std::cout << "read error: " << ec.message() << std::endl;
                }
            });
    }

  public:
    Connection(boost::asio::ip::tcp::socket &&socket)
        : m_socket(std::move(socket))
    {
        m_pattern_insert = std::regex(R"(INSERT (A|B) (\d+) (.*))", std::regex_constants::icase);
        m_pattern_truncate = std::regex(R"(TRUNCATE (A|B))", std::regex_constants::icase);
        m_pattern_intersection = std::regex(R"(INTERSECTION)", std::regex_constants::icase);
        m_pattern_symmetric_difference = std::regex(R"(SYMMETRIC_DIFFERENCE)", std::regex_constants::icase);
    };
    void
    start()
    {
        do_read();
    };
};

class Server : public std::enable_shared_from_this<Server>
{
  private:
    void
    do_accept()
    {
        m_acceptor.async_accept(
            [self = shared_from_this()](
                boost::system::error_code ec,
                boost::asio::ip::tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Connection>(std::move(socket))->start();
                }
                else
                {
                    std::cout << "server error:" << ec.message() << std::endl;
                }
                self->do_accept();
            });
    }

  public:
    Server(boost::asio::io_context &io_context,
           boost::asio::ip::tcp::endpoint endpoint)
        : m_acceptor(io_context, endpoint){};
    void
    start()
    {
        do_accept();
    };

  private:
    boost::asio::ip::tcp::acceptor m_acceptor;
};

int
main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    std::string host{"0.0.0.0"};
    std::string port{argv[1]};
    boost::asio::io_context io_context{};
    try
    {
        boost::asio::ip::tcp::endpoint endpoint{};
        endpoint.address(boost::asio::ip::make_address(host));
        endpoint.port(std::stoi(port));
        std::make_shared<Server>(io_context, endpoint)->start();
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "error run: " << e.what() << std::endl;
    };
    return 0;
}
