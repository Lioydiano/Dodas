#include "cross_platform.hpp"
#include "dodas.hpp"
#include <algorithm>
#include <fstream>
#include <thread>
#include <chrono>
#include <mutex>

#if DEBUG
std::ofstream debug("debug.log");
#endif

sista::SwappableField* field;

std::vector<Bullet*> Bullet::bullets;
std::vector<EnemyBullet*> EnemyBullet::enemyBullets;
std::vector<Zombie*> Zombie::zombies;
std::vector<Walker*> Walker::walkers;
std::vector<Wall*> Wall::walls;
std::vector<Mine*> Mine::mines;
std::vector<Cannon*> Cannon::cannons;
std::vector<ArmedWorker*> ArmedWorker::armedWorkers;
std::vector<Worker*> Worker::workers;
std::vector<Bomber*> Bomber::bombers;
Player* Player::player;
Queen* Queen::queen;

std::bernoulli_distribution Zombie::distribution(ZOMBIE_MOVING_PROBABILITY);
std::bernoulli_distribution Zombie::shootDistribution(ZOMBIE_SHOOTING_PROBABILITY);
std::bernoulli_distribution Walker::distribution(WALKER_MOVING_PROBABILITY);
sista::Cursor cursor;
std::mutex inputOutputMutex;
bool pause_ = false;
bool end = false;

int main(int argc, char** argv) {
    #ifdef __APPLE__
        term_echooff();
    #endif
    std::ios_base::sync_with_stdio(false);
    ANSI::reset(); // Reset the settings
    srand(time(0)); // Seed the random number generator
    #if INTRO
        printIntro();
    #endif
    sista::SwappableField field_(50, 20);
    field = &field_;
    field->clear();
    sista::Border border(
        '#', {
            ANSI::ForegroundColor::F_WHITE,
            ANSI::BackgroundColor::B_BLACK,
            ANSI::Attribute::BRIGHT
        }
    );
    bool unofficial = false;
    bool music = true;
    bool endless = false;
    bool hardcore = false;
    if (argc > 1) {
        for (unsigned short i=1; i<argc; i++) {
            // if argv contains "--unofficial" or "-u" then the game will be played in the unofficial mode
            if (std::string(argv[i]) == "--unofficial" || std::string(argv[i]) == "-u" || std::string(argv[i]) == "-U") {
                border = sista::Border('0', {
                        ANSI::ForegroundColor::F_WHITE,
                        ANSI::BackgroundColor::B_BLACK,
                        ANSI::Attribute::BRIGHT
                    }
                ); // An unofficial run is marked with a '0' in the border
                unofficial = true;
            }
            // if argv contains "--music-off" or "-m" then the music will be turned off
            if (std::string(argv[i]) == "--music-off" || std::string(argv[i]) == "-m" || std::string(argv[i]) == "-M") {
                // The music thread will be joined and the game will be played without music
                music = false;
            }
            // if argv contains "--endless" or "-E" then the game will be played in the endless mode
            if (std::string(argv[i]) == "--endless" || std::string(argv[i]) == "-e" || std::string(argv[i]) == "-E") {
                endless = true;
            }
            // if argv contains "--hardcore" or "-H" then the game will be played in the hardcore mode
            if (std::string(argv[i]) == "--hardcore" || std::string(argv[i]) == "-h" || std::string(argv[i]) == "-H") {
                hardcore = true;
            }
        }
    }

    field->addPrintPawn(Player::player = new Player({10, 18}));
    field->addPrintPawn(Queen::queen = new Queen({10, 49}));
    for (unsigned short j=0; j<20; j++) {
        Wall* wall = new Wall({j, 30}, 3); // There is a vertical macrowall in the middle of the field
        Wall::walls.push_back(wall);
        field->addPrintPawn(wall);
        if (j % 5 == 1) {
            Zombie* zombie = new Zombie({j, 47}); // Zombies are spawned on the right side of the field (the mother side)
            Zombie::zombies.push_back(zombie);
            field->addPrintPawn(zombie);
        }
        if (j % 5 == 3) {
            Walker* walker = new Walker({j, 45}); // Walkers are spawned on the right side of the field (the mother side)
            Walker::walkers.push_back(walker);
            field->addPrintPawn(walker);
        }
        if (j % 5 == 2) {
            Worker* worker = new Worker({j, 1}); // Workers are spawned on the left side of the field (the player side)
            Worker::workers.push_back(worker);
            field->addPrintPawn(worker);
        }
    }
    field->print(border);
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));    
    std::thread th = std::thread([&]() {
        char input = '_';
        while (input != 'Q' /*&& input != 'q'*/) {
            if (end) return;
            #if defined(_WIN32) or defined(__linux__)
                input = getch();
            #elif __APPLE__
                input = getchar();
            #endif
            if (end) return;
            switch (input) {
            case 'w': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->move(Direction::UP);
                break;
            }
            case 'd': case 'D': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->move(Direction::RIGHT);
                break;
            }
            case 's': case 'S': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->move(Direction::DOWN);
                break;
            }
            case 'a': case 'A': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->move(Direction::LEFT);
                break;
            }
            case 'j': case 'J': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->shoot(Direction::LEFT);
                break;
            }
            case 'k': case 'K': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->shoot(Direction::DOWN);
                break;
            }
            case 'l': case 'L': {
                Player::player->shoot(Direction::RIGHT);
                break;
            }
            case 'i': case 'I': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->shoot(Direction::UP);
                break;
            }
            case 'p': case 'P': { // "Projectile" or something
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::BULLET;
                break;
            }
            case 'm': case 'M': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::MINE;
                break;
            }
            case 'c': case 'C': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::CANNON;
                break;
            }
            case 'b': case 'B': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::BOMBER;
                break;
            }
            case 'W': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::WORKER;
                break;
            }
            case 'u': case 'U' : {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::ARMED_WORKER;
                break;
            }
            case '=': case '0': {
                std::lock_guard<std::mutex> lock(inputOutputMutex);
                Player::player->weapon = Type::WALL;
                break;
            }
            case '.':
                pause_ = !pause_;
                break;
            case 'Q': /* case 'q': */
                end = true;
                return;
            default:
                break;
            }
        }
    });
    std::thread music_th;
    if (music) {
        const int genresProbSize = 3; // The number of genres
        int genresProb[] = {3, 6, 1}; // The probability of each genre
        int genresSize_[] = {0, 4, 4, 4}; // The number of tracks for each genre
        std::discrete_distribution<int> genresDistribution(genresProb, genresProb + genresProbSize);
        std::string genres[] = {"F", "MH", "ML", "P"}; // The genres of the music
        std::unordered_map<std::string, int> length_ = { // The length of each track
            {"MH1", 10}, {"MH2", 16}, {"MH3", 16}, {"MH4", 8},
            {"ML1", 4}, {"ML2", 6}, {"ML3", 6}, {"ML4", 6},
            {"P1", 4}, {"P2", 8}, {"P3", 6}, {"P4", 6}
        };
        int length[4][5] = {
            {0, 0, 0, 0, 0},
            {0, 10, 16, 16, 8},
            {0, 4, 6, 6, 6},
            {0, 4, 8, 6, 6}
        };
        // debug << "Before music thread" << std::endl;
        music_th = std::thread([&]() {
            int n, genre;
            #ifdef __APPLE__
            std::vector<int> extendedProb;
            for (unsigned i=0; i<genresProbSize; i++) {
                for (unsigned j=0; j<genresProb[i]; j++) {
                    extendedProb.push_back(i+1);
                }
            }
            while (!end) {
                genre = extendedProb[rand() % extendedProb.size()];
                // debug << "Genre: " << genre << std::endl;
                n = (rand() % genresSize_[genre]) + 1;
                // debug << "n: " << n << std::endl;
                std::string track = genres[genre] + std::to_string(n);
                // debug << "Playing " << track << std::endl;
                try {
                    char buf[1024];
                    snprintf(buf, 1024, "afplay \"audio/%s.mp3\"", track.c_str());
                    system(buf);
                } catch (std::exception& e) {
                    return; // If the music can't be played, the thread ends
                }
                while (pause_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            #elif _WIN32
            while (!end) {
                // debug << "Before genre" << std::endl;
                genre = genresDistribution(rng);
                // debug << "Genre: " << genre << std::endl;
                n = (rand() % genresSize_[genre]) + 1;
                // debug << "n: " << n << std::endl;
                std::string track = genres[genre] + std::to_string(n);
                try {
                    mciSendString((LPCSTR)("play audio/" + track + ".wav").c_str(), NULL, 0, NULL);
                    // debug << "After PlaySound" << std::endl;
                    int wait = length[genre][n];
                    wait *= 1000;
                    wait -= WIN_API_MUSIC_DELAY; // Some time is wasted in API calls, so we have to compensate for that
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
                    // debug << "Already slept for " << wait << " seconds" << std::endl;
                } catch (std::exception& e) {
                    // debug << "Exception" << std::endl;
                    // debug << e.what() << std::endl;
                    return; // If the music can't be played, the thread ends
                }
                if (pause_) {
                    // debug << "Pausing" << std::endl;
                    PlaySound(NULL, 0, 0);
                    // debug << "Paused" << std::endl;
                    // break;
                }
                while (pause_) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            #elif __linux__
            while (!end) {
                genre = genresDistribution(rng);
                // debug << "Genre: " << genre << std::endl;
                n = (rand() % genresSize_[genre]) + 1;
                // debug << "n: " << n << std::endl;
                std::string track = ((std::string)"audio/") + genres[genre] + std::to_string(n) + (std::string)".ogg";
                try {
                    system(("canberra-gtk-play -f " + track).c_str());
                    // debug << "After canberra-gtk-play" << std::endl;
                } catch (std::exception& e) {
                    #if DEBUG
                    debug << e.what() << std::endl;
                    #endif
                    return; // If the music can't be played, the thread ends
                }
                while (pause_) {
                    // debug << "Pausing" << std::endl;
                    if (end) return;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
            #endif
        });
    }
    for (unsigned i=0; !end; i++) {
        if (unofficial) {
            while (pause_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } // So the game doesn't run while paused, and the speedrun is not affected, so it's unofficial
        } else if (pause_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue; // So the game keeps increasing the frame counter, and the speedrun is affected by the pause
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(inputOutputMutex);
        // debug << "Frame: " << i << std::endl;
        #if DEBUG
        for (unsigned j=0; j<Bullet::bullets.size(); j++) {
            // debug << "\tBullet " << j << ": " << Bullet::bullets[j] << std::endl;
        }
        #endif
        // removeNullptrs((std::vector<Entity*>&)Bullet::bullets);
        for (unsigned j=0; j<Bullet::bullets.size(); j++) {
            if (j >= Bullet::bullets.size()) break;
            // debug << "\tBullet " << j << std::endl;
            Bullet* bullet = Bullet::bullets[j];
            if (bullet == nullptr) continue;
            // debug << "\t\tNot nullptr " << bullet << std::endl;
            if (bullet->collided) continue;
            // debug << "\t\tNot collided" << std::endl;
            bullet->move();
            // debug << "\t\tAfter move" << std::endl;
        }
        // debug << "\tAfter bullets" << std::endl;
        for (auto enemyBullet : EnemyBullet::enemyBullets) {
            if (enemyBullet->collided) {
                EnemyBullet::removeEnemyBullet(enemyBullet);
            }
        }
        // debug << "\tAfter enemy bullets deletion" << std::endl;
        for (auto bullet : Bullet::bullets) {
            if (bullet->collided) {
                Bullet::removeBullet(bullet);
            }
        }
        // debug << "\tAfter bullets deletion" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Bullet::bullets);
        // removeNullptrs((std::vector<Entity*>&)EnemyBullet::enemyBullets);
        for (unsigned j=0; j<EnemyBullet::enemyBullets.size(); j++) {
            if (j >= EnemyBullet::enemyBullets.size()) break;
            EnemyBullet* enemyBullet = EnemyBullet::enemyBullets[j];
            if (enemyBullet == nullptr) continue;
            if (enemyBullet->collided) continue;
            enemyBullet->move();
        }
        // debug << "\tAfter enemy bullets" << std::endl;
        for (auto enemyBullet : EnemyBullet::enemyBullets) {
            if (enemyBullet->collided) {
                EnemyBullet::removeEnemyBullet(enemyBullet);
            }
        }
        // debug << "\tAfter enemy bullets deletion" << std::endl;
        for (auto bullet : Bullet::bullets) {
            if (bullet->collided) {
                Bullet::removeBullet(bullet);
            }
        }
        // debug << "\tAfter bullets deletion" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)EnemyBullet::enemyBullets);
        // removeNullptrs((std::vector<Entity*>&)Zombie::zombies);
        for (auto zombie : Zombie::zombies) {
            if (Zombie::distribution(rng))
                zombie->move();
        }
        // debug << "\tAfter zombies" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Walker::walkers);
        for (auto zombie : Zombie::zombies)
            if (Zombie::shootDistribution(rng))
                zombie->shoot();
        // debug << "\tAfter zombies shooting" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Walker::walkers);
        for (auto walker : Walker::walkers)
            if (Walker::distribution(rng))
                walker->move();
        // debug << "\tAfter walkers" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Mine::mines);
        for (auto mine : Mine::mines)
            mine->checkTrigger();
        // debug << "\tAfter mines" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Worker::workers);        
        std::vector<std::vector<unsigned short>> workersPositions(20, std::vector<unsigned short>()); // workersPositions[y] = {x1, x2, x3, ...} where the workers are
        for (auto worker : Worker::workers) {
            workersPositions[worker->getCoordinates().y].push_back(worker->getCoordinates().x);
            if (worker->distribution(rng))
                worker->produce();
        }
        // debug << "\tAfter workers" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Cannon::cannons);
        for (auto worker : ArmedWorker::armedWorkers) {
            if (worker->distribution(rng))
                worker->produce();
            worker->dodgeIfNeeded();
        }

        for (auto cannon : Cannon::cannons) {
            cannon->recomputeDistribution(workersPositions);
            if (cannon->distribution(rng))
                cannon->fire();
        }
        // debug << "\tAfter cannons" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Bomber::bombers);
        for (unsigned j = 0; j < Bomber::bombers.size(); j++) {
            Bomber* bomber = Bomber::bombers[j];
            if (bomber == nullptr) continue;
            bomber->move();
        }
        // debug << "\tAfter bombers" << std::endl;
        try {
            Queen::queen->move();
        } catch (std::exception& e) {
            // Nothing to do here
        }
        // debug << "\tAfter queen" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Wall::walls);
        for (unsigned j = 0; j < Wall::walls.size(); j++) {
            Wall* wall = Wall::walls[j];
            if (wall == nullptr) continue;
            if (wall->strength == 0) {
                Wall::removeWall(wall);
            }
        }
        // debug << "\tAfter walls" << std::endl;
        // removeNullptrs((std::vector<Entity*>&)Bullet::bullets);
        for (unsigned j=0; j<Mine::mines.size(); j++) {
            if (j >= Mine::mines.size()) break;
            if (Mine::mines[j] == nullptr) continue;
            if (Mine::mines[j]->triggered) {
                Mine::mines[j]->explode();
                Mine::removeMine(Mine::mines[j]);
            }
        }
        // debug << "\tAfter mines explosion" << std::endl;

        if (i % 100 == 0) {
            unsigned short y = rand() % 20;
            if (Queen::queen->getCoordinates().y != y) {
                Walker* walker = new Walker({y, 49});
                Walker::walkers.push_back(walker);
                field->addPrintPawn(walker);
            }
        }
        // debug << "\tAfter walkers spawning" << std::endl;
        if (i % 200 == 0) {
            unsigned short y = rand() % 20;
            if (Queen::queen->getCoordinates().y != y) {
                Zombie* zombie = new Zombie({y, 49});
                Zombie::zombies.push_back(zombie);
                field->addPrintPawn(zombie);
            }
        }
        // debug << "\tAfter zombies spawning" << std::endl;
        if (hardcore) {
            // The point of the game is to survive as long as possible, so the spawning rate of the enemies increases over time
            // The hordes of enemies are spawned every 500 frames, but the number of enemies in each horde increases over time
            if (i % 500 == 250) {
                for (unsigned short j=0; j<i/100; j++) {
                    unsigned short y = rand() % 20;
                    if (Queen::queen->getCoordinates().y != y) {
                        Walker* walker = new Walker({y, 49});
                        Walker::walkers.push_back(walker);
                        field->addPrintPawn(walker);
                    }
                }
                for (unsigned short j=0; j<i/200; j++) {
                    unsigned short y = rand() % 20;
                    if (Queen::queen->getCoordinates().y != y) {
                        Zombie* zombie = new Zombie({y, 49});
                        Zombie::zombies.push_back(zombie);
                        field->addPrintPawn(zombie);
                    }
                }
            }
            // Too many zombies increase the probability of segfaults, so every REPOPULATE frames we empty and then repopulate the field
            if (i % REPOPULATE == REPOPULATE - 1) {
                field->clear();
                field->addPrintPawn(Player::player);
                field->addPrintPawn(Queen::queen);
                for (auto wall : Wall::walls) {
                    field->addPrintPawn(wall);
                }
                for (auto zombie : Zombie::zombies) {
                    field->addPrintPawn(zombie);
                }
                for (auto walker : Walker::walkers) {
                    field->addPrintPawn(walker);
                }
                for (auto mine : Mine::mines) {
                    field->addPrintPawn(mine);
                }
                for (auto cannon : Cannon::cannons) {
                    field->addPrintPawn(cannon);
                }
                for (auto worker : Worker::workers) {
                    field->addPrintPawn(worker);
                }
                for (auto worker : ArmedWorker::armedWorkers) {
                    field->addPrintPawn(worker);
                }
                for (auto bomber : Bomber::bombers) {
                    field->addPrintPawn(bomber);
                }
            }
        }
        if (endless) {
            // The game is endless, so the queen regenerates life
            Queen::queen->life = 9;
        }
        Queen::queen->setSymbol('0' + Queen::queen->life);
        field->rePrintPawn(Queen::queen);
        if (i % 10 == 0) {
            sista::clearScreen();
            field->print(border);
        }
        // Statistics
        Queen::queenStyle.apply();
        cursor.set(8, 55);
        std::cout << "Frame elapsed: " << i << " ";
        cursor.set(10, 55);
        std::cout << "Ammonitions: " << Player::player->ammonitions << "    ";
        cursor.set(12, 55);
        std::cout << "Life: " << Queen::queen->life;
        if (!unofficial) {
            cursor.set(14, 55);
            std::cout << START_AMMONITION; // The official run should show the starting ammonition
        }
        std::cout << std::flush;

        // At the end of the frame we check if in the Field there is any Entity which isn't in any of the lists
        std::vector<sista::Coordinates> coordinates;
        for (unsigned short j=0; j<20; j++) {
            for (unsigned short i=0; i<50; i++) {
                Entity* pawn = (Entity*)field->getPawn(j, i);
                if (pawn == nullptr) continue;
                if (std::find(Bullet::bullets.begin(), Bullet::bullets.end(), pawn) == Bullet::bullets.end() &&
                    std::find(EnemyBullet::enemyBullets.begin(), EnemyBullet::enemyBullets.end(), pawn) == EnemyBullet::enemyBullets.end() &&
                    std::find(Zombie::zombies.begin(), Zombie::zombies.end(), pawn) == Zombie::zombies.end() &&
                    std::find(Walker::walkers.begin(), Walker::walkers.end(), pawn) == Walker::walkers.end() &&
                    std::find(Wall::walls.begin(), Wall::walls.end(), pawn) == Wall::walls.end() &&
                    std::find(Mine::mines.begin(), Mine::mines.end(), pawn) == Mine::mines.end() &&
                    std::find(Cannon::cannons.begin(), Cannon::cannons.end(), pawn) == Cannon::cannons.end() &&
                    std::find(Worker::workers.begin(), Worker::workers.end(), pawn) == Worker::workers.end() &&
                    std::find(ArmedWorker::armedWorkers.begin(), ArmedWorker::armedWorkers.end(), pawn) == ArmedWorker::armedWorkers.end() &&
                    std::find(Bomber::bombers.begin(), Bomber::bombers.end(), pawn) == Bomber::bombers.end() &&
                    pawn != Player::player && pawn != Queen::queen) {
                    coordinates.push_back(pawn->getCoordinates());
                    #if DEBUG
                    debug << "Erasing " << pawn << " at " << pawn->getCoordinates() << std::endl;
                    debug << "\t" << typeid(*pawn).name() << std::endl;
                    #endif
                }
            }
        }
        // debug << "\tBefore erasing nullptrs" << std::endl;
        for (auto coord : coordinates) {
            field->erasePawn(coord);
        }
        // debug << "\tAfter erasing nullptrs" << std::endl;
    }
    if (music) {
        music_th.join();
    }
    th.join();
    flushInput();
    cursor.set(52, 0); // Move the cursor to the bottom of the screen, so the terminal is not left in a weird state
    #ifdef __APPLE__
    tcsetattr(0, TCSANOW, &orig_termios);
    #endif
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}

void printIntro() {
    std::cout << CLS; // Clear screen
    std::cout << SSB; // Clear scrollback buffer
    // if linux
    #if __linux__ or __APPLE__
        std::cout << "\x1b]2;de dödas angrepp\x07"; // Set window title
        std::cout << "\t\t\t\t\t\x1b[3m  Dödas\x1b[0m\n\t\t\t\t\x1b[3mYou better not fuck with \x1b[34;40mB\x1b[0m\n\n";
        std::cout << "\t\t\t  \x1b[31;1mDödas v" << VERSION << "\t\tFLAK-ZOSO " << DATE << "\x1b[0m\n\n";
    #elif defined(_WIN32)
        std::cout << "\x1b]2;de dodas angrepp\x07"; // Set window title
        std::cout << "\t\t\t\t\t\x1b[3m  Dodas\x1b[0m\n\t\t\t\t\x1b[3mYou better not fuck with \x1b[34;40mB\x1b[0m\n\n";
        std::cout << "\t\t\t  \x1b[31;1mDodas v" << VERSION << "\t\tFLAK-ZOSO " << DATE << "\x1b[0m\n\n";
    #endif

    std::cout << "\t\thttps://github.com/Lioydiano/Dodas?tab=readme-ov-file#how-to-play\n\n";
    std::cout << "\t\t\t\t- '\x1b[35mQ\x1b[0m' to \x1b[3mquit\x1b[0m\n";
    std::cout << "\t\t\t\t- '\x1b[35m.\x1b[0m' to \x1b[3mpause\x1b[0m and \x1b[3mresume\x1b[0m\n";
    std::cout << "\t\t\t\t- '\x1b[35mw\x1b[0m | \x1b[35ma\x1b[0m | \x1b[35ms\x1b[0m | \x1b[35md\x1b[0m' to step\n";
    std::cout << "\t\t\t\t- '\x1b[35mi\x1b[0m | \x1b[35mj\x1b[0m | \x1b[35mk\x1b[0m | \x1b[35ml\x1b[0m' to fire a bullet\n";
    std::cout << "\t\t\t\t- '\x1b[35mp\x1b[0m' to select bullets\n";
    std::cout << "\t\t\t\t- '\x1b[35mm\x1b[0m' to select mines\n";
    std::cout << "\t\t\t\t- '\x1b[35mc\x1b[0m' to select cannons\n";
    std::cout << "\t\t\t\t- '\x1b[35mW\x1b[0m' to select workers\n";
    std::cout << "\t\t\t\t- '\x1b[35mU\x1b[0m' to select armed workers\n";
    std::cout << "\t\t\t\t- '\x1b[35m=\x1b[0m' or '\x1b[35m0\x1b[0m' to select walls\n";
    std::cout << "\t\t\t\t- '\x1b[35mb\x1b[0m' to select bombers\n\n";

    std::cout << "\t\t\t\tDefeat the \x1b[31mqueen\x1b[0m to win\x1b[0m\n\n";
    std::cout << "\t\t\t\t\x1b[3mPress any key to start\x1b[0m";
    std::flush(std::cout);
    #if defined(_WIN32) or defined(__linux__)
        getch();
    #elif __APPLE__
        getchar();
    #endif
    sista::clearScreen();
}


std::unordered_map<Direction, sista::Coordinates> directionMap = {
    {Direction::UP, {(unsigned short)-1, 0}},
    {Direction::RIGHT, {0, 1}},
    {Direction::DOWN, {1, 0}},
    {Direction::LEFT, {0, (unsigned short)-1}}
};
std::unordered_map<Direction, char> directionSymbol = {
    {Direction::UP, '^'},
    {Direction::RIGHT, '>'},
    {Direction::DOWN, 'v'},
    {Direction::LEFT, '<'}
};
std::mt19937 rng(std::chrono::system_clock::now().time_since_epoch().count());

Entity::Entity(char symbol, sista::Coordinates coordinates, ANSI::Settings& settings, Type type) : sista::Pawn(symbol, coordinates, settings), type(type) {}
Entity::Entity() : sista::Pawn(' ', sista::Coordinates(0, 0), Wall::wallStyle), type(Type::PLAYER) {}

ANSI::Settings Bullet::bulletStyle = {
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
void Bullet::removeBullet(Bullet* bullet) {
    Bullet::bullets.erase(std::find(Bullet::bullets.begin(), Bullet::bullets.end(), bullet));
    field->erasePawn(bullet);
    delete bullet;
}
Bullet::Bullet() : Entity(' ', {0, 0}, bulletStyle, Type::BULLET), direction(Direction::RIGHT), speed(1) {}
Bullet::Bullet(sista::Coordinates coordinates, Direction direction) : Entity(directionSymbol[direction], coordinates, bulletStyle, Type::BULLET), direction(direction), speed(1) {}
Bullet::Bullet(sista::Coordinates coordinates, Direction direction, unsigned short speed) : Entity(directionSymbol[direction], coordinates, bulletStyle, Type::BULLET), direction(direction), speed(speed) {}
void Bullet::move() {
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction]*speed;
    if (field->isOutOfBounds(nextCoordinates)) {
        // debug << "Out of bounds" << std::endl;
        Bullet::removeBullet(this);
        // debug << "After remove" << std::endl;
        return;
    } else if (field->isFree(nextCoordinates)) {
        // debug << "Free" << std::endl;
        field->movePawn(this, nextCoordinates);
        // debug << "After move" << std::endl;
        coordinates = nextCoordinates;
        return;
    } else { // Something was hitten
        Entity* hitten = (Entity*)field->getPawn(nextCoordinates);
        // debug << "Hitten " << hitten << std::endl;
        if (hitten->type == Type::WALL) {
            // debug << "\tWall" << std::endl;
            Wall* wall = (Wall*)hitten;
            wall->strength--;
            // debug << "\tWall's strength: " << wall->strength << std::endl;
            if (wall->strength == 0) {
                wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
            }
        } else if (hitten->type == Type::ZOMBIE) {
            // debug << "\tZombie" << std::endl;
            Zombie::removeZombie((Zombie*)hitten);
            // debug << "\tZombie removed" << std::endl;
        } else if (hitten->type == Type::WALKER) {
            // debug << "\tWalker" << std::endl;
            Walker::removeWalker((Walker*)hitten);
            // debug << "\tWalker removed" << std::endl;
        } else if (hitten->type == Type::BULLET) {
            // debug << "\tBullet" << std::endl;
            ((Bullet*)hitten)->collided = true;
            // debug << "\tBullet collided" << std::endl;
            // Bullet::removeBullet((Bullet*)hitten);
            return;
        } else if (hitten->type == Type::ENEMYBULLET) {
            // When two bullets collide, their "collided" attribute is set to true
            // debug << "\tEnemy bullet" << std::endl;
            ((EnemyBullet*)hitten)->collided = true;
            // debug << "\tEnemy bullet collided" << std::endl;
            collided = true;
            // debug << "\tBullet collided" << std::endl;
            return;
        } else if (hitten->type == Type::MINE) {
            // debug << "\tMine" << std::endl;
            Mine* mine = (Mine*)hitten;
            mine->triggered = true;
            // debug << "\tMine triggered" << std::endl;
        } else if (hitten->type == Type::CANNON) {
            // debug << "\tCannon" << std::endl;
            Cannon* cannon = (Cannon*)hitten;
            // Makes the cannon fire
            // debug << "\tCannon firing" << std::endl;
            cannon->fire();
            // debug << "\tCannon fired" << std::endl;
        } else if (hitten->type == Type::QUEEN) {
            // debug << "\tQueen" << std::endl;
            Queen* mother = (Queen*)hitten;
            mother->life--;
            // debug << "\tQueen's life: " << mother->life << std::endl;
            field->rePrintPawn(mother);
            mother->createWall();
            // debug << "\tQueen's wall created" << std::endl;
            if (mother->life == 0) {
                // win();
                end = true;
            }
        }
        // debug << "\tAfter collision" << std::endl;
        Bullet::removeBullet(this);
        // debug << "\tAfter remove" << std::endl;
    }
}


ANSI::Settings EnemyBullet::enemyBulletStyle = {
    ANSI::ForegroundColor::F_GREEN,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
EnemyBullet::EnemyBullet(sista::Coordinates coordinates, Direction direction, unsigned short speed) : Entity(directionSymbol[direction], coordinates, enemyBulletStyle, Type::ENEMYBULLET), direction(direction), speed(speed) {}
EnemyBullet::EnemyBullet(sista::Coordinates coordinates, Direction direction) : Entity(directionSymbol[direction], coordinates, enemyBulletStyle, Type::ENEMYBULLET), direction(direction), speed(1) {}
EnemyBullet::EnemyBullet() : Entity(' ', {0, 0}, enemyBulletStyle, Type::ENEMYBULLET), direction(Direction::UP), speed(1) {}
void EnemyBullet::removeEnemyBullet(EnemyBullet* enemyBullet) {
    EnemyBullet::enemyBullets.erase(std::find(EnemyBullet::enemyBullets.begin(), EnemyBullet::enemyBullets.end(), enemyBullet));
    field->erasePawn(enemyBullet);
    delete enemyBullet;
}
void EnemyBullet::move() { // Pretty sure there's a segfault here
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction]*speed;
    if (field->isOutOfBounds(nextCoordinates)) {
        EnemyBullet::removeEnemyBullet(this);
        return;
    } else if (field->isFree(nextCoordinates)) {
        field->movePawn(this, nextCoordinates);
        coordinates = nextCoordinates;
        return;
    } else { // Something was hitten
        Entity* hitten = (Entity*)field->getPawn(nextCoordinates);
        if (hitten->type == Type::PLAYER) {
            // lose();
            end = true;
        } else if (hitten->type == Type::WALL) {
            Wall* wall = (Wall*)hitten;
            wall->strength--;
            if (wall->strength == 0) {
                wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
            }
        } else if (hitten->type == Type::BULLET) {
            ((Bullet*)hitten)->collided = true;
            collided = true;
            return;
        } else if (hitten->type == Type::ZOMBIE || hitten->type == Type::WALKER) {
            // No friendly fire
        } if (hitten->type == Type::ENEMYBULLET) {
            collided = true;
            return;
            // EnemyBullet::removeEnemyBullet((EnemyBullet*)hitten);
        } else if (hitten->type == Type::MINE) {
            Mine* mine = (Mine*)hitten;
            mine->triggered = true;
        } else if (hitten->type == Type::CANNON) { // The cannon is destroyed by the enemy bullet
            Cannon::removeCannon((Cannon*)hitten);
        } else if (hitten->type == Type::WORKER) {
            Worker::removeWorker((Worker*)hitten);
        } else if (hitten->type == Type::ARMED_WORKER) {
            ArmedWorker::removeArmedWorker((ArmedWorker*)hitten);
        } else if (hitten->type == Type::BOMBER) {
            Bomber::removeBomber((Bomber*)hitten);
        }
        EnemyBullet::removeEnemyBullet(this);
    }
}

ANSI::Settings Player::playerStyle = {
    ANSI::ForegroundColor::F_RED,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
Player::Player(sista::Coordinates coordinates) : Entity('$', coordinates, playerStyle, Type::PLAYER), weapon(Type::BULLET), ammonitions(START_AMMONITION) {}
Player::Player() : Entity('$', {0, 0}, playerStyle, Type::PLAYER), weapon(Type::BULLET), ammonitions(START_AMMONITION) {}
void Player::move(Direction direction) {
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction];
    if (field->isOutOfBounds(nextCoordinates) || !field->isFree(nextCoordinates) || nextCoordinates.x >= 30) {
        return; // No complications, if you can't move there just pretend the command was never given
    }
    field->movePawn(this, nextCoordinates);
    coordinates = nextCoordinates;
}
void Player::shoot(Direction direction) {
    sista::Coordinates spawn = this->coordinates + directionMap[direction];
    if (!field->isFree(spawn)) {
        return; // No complications, if you can't spawn something there just pretend the command was never given
    }
    if (Player::player->ammonitions <= 0) {
        std::cout << "\7";
        return; // No complications, if you can't spawn something there just pretend the command was never given
    }
    switch (weapon) {
    case Type::BULLET: {
        Player::player->ammonitions--;
        Bullet* newbullet = new Bullet(spawn, direction);
        Bullet::bullets.push_back(newbullet);
        field->addPrintPawn(newbullet);
        break;
    }
    case Type::MINE: {
        if (Player::player->ammonitions < 3)
            return;
        Player::player->ammonitions -= 3;
        Mine* newmine = new Mine(spawn);
        Mine::mines.push_back(newmine);
        field->addPrintPawn(newmine);
        break;
    }
    case Type::CANNON: {
        if (Player::player->ammonitions < 5)
            return;
        Player::player->ammonitions -= 5;
        Cannon* newcannon = new Cannon(spawn, CANNON_FIRE_PERIOD);
        Cannon::cannons.push_back(newcannon);
        field->addPrintPawn(newcannon);
        break;
    }
    case Type::BOMBER: {
        if (Player::player->ammonitions < 7)
            return;
        Player::player->ammonitions -= 7;
        Bomber* newbomber = new Bomber(spawn);
        Bomber::bombers.push_back(newbomber);
        field->addPrintPawn(newbomber);
        break;
    }
    case Type::WORKER: {
        if (Player::player->ammonitions < 5)
            return;
        Player::player->ammonitions -= 5;
        Worker* newworker = new Worker(spawn, WORKER_PRODUCTION_PERIOD);
        Worker::workers.push_back(newworker);
        field->addPrintPawn(newworker);
        break;
    }
    case Type::ARMED_WORKER: {
        if (Player::player->ammonitions < 8)
            return;
        Player::player->ammonitions -= 8;
        ArmedWorker* newworker = new ArmedWorker(spawn, WORKER_PRODUCTION_PERIOD);
        ArmedWorker::armedWorkers.push_back(newworker);
        field->addPrintPawn(newworker);
        break;
    }
    case Type::WALL: {
        if (Player::player->ammonitions < 1)
            return;
        Player::player->ammonitions -= 1;
        Wall* newwall = new Wall(spawn, 2);
        Wall::walls.push_back(newwall);
        field->addPrintPawn(newwall);
        break;
    }
    default:
        break;
    }
}

ANSI::Settings Zombie::zombieStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_GREEN,
    ANSI::Attribute::FAINT
};
void Zombie::removeZombie(Zombie* zombie) {
    Zombie::zombies.erase(std::find(Zombie::zombies.begin(), Zombie::zombies.end(), zombie));
    field->erasePawn(zombie);
    delete zombie;
}
Zombie::Zombie(sista::Coordinates coordinates) : Entity('Z', coordinates, zombieStyle, Type::ZOMBIE) {}
Zombie::Zombie() : Entity('Z', {0, 0}, zombieStyle, Type::ZOMBIE) {}
void Zombie::move() { // Zombies mostly move vertically and stay defending the mother
    sista::Coordinates nextCoordinates;
    // The zombie may move towards the player if it's in the same row, but it may also move the other way
    if (Player::player->getCoordinates().y == coordinates.y) {
        // Player.x is always < Zombie.x, so no need to check that
        nextCoordinates = coordinates + directionMap[Direction::LEFT];
        // If the Zombie is too left, it will move right
        if (coordinates.x < 30) {
            nextCoordinates = coordinates + directionMap[Direction::RIGHT];
        }
    } else {
        if (rand() % 2 == 0) {
            nextCoordinates = coordinates + directionMap[Direction::DOWN];
        } else {
            nextCoordinates = coordinates + directionMap[Direction::UP];
        }
    }
    if (field->isFree(nextCoordinates)) {
        field->movePawn(this, nextCoordinates);
        coordinates = nextCoordinates;
    }
}
void Zombie::shoot() {
    sista::Coordinates spawn = coordinates + directionMap[Direction::LEFT];
    if (!field->isFree(spawn)) {
        return; // No complications, if you can't spawn something there just pretend the command was never given
    }
    EnemyBullet* newbullet = new EnemyBullet(spawn, Direction::LEFT);
    EnemyBullet::enemyBullets.push_back(newbullet);
    field->addPrintPawn(newbullet);
}

ANSI::Settings Queen::queenStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_RED,
    ANSI::Attribute::BRIGHT
};
Queen::Queen(sista::Coordinates coordinates) : Entity('9', coordinates, queenStyle, Type::QUEEN), life(9) {}
Queen::Queen() : Entity('9', {0, 0}, queenStyle, Type::QUEEN), life(9) {}
void Queen::move() {
    // Queen's movement is only vertical and it is always near the center of its side {10, 49}
    if (rand() % 10 == 0) {
        if (coordinates.y < 6) return;
        sista::Coordinates nextCoordinates = coordinates + directionMap[Direction::UP];
        if (field->isFree(nextCoordinates)) {
            field->movePawn(this, nextCoordinates);
            coordinates = nextCoordinates;
        }
    } else if (rand() % 10 == 1) {
        if (coordinates.y > 14) return;
        sista::Coordinates nextCoordinates = coordinates + directionMap[Direction::DOWN];
        if (field->isFree(nextCoordinates)) {
            field->movePawn(this, nextCoordinates);
            coordinates = nextCoordinates;
        }
    }
}
void Queen::createWall() {
    // First determine the length of the wall
    unsigned short length = rand() % 3 + 3; // in range [3, 5]
    // Then determine the position of the wall (the center of the wall is on the y coordinate of the queen)
    unsigned short y = coordinates.y;
    // Then search for an x coordinate which is free
    unsigned short x = 48;
    for (; x >= 30; x--) {
        // We need to check all the cells in range {[y-length/2, y+1+length/2], x}
        bool free = true;
        for (unsigned short j=y-length/2; j<=y+1+length/2; j++) {
            if (!field->isFree(j, x)) {
                free = false;
                break;
            }
        }
        if (free) break;
    }
    if (x <= 30) return; // No free space to create the wall
    // Now we can create the wall
    for (unsigned short j=y-length/2; j<=y+1+length/2; j++) {
        Wall* wall = new Wall({j, x}, 1);
        Wall::walls.push_back(wall);
        field->addPrintPawn(wall);
    }
}

ANSI::Settings Wall::wallStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
void Wall::removeWall(Wall* wall) {
    Wall::walls.erase(std::find(Wall::walls.begin(), Wall::walls.end(), wall));
    field->erasePawn(wall);
    delete wall;
}
Wall::Wall(sista::Coordinates coordinates, short int strength) : Entity('=', coordinates, wallStyle, Type::WALL), strength(strength) {}
Wall::Wall() : Entity('=', {0, 0}, wallStyle, Type::WALL), strength(3) {}

ANSI::Settings Mine::mineStyle = {
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BLINK   
};
void Mine::removeMine(Mine* mine) {
    Mine::mines.erase(std::find(Mine::mines.begin(), Mine::mines.end(), mine));
    field->erasePawn(mine);
    delete mine;
}
Mine::Mine(sista::Coordinates coordinates) : Entity('*', coordinates, mineStyle, Type::MINE), triggered(false) {}
Mine::Mine() : Entity('*', {0, 0}, mineStyle, Type::MINE), triggered(false) {}
bool Mine::checkTrigger() {
    for (int j=-1; j<=1; j++) {
        for (int i=-1; i<=1; i++) {
            if (i == 0 && j == 0) continue;
            sista::Coordinates nextCoordinates = coordinates + sista::Coordinates(j, i);
            if (field->isOutOfBounds(nextCoordinates)) continue;
            Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
            if (neighbor == nullptr) {
                continue;
            } else if (neighbor->type == Type::ZOMBIE || neighbor->type == Type::WALKER) {
                trigger();
                return true;
            }
        }
    }
    return false;
}
void Mine::trigger() {
    triggered = true;
    symbol = '%';
    settings.foregroundColor = ANSI::ForegroundColor::F_WHITE;
}
void Mine::explode() {
    for (int j=-2; j<=2; j++) {
        for (int i=-2; i<=2; i++) {
            if (i == 0 && j == 0) continue;
            sista::Coordinates nextCoordinates = coordinates + sista::Coordinates(j, i);
            if (field->isOutOfBounds(nextCoordinates)) continue;
            Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
            if (neighbor == nullptr) {
                continue;
            } else if (neighbor->type == Type::ZOMBIE) {
                Zombie::removeZombie((Zombie*)neighbor);
            } else if (neighbor->type == Type::WALKER) {
                Walker::removeWalker((Walker*)neighbor);
            } else if (neighbor->type == Type::ENEMYBULLET) {
                EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
            } else if (neighbor->type == Type::MINE) {
                Mine* mine = (Mine*)neighbor;
                mine->triggered = true;
            } else if (neighbor->type == Type::CANNON) {
                Cannon::removeCannon((Cannon*)neighbor);
            } else if (neighbor->type == Type::QUEEN) {
                Queen* mother = (Queen*)neighbor;
                mother->life--;
                field->rePrintPawn(mother);
                mother->createWall();
                if (mother->life == 0) {
                    // win();
                    end = true;
                }
            } else if (neighbor->type == Type::WALL) {
                Wall* wall = (Wall*)neighbor;
                int damage = rand() % 3 + 1;
                if (wall->strength <= damage) {
                    wall->strength = 0;
                    wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                    field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
                } else {
                    wall->strength -= damage;
                }
            }
        }
    }
}

ANSI::Settings Cannon::cannonStyle = {
    ANSI::ForegroundColor::F_RED,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
std::bernoulli_distribution Cannon::distribution(1.0/CANNON_FIRE_PERIOD);
void Cannon::removeCannon(Cannon* cannon) {
    Cannon::cannons.erase(std::find(Cannon::cannons.begin(), Cannon::cannons.end(), cannon));
    field->erasePawn(cannon);
    delete cannon;
}
Cannon::Cannon(sista::Coordinates coordinates, unsigned short period) : Entity('C', coordinates, cannonStyle, Type::CANNON) {}
Cannon::Cannon() : Entity('C', {0, 0}, cannonStyle, Type::CANNON) {}
void Cannon::fire() {
    sista::Coordinates spawn = coordinates + directionMap[Direction::RIGHT];
    if (!field->isFree(spawn)) {
        return; // No complications, if you can't spawn something there just pretend the command was never given
    }
    if (Player::player->ammonitions <= 0) {
        return; // No complications, if you can't spawn something there just pretend the command was never given
    }
    Player::player->ammonitions--;
    Bullet* newbullet = new Bullet(spawn, Direction::RIGHT);
    Bullet::bullets.push_back(newbullet);
    field->addPrintPawn(newbullet);
}
void Cannon::recomputeDistribution(std::vector<std::vector<unsigned short>>& workersPositions) {
    // Count the consecutive workers in the same row right back to the cannon
    unsigned short count = 0;
    for (unsigned short i=coordinates.x-1; i>=0; i--) {
        if (std::find(workersPositions[coordinates.y].begin(), workersPositions[coordinates.y].end(), i) != workersPositions[coordinates.y].end()) {
            count++;
        } else {
            break; // If there's no worker in the position, then there's no need to keep counting
        }
    }
    distribution = std::bernoulli_distribution(1.0/((float)CANNON_FIRE_PERIOD - std::min(1.4*count, 39.0)));
}

ANSI::Settings Worker::workerStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::UNDERSCORE
};
void Worker::removeWorker(Worker* worker) {
    Worker::workers.erase(std::find(Worker::workers.begin(), Worker::workers.end(), worker));
    field->erasePawn(worker);
    delete worker;
}
Worker::Worker(sista::Coordinates coordinates, unsigned short productionRate) : Entity('W', coordinates, workerStyle, Type::WORKER), distribution(std::bernoulli_distribution(1.0/productionRate)) {}
Worker::Worker(sista::Coordinates coordinates) : Entity('W', coordinates, workerStyle, Type::WORKER), distribution(std::bernoulli_distribution(1.0/WORKER_PRODUCTION_PERIOD)) {}
Worker::Worker() : Entity('W', {0, 0}, workerStyle, Type::WORKER), distribution(std::bernoulli_distribution(1.0/WORKER_PRODUCTION_PERIOD)) {}
void Worker::produce() {
    Player::player->ammonitions++;
}

ANSI::Settings ArmedWorker::armedWorkerStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLUE,
    ANSI::Attribute::UNDERSCORE
};
void ArmedWorker::removeArmedWorker(ArmedWorker* worker) {
    ArmedWorker::armedWorkers.erase(std::find(ArmedWorker::armedWorkers.begin(), ArmedWorker::armedWorkers.end(), worker));
    field->erasePawn(worker);
    delete worker;
}
ArmedWorker::ArmedWorker(sista::Coordinates coordinates, unsigned short productionRate) : Entity('W', coordinates, armedWorkerStyle, Type::ARMED_WORKER), distribution(std::bernoulli_distribution(1.0/productionRate)) {}
ArmedWorker::ArmedWorker(sista::Coordinates coordinates) : Entity('W', coordinates, armedWorkerStyle, Type::ARMED_WORKER), distribution(std::bernoulli_distribution(1.0/WORKER_PRODUCTION_PERIOD)) {}
ArmedWorker::ArmedWorker() : Entity('W', {0, 0}, armedWorkerStyle, Type::ARMED_WORKER), distribution(std::bernoulli_distribution(1.0/WORKER_PRODUCTION_PERIOD)) {}
void ArmedWorker::produce() {
    Player::player->ammonitions++;
}
void ArmedWorker::dodgeIfNeeded() {
    sista::Coordinates target = this->coordinates + directionMap[Direction::RIGHT] * 3;
    if (field->isOutOfBounds(target))
        return;
    if (field->isOccupied(target)) {
        if (((Entity*)field->getPawn(target))->type == Type::ENEMYBULLET) {
            Wall* newwall = new Wall(this->coordinates + directionMap[Direction::RIGHT], 2);
            Wall::walls.push_back(newwall);
            field->addPrintPawn(newwall);

            sista::Coordinates destination = this->coordinates + directionMap[Direction::UP];
            Direction moved = Direction::UP;
            if (field->isFree(destination)) {
                field->movePawn(this, destination);
                std::cout << std::flush;
            } else {
                destination = this->coordinates + directionMap[Direction::DOWN];
                moved = Direction::DOWN;
                if (field->isFree(destination)) {
                    field->movePawn(this, destination);
                    std::cout << std::flush;
                } else {
                    return;
                }
            }
            
            Bullet* newbullet = new Bullet(this->coordinates + directionMap[Direction::RIGHT], Direction::RIGHT);
            Bullet::bullets.push_back(newbullet);
            field->addPrintPawn(newbullet);

            destination = this->coordinates + directionMap[moved == Direction::UP ? Direction::DOWN : Direction::UP];
            if (field->isFree(destination)) {
                field->movePawn(this, destination);
            }
        }
    }
}

ANSI::Settings Bomber::bomberStyle = {
    ANSI::ForegroundColor::F_BLUE,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
void Bomber::removeBomber(Bomber* bomber) {
    Bomber::bombers.erase(std::find(Bomber::bombers.begin(), Bomber::bombers.end(), bomber));
    field->erasePawn(bomber);
    delete bomber;
}
Bomber::Bomber(sista::Coordinates coordinates) : Entity('B', coordinates, bomberStyle, Type::BOMBER) {}
Bomber::Bomber() : Entity('B', {0, 0}, bomberStyle, Type::BOMBER) {}
void Bomber::move() {
    sista::Coordinates nextCoordinates = coordinates + directionMap[Direction::RIGHT];
    if (field->isOutOfBounds(nextCoordinates)) {
        if (coordinates.x == 49) {
            this->explode();
        }
        Bomber::removeBomber(this);
        return;
    }
    Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
    if (neighbor == nullptr) {
        field->movePawn((Pawn*)this, nextCoordinates);
        coordinates = nextCoordinates;
        return;
    } else if (neighbor->type == Type::WALL) {
        Wall* wall = (Wall*)neighbor;
        wall->strength = 0;
        wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
        field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
        explode();
    } else if (neighbor->type == Type::PLAYER) {
        // lose();
        end = true;
    } else if (neighbor->type == Type::BULLET) {
        ((Bullet*)neighbor)->collided = true;
    } else if (neighbor->type == Type::ENEMYBULLET) {
        ((EnemyBullet*)neighbor)->collided = true;
    } else if (neighbor->type == Type::MINE) {
        Mine* mine = (Mine*)neighbor;
        mine->triggered = true;
    } else if (neighbor->type == Type::WALKER || neighbor->type == Type::ZOMBIE) {
        explode();
    } else if (neighbor->type == Type::QUEEN) {
        Queen* mother = (Queen*)neighbor;
        mother->life--;
        field->rePrintPawn(mother);
        mother->createWall();
        if (mother->life == 0) {
            // win();
            end = true;
        }
    } else if (neighbor->type == Type::BOMBER) {
        return;
    }
    Bomber::removeBomber(this);
}
void Bomber::explode() {
    for (int j=-2; j<=2; j++) {
        for (int i=-2; i<=2; i++) {
            if (i == 0 && j == 0) continue;
            sista::Coordinates nextCoordinates = coordinates + sista::Coordinates(j, i);
            if (field->isOutOfBounds(nextCoordinates)) {
                continue;
            }
            Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
            if (neighbor == nullptr) {
                continue;
            } else if (neighbor->type == Type::ZOMBIE) {
                Zombie::removeZombie((Zombie*)neighbor);
            } else if (neighbor->type == Type::WALKER) {
                Walker::removeWalker((Walker*)neighbor);
            } else if (neighbor->type == Type::MINE) {
                Mine* mine = (Mine*)neighbor;
                mine->triggered = true;
            } else if (neighbor->type == Type::CANNON) {
                Cannon::removeCannon((Cannon*)neighbor);
            } else if (neighbor->type == Type::QUEEN) {
                Queen* mother = (Queen*)neighbor;
                mother->life--;
                field->rePrintPawn(mother);
                mother->createWall();
                if (mother->life == 0) {
                    // win();
                    end = true;
                }
            } else if (neighbor->type == Type::WALL) {
                Wall* wall = (Wall*)neighbor;
                int damage = rand() % 3 + 1;
                if (wall->strength <= damage) {
                    wall->strength = 0;
                    wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                    field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
                } else {
                    wall->strength -= damage;
                }
            } else if (neighbor->type == Type::BOMBER) {
                continue;
            }
        }
    }
}

ANSI::Settings Walker::walkerStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_GREEN,
    ANSI::Attribute::BRIGHT
};
void Walker::removeWalker(Walker* walker) {
    Walker::walkers.erase(std::find(Walker::walkers.begin(), Walker::walkers.end(), walker));
    field->erasePawn(walker);
    delete walker;
}
Walker::Walker(sista::Coordinates coordinates) : Entity('Z', coordinates, walkerStyle, Type::WALKER) {}
Walker::Walker() : Entity('Z', {0, 0}, walkerStyle, Type::WALKER) {}
void Walker::move() { // Walkers mostly move horizontally because they only rarely shoot bullets and they walk slowly towards the left side
    Direction direction_ = (rand() % 30 ? Direction::LEFT : Direction::DOWN);
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction_];
    if (field->isOutOfBounds(nextCoordinates)) {
        if (coordinates.x == 0) { // Touchdown, the player loses all the ammonitions
            Player::player->ammonitions = 0;
            this->explode();
            Walker::removeWalker(this);
            return;
        } else { // Touched bottom limit, we can use pacman effect which clearly can be used by walkers
            try {
                field->movePawnBy(this, directionMap[Direction::DOWN], PACMAN_EFFECT);
            } catch (const std::exception& e) {
                // Nothing happens, but trying to apply manually the pacman effect and hitting something on the other side would be awkward
            }
            return;
        }
    }
    Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
    if (neighbor == nullptr) {
        field->movePawn((Pawn*)this, nextCoordinates);
        coordinates = nextCoordinates;
    } else if (neighbor->type == Type::PLAYER) {
        // lose();
        end = true;
    } else if (neighbor->type == Type::BULLET) {
        Bullet::removeBullet((Bullet*)neighbor);
        Walker::removeWalker(this);
    } else if (neighbor->type == Type::WALL) {
        Wall* wall = (Wall*)neighbor;
        if (wall->strength == 0) {
            return;
        }
        wall->strength--;
        if (wall->strength == 0) {
            wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
            field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
        }
    } else if (neighbor->type == Type::MINE) {
        Mine* mine = (Mine*)neighbor;
        mine->triggered = true;
    } else if (neighbor->type == Type::CANNON) {
        Cannon::removeCannon((Cannon*)neighbor);
    } else if (neighbor->type == Type::WORKER) {
        Worker::removeWorker((Worker*)neighbor);
    } else if (neighbor->type == Type::ARMED_WORKER) {
        ArmedWorker::removeArmedWorker((ArmedWorker*)neighbor);
    }
}
void Walker::explode() {
    for (int j=-1; j<=1; j++) {
        for (int i=-1; i<=1; i++) {
            if (i == 0 && j == 0) continue;
            sista::Coordinates nextCoordinates = coordinates + sista::Coordinates(j, i);
            if (field->isOutOfBounds(nextCoordinates)) {
                continue;
            }
            Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
            if (neighbor == nullptr) {
                continue;
            } else if (neighbor->type == Type::ZOMBIE) {
                Zombie::removeZombie((Zombie*)neighbor);
            } else if (neighbor->type == Type::WALKER) {
                Walker::removeWalker((Walker*)neighbor);
            } else if (neighbor->type == Type::ENEMYBULLET) {
                EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
            } else if (neighbor->type == Type::MINE) {
                Mine* mine = (Mine*)neighbor;
                mine->triggered = true;
            } else if (neighbor->type == Type::CANNON) {
                Cannon::removeCannon((Cannon*)neighbor);
            } else if (neighbor->type == Type::WALL) {
                Wall* wall = (Wall*)neighbor;
                int damage = rand() % 3 + 1;
                if (wall->strength <= damage) {
                    wall->strength = 0;
                    wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                    field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
                } else {
                    wall->strength -= damage;
                }
            } else if (neighbor->type == Type::WORKER) {
                Worker::removeWorker((Worker*)neighbor);
            } else if (neighbor->type == Type::ARMED_WORKER) {
                ArmedWorker::removeArmedWorker((ArmedWorker*)neighbor);
            }
        }
    }
}

void removeNullptrs(std::vector<Entity*>& entities) {
    for (unsigned i=0; i<entities.size(); i++) {
        if (entities[i] == nullptr) {
            entities.erase(entities.begin() + i);
            i--;
        }
    }
}
