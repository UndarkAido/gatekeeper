[![CMake](https://github.com/DiscordPP/echo-bot/workflows/CMake/badge.svg)](https://github.com/DiscordPP/echo-bot/actions?query=workflow%3ACMake)

# Discord++ example bot, Echo<sup>Echo<sup>Echo</sup></sup>

## Community & Support <sup>Some incredibly nice people here!</sup>
[Discord++ Discord Server](https://discord.gg/0usP6xmT4sQ4kIDh)

## How to Get Started <sup>Let's get it sta-arted! Let's get it started in here!</sup>

### Install Prerequisites
#### Install Git
 * If you want to use Git in the command line, run `sudo apt install git` on Ubuntu, on Windows you can download [Git for Windows](https://git-scm.com/) or use `choco install git` if you have Chocolatey installed.
 * If you're less terminal-inclined, you can install a Git GUI wrapper. [There's a nice list here.](https://git-scm.com/downloads/guis/)
    * I would personally recommend [GitKraken](https://www.gitkraken.com/). The free version has everyhting you need, (although their conflict resolver for Pro is nice,) and you can also get Pro free with [GitHub Education](https://education.github.com/)!
#### <sup>AND</sup>Install CMake
 * Debian-based (Ubuntu) run `sudo apt install cmake`
 * Fedora/RHEL-based `sudo dnf install cmake`
 * Windows Download [CMake](https://cmake.org/download/) or run `choco install cmake` if you have Chocolatey installed.
#### <sup>AND</sup> Install C++
 1. Install a C++ compiler
    * the typical standard on Linux is `gcc`, you can install it on Ubuntu with `sudo apt install gcc`
    * `clang` is an alternative that I feel has better error statements, you can install it on Ubuntu with `sudo apt install clang`
    * The typical compiler on Windows is [Microsoft Visual C++](https://www.visualstudio.com/)
    * [MinGW](https://www.msys2.org/) or [Cygwin](https://www.cygwin.com/) are valid alternatives if you like GCC running on Windows, there's also [WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10) which allows  you to run a Linux based distribution under Windows.
#### <sup>AND OPTIONALLY</sup>Install vcpkg
 * Vcpkg is a cross platform library manager, you can install it by reading the instructions on this [page](https://github.com/microsoft/vcpkg)
 * **This program is required for building Echo under Windows.**
#### <sup>AND</sup> Install Libraries
 * Debian-based (Ubuntu) `sudo apt install build-essential libboost-all-dev libssl-dev`
 * Fedora/RHEL-based `sudo dnf install openssl-devel boost-devel`
 * Vcpkg `vcpkg install boost-system boost-beast openssl`
#### <sup>OR</sup> Install Everything at Once
 * Debian-based (Ubuntu) `sudo apt install git clang cmake build-essential libboost-all-dev libssl-dev`

### Set Up Repository
#### Fork Echo <sup>Optional, I guess</sup>
 * Click the 'Fork' button in the upper-right of the repo's page
 * If you're in an organization, elect the where you want the fork to go
 * Click on your new 'Settings' tab, and rename your new repo
 
#### Clone the Code
Run `git clone --recursive https://github.com/discordpp/echo-bot.git` in your desired directory.
If you made a fork, replace `echo-bot` as applicable.

### Register a Bot
 1. Go to Discord's [Developer Portal 'Application' page](https://discordapp.com/developers/applications/), it will prompt you to sign in <sup>By the way, do you have 2FA enabled? It's a really good idea, especially with bots on your account</sup>
 2. Click 'New Application', and enter a name for your bot
 3. Click on 'Bot' under 'Settings' on the right, click 'Add Bot', and then 'Yes, do it!'
    * Make note of the 'Copy' button under 'Token', we'll get back to this later
    * Also, come back and click the 'regenerate' button if anyone ever gets a hold of your bot's token
    * If you want to give your bot an avatar, I'd recommend doing it both on the 'General Information' landing page and here on the 'Bot' page, it makes things less confusing if you ever need to come back.

### Add your Bot to a Server
 1. Copy `https://discordapp.com/api/oauth2/authorize?client_id=[ID]&scope=bot&permissions=[permissions]` to somewhere, such as notepad
 1. Go back to the 'General Information' page of your application
 2. Click 'Copy' under 'Client ID', and paste it into your URL, taking care to replace the word and brackets
 3. Go to the 'Bot' page
 4. Select the permissions you want your bot to have from under 'Bot Permissions'
    * You probably want at least 'Send Messages', `2048`
 5. Click the 'Copy' button under 'Permissions Integer', and paste it in your URL, make sure you get the brackets like before
 6. Paste the URL into a web btowser, select a server to add it to, and click 'Authorize'
 

### Build your Bot
 1. Create a `build` directory (folder) with `mkdir build`
 2. Enter the folder with `cd build`
 3. Go back to your Application's 'bot' page from before, and copy the token. Run `echo Bot [token] > token.dat`, replacing `[token]` with your bot's token.  Alternatively, you can set the `ECHO_BOT_TOKEN` environment variable to `Bot [token]`.
 4. Build the makefiles with `cmake ../`
     * If you are using vcpkg then run `cmake -DCMAKE_TOOLCHAIN_FILE="your vcpkg.cmake folder" ../`
 5. Build the bot with `make` (if you are using Visual Studio open `echo_bot.sln` and build from there)
 6. Run the bot with `./echo_bot`
    * You can change the executable name by modifying the `project` command in `CMakeLists.txt`
 7. ???
 8. Profit! Mention your bot in a scope it can read & write in, it should echo back your message.

## Contributing
 * Echo is pretty simple and probably doesn't need much, but if you have a suggestion or spot an issue, make a pull request on Github!
 * Also, this repo needs better jokes.
