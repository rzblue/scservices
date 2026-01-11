#pragma once

#include <functional>
#include <memory>
#include <string>

#include <wpi/net/HttpParser.hpp>
#include <wpi/net/HttpUtil.hpp>
#include <wpi/net/ParallelTcpConnector.hpp>
#include <wpi/net/uv/Loop.hpp>
#include <wpi/net/uv/Tcp.hpp>
#include <wpi/util/Logger.hpp>

class HttpClient {
public:
    using CompletionCallback = std::function<void(int statusCode, std::string body)>;
    
    HttpClient(wpi::net::uv::Loop& loop, wpi::util::Logger& logger);
    ~HttpClient();
    
    // Make an HTTP GET request
    void Get(const std::string& url, CompletionCallback callback);
    
    bool IsBusy() const { return m_requestPending; }
    
private:
    void OnTcpConnected(wpi::net::uv::Tcp& tcp, const wpi::net::HttpRequest& httpReq);
    
    wpi::net::uv::Loop& m_loop;
    wpi::util::Logger& m_logger;
    std::shared_ptr<wpi::net::ParallelTcpConnector> m_tcpConnector;
    std::weak_ptr<wpi::net::uv::Tcp> m_tcpConn;
    std::unique_ptr<wpi::net::HttpParser> m_httpParser;
    std::string m_bodyBuffer;
    int m_statusCode = 0;
    bool m_requestPending = false;
    bool m_messageComplete = false;
    CompletionCallback m_callback;
};
