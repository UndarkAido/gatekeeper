#include <fstream>
#include <iostream>
#include <regex>

#include <boost/asio.hpp>

#include <discordpp/bot.hh>
#include <discordpp/log.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/plugin-ratelimit.hh>
#include <discordpp/plugin-responder.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-beast.hh>

namespace asio = boost::asio;
using json = nlohmann::json;
namespace dpp = discordpp;

using DppBot = dpp::PluginResponder<dpp::PluginOverload<
    dpp::PluginRateLimit<dpp::WebsocketBeast<dpp::RestBeast<dpp::Bot>>>>>;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void filter(std::string &target, const std::string &pattern);

int main() {
    dpp::log::filter = dpp::log::info;
    dpp::log::out = &std::cerr;

    std::cout
        << "Howdy, and thanks for trying out Discord++!\n"
        << "Feel free to drop into the official server at "
           "https://discord.gg/0usP6xmT4sQ4kIDh if you have any questions.\n\n"
        << std::flush;

    std::cout << "Starting bot...\n\n";

    std::string token = getToken();
    if (token.empty()) {
        std::cerr << "CRITICAL: "
                  << "There is no valid way for Echo to obtain a token! Use "
                     "one of the following ways:"
                  << std::endl
                  << "(1) Fill the BOT_TOKEN environment variable with the "
                     "token (e.g. 'Bot 123456abcdef')."
                  << std::endl
                  << "(2) Copy the example `token.eg.dat` as `token.dat` and "
                     "write your own token to it.\n";
        exit(1);
    }

    // Create Bot object
    auto bot = std::make_shared<DppBot>();
    // Don't complain about unhandled events
    bot->debugUnhandled = false;

    /*/
     * Create handler for the READY payload, this may be handled by the bot in
    the future.
     * The `self` object contains all information about the 'bot' user.
    /*/
    json self;
    bot->handlers.insert(
        {"READY", [&self](json data) { self = data["user"]; }});

    bot->prefix = "~";

    bot->respond("help", "Mention me and I'll echo your message back!");

    bot->respond("about", [&bot](json msg) {
        std::ostringstream content;
        content
            << "Sure thing, "
            << (msg["member"]["nick"].is_null()
                    ? msg["author"]["username"].get<std::string>()
                    : msg["member"]["nick"].get<std::string>())
            << "!\n"
            << "I'm a simple bot meant to demonstrate the Discord++ library.\n"
            << "You can learn more about Discord++ at "
               "https://discord.gg/0usP6xmT4sQ4kIDh";
        bot->call("POST",
                  "/channels/" + msg["channel_id"].get<std::string>() +
                      "/messages",
                  json({{"content", content.str()}}));
    });

    // Create handler for the MESSAGE_CREATE payload, this receives all messages
    // sent that the bot can see.
    bot->handlers.insert(
        {"MESSAGE_CREATE", [&bot, &self](json msg) {
             // Scan through mentions in the message for self
             bool mentioned = false;
             for (const json &mention : msg["mentions"]) {
                 mentioned = mentioned or mention["id"] == self["id"];
             }
             if (mentioned) {
                 // Identify and remove mentions of self from the message
                 std::string content = std::regex_replace(
                     msg["content"].get<std::string>(),
                     std::regex(R"(<@!?)" + self["id"].get<std::string>() +
                                R"(> ?)"),
                     "");

                 // Get the target user's display name
                 std::string name =
                     (msg["member"]["nick"].is_null()
                          ? msg["author"]["username"].get<std::string>()
                          : msg["member"]["nick"].get<std::string>());

                 std::cout << "Echoing " << name << '\n';

                 // Echo the created message
                 bot->call("POST",
                           "/channels/" + msg["channel_id"].get<std::string>() +
                               "/messages",
                           json({{"content", content}}));

                 // Set status to Playing "with [author]"
                 bot->send(3,
                           {{"game", {{"name", "with " + name}, {"type", 0}}},
                            {"status", "online"},
                            {"afk", false},
                            {"since", "null"}});
             }
         }});

    // Create Asio context, this handles async stuff.
    auto aioc = std::make_shared<asio::io_context>();

    // Set the bot up
    bot->initBot(6, token, aioc);

    // Run the bot!
    bot->run();

    return 0;
}

std::string getToken() {
    std::string token;

    /*
                    First attempt to read the token from the BOT_TOKEN
       environment variable.
    */
    char const *env = std::getenv("BOT_TOKEN");
    if (env != nullptr) {
        token = std::string(env);
    } else {
        /*/
         * Read token from token file.
         * Tokens are required to communicate with Discord, and hardcoding
        tokens is a bad idea.
         * If your bot is open source, make sure it's ignore by git in your
        .gitignore file.
        /*/
        std::ifstream tokenFile("token.dat");
        if (!tokenFile) {
            return "";
        }
        safeGetline(tokenFile, token);
        tokenFile.close();
    }
    return token;
}

/*/
 * Source: https://stackoverflow.com/a/6089413/1526048
/*/
std::istream &safeGetline(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if (sb->sgetc() == '\n') {
                sb->sbumpc();
            }
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if (t.empty()) {
                is.setstate(std::ios::eofbit);
            }
            return is;
        default:
            t += (char)c;
        }
    }
}
