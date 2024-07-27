# Dodas
Dodas stands for *"de dödas angrepp"*, literally *"attack of the dead"* and is a terminal videogame.

Dodas is written in C++ and uses the [`Sista`](https://github.com/Lioydiano/Dodas/blob/b32347f3daeb863757ac8ba256a22257d2bd6932/dodas.cpp#L256) library for terminal graphics.

## Download

You can download the game by cloning the repository:

```bash
git clone https://github.com/Lioydiano/Dodas
```

Otherwise, you can download the latest release from the [releases](https://github.com/Lioydiano/Dodas/releases) page.

## Installation

Often with the release, there is a static executable for both Windows and Linux that you can run directly, without the need to compile the game.

Since this is not always the case, and not everybody uses either Windows or Linux, you can compile the game yourself.

### Compiling

The release includes a `Makefile` that you can use to compile the game. You can compile the game by running:

```bash
make
```

In case you don't have `make` installed, you can compile the game by running:

```bash
g++ include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp dodas.cpp -o dodas
```

Add the `-static` flag if you want to compile the game statically and share it with others.

```bash
g++ -static include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp dodas.cpp -o dodas 
```

Also, in some cases you might need to specify `-std=c++17` which is the standard used in this project.

```bash
g++ -std=c++17 -static include/sista/ANSI-Settings.cpp include/sista/border.cpp include/sista/coordinates.cpp include/sista/cursor.cpp include/sista/field.cpp include/sista/pawn.cpp dodas.cpp -o dodas 
```

### Running

After compiling the game, you can run it by executing the `dodas` executable:

```bash
./dodas
```

Which on Windows can be done by running:

```batch
dodas
```

### Options

If you want your record to be considered, you must run `./dodas` with no `-U` flag and without editing constants such as `START_AMMONITION`.

If you want the game to be pausable, you must run `./dodas -U` or `./dodas -unofficial`.

If you don't want the music to be played, you must run `./dodas -M` or `./dodas --music-off`.

If you want the music to be played correctly, you may need some libraries, see this list:

- Linux: [`canberra-gtk-play`](https://askubuntu.com/questions/1175572/how-to-fix-error-failed-to-load-module-canberra-gtk-module) which must be installed with `sudo apt-get install libcanberra-gtk-module`
- MacOS: [`afplay`](https://ss64.com/mac/afplay.html) which is included in the MacOS API
- Windows: `PlaySound` from `winmm.dll` which is included in the Windows API

Also, you must have the `music` folder in the same directory as the `dodas` executable.

## How to play

### Plot

You are the red `$` symbol and you have to kill the `9` queen.

The queen defends herself by spawning bold shooting `Z` zombies and walking `Z` zombies.

The queen also spawns `=` walls to protect herself when hitted.

You must kill the queen as fast as possible, because the zombies will keep spawning.

### Controls

⚠️ sadly, on MacOS you must always press `enter` to send commands

- `w`/`a`/`s`/`d` to move (up/left/down/right, uppercase `A`/`S`/`D` also work)
- `Q` to quit (lowercase `q` won't work because it caused unintentional exits)
- `i`/`j`/`k`/`l` to shoot and build (up/left/down/right)
- `.` to pause and unpause the game

### Weapons

You can select different weapons by pressing the following keys:

- `p` to select bullets (`P` also works)
- `m` to select mines (`M` also works)
- `c` to select cannons (`C` also works)
- `b` to select bombers (`B` also works)
- `W` to select workers
- `=` to select walls (`0` also works)

### Ammunition

Each weapon has a different ammunition cost:

- Bullets: 1
- Walls: 1
- Mines: 3
- Workers: 5
- Cannons: 5
- Bombers: 7

## Entities

### Workers

Workers (underscored yellow `W`) produce ammunition for you: more workers, more ammunition.

### Walls

Walls (yellow `=`) are used to protect anybody from bullets and enemies in general.

### Mines

Mines (blinking purple `*`) are used to kill enemies that step on them.

### Cannons

Cannons (red `C`) are used to kill enemies in a straight line shooting bullets.

Cannons shoot more bullets if there are more workers directly behind them.

```bash
 WWWC    >
```

### Bombers

Bombers (blue `B`) are used to kill enemies in a square 5x5 area.

### Zombies

Zombies (faint `Z`) defend the queen by attacking you.

### Walkers

Walkers (bold `Z`) are zombies that walk towards you.

They are slow, they walk generally in a straight line and they are easy to kill.

They are able to destroy walls and workers, and they explode in a 3x3 area when they reach the left border, removing also all your ammonition.

### Queen

The queen (bold `9`) is the main enemy of the game.

She spawns zombies and walls to protect herself.

She only moves on the y-axis, so she's always close to the right border.

Despite not being intelligent in any way, her movements are unpredictable.

# Acknowledgments

- [Sista](https://github.com/Lioydiano/Sista) library for terminal graphics
- Luca Corradin for the idea of the game
- Francesco Corradin for beta testing and suggestions
- FLAK-ZOSO for writing the code
- Il Pensionato Sentenzioso ([Karmolupe](https://karmolupe.bandcamp.com/)) for the music, still in development

# Screenshots

![v0.3.0 speedrun record](images/v0.3%20record.png)
![Bombers exploding, breaking walls and hitting the queen](images/bombers%20and%20broken%20walls.png)
