#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
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

    void
    print_intersection()
    {
        std::stringstream buffer{};
        buffer << "id | A         | B\n";
        buffer << "---+-----------+---------\n";
        for (const auto &[key, valueA] : A)
        {
            auto itB = B.find(key);
            if (itB != B.end())
            {
                buffer << key << " | " << valueA << " | " << itB->second << "\n";
            }
        }
        boost::asio::write(m_socket, boost::asio::buffer(buffer.str()));
    }

    void
    print_symmetric_difference()
    {
        std::stringstream buffer{};
        buffer << "id | A         | B\n";
        buffer << "---+-----------+---------\n";

        // Сначала выводим элементы из A, которых нет в B
        for (const auto &[key, valueA] : A)
        {
            if (B.find(key) == B.end())
            {
                buffer << key << " | " << valueA << " | \n";
            }
        }

        // Затем выводим элементы из B, которых нет в A
        for (const auto &[key, valueB] : B)
        {
            if (A.find(key) == A.end())
            {
                buffer << key << " |  | " << valueB << "\n";
            }
        }
        boost::asio::write(m_socket, boost::asio::buffer(buffer.str()));
    }

    void
    print_ok()
    {
        boost::asio::write(m_socket, boost::asio::buffer("OK"));
    }

    void
    print_error(const std::exception &err)
    {
        std::string e = err.what();
        boost::asio::write(m_socket, boost::asio::buffer(e));
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
                                int number = std::stoi(match_insert[1]);
                                if (match_insert[0] == "A")
                                {
                                    self->A.emplace(number, match_insert[2]);
                                }
                                else if (match_insert[0] == "B")
                                {
                                    self->B.emplace(number, match_insert[2]);
                                }
                                self->print_ok();
                            }
                            else if (std::regex_search(line, match_insert, self->m_pattern_truncate)) // truncate
                            {
                                if (match_insert[0] == "A")
                                {
                                    self->A.clear();
                                }
                                else if (match_insert[0] == "B")
                                {
                                    self->B.clear();
                                }
                                self->print_ok();
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
        m_pattern_insert = std::regex(R"(INSERT ((A|B)) (\d+) (.*))", std::regex_constants::icase);
        m_pattern_truncate = std::regex(R"(TRUNCATE ((A|B))", std::regex_constants::icase);
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
    // if (argc != 2) {
    //   std::cerr << "Usage: " << argv[0] << " [<host>:]<port>" << std::endl;
    // }
    std::string host{"localhost"};
    std::string port{"5000"};
    boost::asio::io_context io_context{};
    try
    {
        boost::asio::ip::tcp::resolver resolver(io_context);
        auto endpoint = resolver.resolve(host, port);
        std::make_shared<Server>(io_context, *endpoint.begin())->start();
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    };
    return 0;
}
