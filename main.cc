#include <fstream>
#include <iostream>
#include <random>
#include <regex>

#include <boost/asio.hpp>

#include <discordpp/bot.hh>
#include <discordpp/log.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/plugin-ratelimit.hh>
#include <discordpp/plugin-responder.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-simpleweb.hh>

#define GUILD "164234463247597568"
#define QUIZCHANNEL "742540278484959322"
#define KEYROLE "742606708383154177"
#define LOCKROLE "761803600199680001"
#define GREETCHANNEL "164234463247597568"
#define LOGCHANNEL "475052413775249408"

namespace asio = boost::asio;
using json = nlohmann::json;
namespace dpp = discordpp;

using DppBot = dpp::PluginResponder<dpp::PluginOverload<
    dpp::PluginRateLimit<dpp::WebsocketSimpleWeb<dpp::RestBeast<dpp::Bot>>>>>;

std::string getToken();

std::istream &safeGetline(std::istream &is, std::string &t);

void filter(std::string &target, const std::string &pattern);

void greet(std::shared_ptr<DppBot> bot, std::string &questionID, int &answer,
           const json &user);

void printAndTell(std::shared_ptr<DppBot> bot, std::string message);

int main() {
    dpp::log::filter = dpp::log::info;
    dpp::log::out = &std::cerr;

    std::cout
        << "Howdy, and thanks for trying out Discord++ Gatekeeper!\n"
        << "Feel free to drop into the official server at "
           "https://discord.gg/0usP6xmT4sQ4kIDh if you have any questions.\n\n"
        << std::flush;

    std::cout << "Starting bot...\n\n";

    std::string token = getToken();
    if (token.empty()) {
        std::cerr
            << "CRITICAL: "
            << "There is no valid way for Gatekeeper to obtain a token! Use "
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

    // Don't show heartbeat status
    bot->showHeartbeats = false;

    json self;
    bot->handlers.insert(
        {"READY", [&self](json data) { self = data["user"]; }});

    bot->prefix = "!";

    bot->respond("help", "`help`: Print this message.\n"
                         "`join`: Re-send the join prompt");

    std::string questionID;
    int answer;

    bot->handlers.insert(
        {"GUILD_MEMBER_ADD", [&bot, &questionID, &answer](const json &msg) {
             if (msg["guild_id"].get<std::string>() != GUILD)
                 return;
             std::cout << msg["user"]["username"] << " joined.\n";
             greet(bot, questionID, answer, msg["user"]);
             /*bot->call(
                 "PUT",
                 "/guilds/" GUILD "/members/" +
                     msg["user"]["id"].get<std::string>() + "/roles/" ROLE,
                 [&bot, &questionID, &answer, msg](bool fail, const json &) {
                     greet(bot, questionID, answer, msg["user"]);
                 });*/
         }});

    bot->respond("join", [&bot, &questionID, &answer](const json &msg) {
        if (msg["guild_id"].get<std::string>() != GUILD ||
            msg["channel_id"].get<std::string>() != QUIZCHANNEL)
            return;
        printAndTell(bot, msg["author"]["username"].get<std::string>() +
                              " requested a quiz.");
        greet(bot, questionID, answer, msg["author"]);
    });

    bot->handlers.insert(
        {"MESSAGE_REACTION_ADD",
         [bot, &self, &questionID, &answer](const json &msg) {
             if (msg["guild_id"].get<std::string>() != GUILD ||
                 msg["channel_id"].get<std::string>() != QUIZCHANNEL ||
                 msg["message_id"].get<std::string>() != questionID ||
                 msg["member"]["user"]["id"].get<std::string>() ==
                     self["id"].get<std::string>())
                 return;

             bool isKeyed = false;
             for (const auto &role : msg["member"]["roles"]) {
                 isKeyed = isKeyed || role.get<std::string>() == KEYROLE;
             }
             if (isKeyed)
                 return;

             std::string strAnswer = "🇦";
             strAnswer[3] += answer;

             bool passed = msg["emoji"]["name"].get<std::string>() == strAnswer;

             printAndTell(bot,
                          msg["member"]["user"]["username"].get<std::string>() +
                              " " + (passed ? "passed" : "failed") +
                              " the quiz.\n");

             bot->call("DELETE",
                       "/channels/" QUIZCHANNEL "/messages/" + questionID);

             bot->call("PUT",
                       "/guilds/" GUILD "/members/" +
                           msg["member"]["user"]["id"].get<std::string>() +
                           "/roles/" + (passed ? KEYROLE : LOCKROLE));
         }});

    // Create Asio context, this handles async stuff.
    auto aioc = std::make_shared<asio::io_context>();

    // Set the bot up
    bot->initBot(6, token, aioc);

    // Run the bot!
    bot->run();

    return 0;
}

void greet(std::shared_ptr<DppBot> bot, std::string &questionID, int &answer,
           const json &user) {
    if (!questionID.empty()) {
        bot->call("DELETE", "/channels/" QUIZCHANNEL "/messages/" + questionID);
    }

    std::ostringstream content;
    content << "Howdy, <@" << user["id"].get<std::string>()
            << ">! To join the server, please answer the following question "
               "by reacting:\n**What is Discord++?**\n";

    static const std::string correct = "A C++ library";
    static const std::vector<std::string> incorrect{
        "A hacked app", "A way to get free Nitro",
        "A hacking/griefing toolset"};
    static auto rng = std::mt19937(std::random_device()());
    static std::uniform_int_distribution<int> distribution(0, incorrect.size());

    std::vector<std::string> choices(incorrect);
    std::shuffle(choices.begin(), choices.end(), rng);
    answer = distribution(rng);

    choices.insert(choices.begin() + answer, correct);

    for (int i = 0; i < choices.size(); i++) {
        content << ":regional_indicator_" << (char)('a' + i) << ": "
                << choices[i] << '\n';
    }

    bot->call(
        "POST", "/channels/" QUIZCHANNEL "/messages",
        {{"content", content.str()}},
        [bot, &questionID](bool fail, const json &res) {
            if (fail)
                return;
            questionID = res["id"].get<std::string>();
            bot->call(
                "PUT",
                "/channels/" QUIZCHANNEL "/messages/" + questionID +
                    "/reactions/%F0%9F%87%A6/@me",
                [bot, &questionID](bool fail, const json &) {
                    if (fail)
                        return;
                    bot->call(
                        "PUT",
                        "/channels/" QUIZCHANNEL "/messages/" + questionID +
                            "/reactions/%F0%9F%87%A7/@me",
                        [bot, &questionID](bool fail, const json &) {
                            if (fail)
                                return;
                            bot->call(
                                "PUT",
                                "/channels/" QUIZCHANNEL "/messages/" +
                                    questionID + "/reactions/%F0%9F%87%A8/@me",
                                [bot, &questionID](bool fail, const json &) {
                                    if (fail)
                                        return;
                                    bot->call(
                                        "PUT",
                                        "/channels/" QUIZCHANNEL "/"
                                        "messages/" +
                                            questionID +
                                            "/reactions/%F0%9F%87%A9/@me");
                                });
                        });
                });
        });
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

void printAndTell(std::shared_ptr<DppBot> bot, std::string message) {
    std::cout << message << '\n';
    bot->call("POST", "/channels/" LOGCHANNEL "/messages",
             {{"content", message}});
}
