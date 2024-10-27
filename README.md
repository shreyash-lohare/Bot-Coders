# Halloween Multiplayer Game 🎃👻

An original, spooky multiplayer game where two players team up to survive a Halloween-themed challenge! This game, developed entirely in C, features a custom-built networking library for seamless multiplayer interactions and a unique survival mechanic involving an AI-controlled ghost.

## Table of Contents
- [About the Game](#about-the-game)
- [Features](#features)
- [Installation](#installation)
- [How to Play](#how-to-play)
- [Technical Details](#technical-details)
- [Contributing](#contributing)
- [License](#license)

## About the Game
In this spine-chilling Halloween adventure, two players must work together to survive 45 seconds against a relentless, AI-controlled ghost. The game takes place on a haunted map filled with surprises, including three mysterious portals. These portals randomly teleport players—and the ghost—to unpredictable locations on the map. This can either be a blessing or a curse, allowing players to evade the ghost or, with some bad luck, end up face-to-face with it. Can you outwit the ghost and make it through the full 45 seconds?

### Current Status and Future Plans
Currently, both players can play on the same device locally. Our next goal is to implement client-server functionality, allowing players to connect remotely from different devices. This enhancement will enable a more immersive multiplayer experience over a network.

## Features
- **Multiplayer Survival Gameplay**: Join forces with another player in a cooperative survival experience against an AI ghost.
- **AI-Controlled Ghost**: An intelligent ghost hunts players, creating tension and excitement.
- **Random Teleportation Portals**: Three portals on the map teleport players and the ghost to random positions, adding an element of surprise and strategy.
- **Client-Server Architecture (Upcoming)**: In addition to current local play, we are working to support client-server connections so players can join remotely from different devices.
- **Cross-Platform Compatibility**: Primarily designed for GNU/Linux, with potential support for Windows and macOS.

## How to Play
1. Both players connect to the game on the same device (remote play will be added soon).
2. Once the game begins, navigate the haunted map and avoid the ghost for 45 seconds to win.
3. Use the portals strategically, as they can either help you escape or lead you directly into the ghost’s path.

*Note*: A more detailed guide on gameplay mechanics will be provided as development progresses.

## Technical Details
- **Custom Networking Library**: Built on top of the socket API in C, this library handles player connections and messaging.
- **Client-Server Model (Upcoming)**: We are extending the game’s architecture to support a client-server model for remote multiplayer gameplay.
- **Code Quality**: Clean, modular code that can be extended and maintained easily for future enhancements.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request to suggest improvements, report bugs, or add features.

## License
This project is open-source and available under the [MIT License](LICENSE).

---

