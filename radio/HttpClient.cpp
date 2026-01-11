#include "HttpClient.h"

#include <fmt/format.h>
#include <wpi/net/uv/Buffer.hpp>

HttpClient::HttpClient(wpi::net::uv::Loop& loop, wpi::util::Logger& logger)
    : m_loop(loop), m_logger(logger) {}

HttpClient::~HttpClient() {
    if (m_tcpConnector) {
        m_tcpConnector->Close();
    }
}

void HttpClient::Get(const std::string& url) {
    if (m_requestPending) {
        return;  // Skip if previous request still pending
    }
    
    m_requestPending = true;
    m_bodyBuffer.clear();
    m_statusCode = 0;
    m_messageComplete = false;
    
    // Parse URL
    bool error = false;
    std::string errorMsg;
    wpi::net::HttpLocation loc{url, &error, &errorMsg};
    if (error) {
        printf("Failed to parse URL: %s\n", errorMsg.c_str());
        m_requestPending = false;
        completed(0, "");
        return;
    }
    
    wpi::net::HttpRequest httpReq{loc};
    
    // Create HTTP parser
    m_httpParser = std::make_unique<wpi::net::HttpParser>(wpi::net::HttpParser::kResponse);
    
    // Set up parser callbacks
    m_httpParser->status.connect([this](std::string_view statusStr) {
        m_statusCode = m_httpParser->GetStatusCode();
    });
    
    m_httpParser->body.connect([this](std::string_view data, bool isFinal) {
        m_bodyBuffer.append(data);
    });
    
    m_httpParser->messageComplete.connect([this](bool shouldKeepAlive) {
        m_requestPending = false;
        m_messageComplete = true;
        
        completed(m_statusCode, m_bodyBuffer);
    });
    
    // Create TCP connector if needed
    if (!m_tcpConnector) {
        m_tcpConnector = wpi::net::ParallelTcpConnector::Create(
            m_loop, wpi::net::uv::Timer::Time{5000}, m_logger,
            [this, httpReq](wpi::net::uv::Tcp& tcp) {
                OnTcpConnected(tcp, httpReq);
            },
            true);
        
        if (!m_tcpConnector) {
            printf("Failed to create TCP connector\n");
            m_requestPending = false;
            completed(0, "");
            return;
        }
        
        std::array<std::pair<std::string, unsigned int>, 1> servers;
        servers[0] = {std::string{httpReq.host}, static_cast<unsigned int>(httpReq.port)};
        m_tcpConnector->SetServers(servers);
    } else {
        m_tcpConnector->Disconnected();
    }
}

void HttpClient::OnTcpConnected(wpi::net::uv::Tcp& tcp, const wpi::net::HttpRequest& httpReq) {
    m_tcpConn = tcp.shared_from_this();
    m_tcpConnector->Succeeded(tcp);
    tcp.StartRead();
    
    // Send HTTP GET request
    std::string request = fmt::format(
        "GET /{} HTTP/1.0\r\n"
        "Host: {}\r\n"
        "\r\n",
        httpReq.path, httpReq.host);
    
    tcp.Write({wpi::net::uv::Buffer(request)},
              [](auto bufs, wpi::net::uv::Error err) {
                  if (err) {
                      printf("HTTP write error: %s\n", err.str());
                  }
              });
    
    // Handle incoming data
    tcp.data.connect([this](wpi::net::uv::Buffer& buf, size_t size) {
        if (m_httpParser) {
            std::string_view data{buf.base, size};
            m_httpParser->Execute(data);
            
            if (m_messageComplete) {
                m_bodyBuffer.clear();
                m_httpParser.reset();
            }
            
            if (m_httpParser && m_httpParser->HasError()) {
                printf("HTTP parse error: %s\n",
                       http_errno_description(m_httpParser->GetError()));
                m_requestPending = false;
                m_bodyBuffer.clear();
                m_httpParser.reset();
                
                if (auto conn = m_tcpConn.lock()) {
                    conn->Close();
                }
                
                completed(0, "");
            }
        }
    });
    
    // Handle connection end
    tcp.end.connect([this]() {
        if (m_httpParser) {
            m_httpParser->Execute("");
            
            if (m_messageComplete) {
                m_bodyBuffer.clear();
                m_httpParser.reset();
            }
            
            if (m_httpParser && m_httpParser->HasError()) {
                printf("HTTP parse error on EOF: %s\n",
                       http_errno_description(m_httpParser->GetError()));
                m_requestPending = false;
                m_bodyBuffer.clear();
                m_httpParser.reset();
                
                completed(0, "");
            }
        }
        
        if (auto conn = m_tcpConn.lock()) {
            conn->Close();
        }
    });
    
    // Handle errors
    tcp.error.connect([this](wpi::net::uv::Error err) {
        printf("HTTP connection error: %s\n", err.str());
        m_requestPending = false;
        m_bodyBuffer.clear();
        m_httpParser.reset();
        
        completed(0, "");
        
        if (m_tcpConnector) {
            m_tcpConnector->Disconnected();
        }
    });
}
