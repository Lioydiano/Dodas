#include "cross_platform.hpp"
#include "dodas.hpp"
#include <thread>
#include <chrono>


sista::SwappableField* field;

std::vector<Bullet*> Bullet::bullets;
std::vector<EnemyBullet*> EnemyBullet::enemyBullets;
std::vector<Zombie*> Zombie::zombies;
std::vector<Walker*> Walker::walkers;
std::vector<Wall*> Wall::walls;
std::vector<Mine*> Mine::mines;
std::vector<Cannon*> Cannon::cannons;
std::vector<Worker*> Worker::workers;
std::vector<Bomber*> Bomber::bombers;
Player* Player::player;
Mother* Mother::mother;

std::bernoulli_distribution Zombie::distribution(ZOMBIE_MOVING_PROBABILITY);
std::bernoulli_distribution Walker::distribution(WALKER_MOVING_PROBABILITY);
sista::Cursor cursor;
bool end = false;

int main() {
    #ifdef __APPLE__
        term_echooff();
    #endif
    std::ios_base::sync_with_stdio(false);
    ANSI::reset(); // Reset the settings
    srand(time(NULL)); // Seed the random number generator
    #if INTRO
        printIntro();
    #endif
    sista::SwappableField field_(50, 20);
    field = &field_;
    sista::Border border(
        '#', {
            ANSI::ForegroundColor::F_WHITE,
            ANSI::BackgroundColor::B_BLACK,
            ANSI::Attribute::BRIGHT
        }
    );

    field->addPrintPawn(Player::player = new Player({10, 18}));
    field->addPrintPawn(Mother::mother = new Mother({10, 49}));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::thread th = std::thread([&]() {
        char input = 'Q';
        while (input != 'q') {
            if (end) return;
            #if defined(_WIN32) or defined(__linux__)
                input = getch();
            #elif __APPLE__
                input = getchar();
            #endif
            if (end) return;
            switch (input) {
            case 'w':
                Player::player->move(Direction::UP);
                break;
            case 'd': case 'D':
                Player::player->move(Direction::RIGHT);
                break;
            case 's': case 'S':
                Player::player->move(Direction::DOWN);
                break;
            case 'a': case 'A':
                Player::player->move(Direction::LEFT);
                break;
            case 'j': case 'J':
                Player::player->shoot(Direction::LEFT);
                break;
            case 'k': case 'K':
                Player::player->shoot(Direction::DOWN);
                break;
            case 'l': case 'L':
                Player::player->shoot(Direction::RIGHT);
                break;
            case 'i': case 'I':
                Player::player->shoot(Direction::UP);
                break;
            case 'p': case 'P': // "Projectile" or something
                Player::player->weapon = Type::BULLET;
                break;
            case 'm': case 'M':
                Player::player->weapon = Type::MINE;
                break;
            case 'c': case 'C':
                Player::player->weapon = Type::CANNON;
                break;
            case 'b': case 'B':
                Player::player->weapon = Type::BOMBER;
                break;
            case 'W':
                Player::player->weapon = Type::WORKER;
                break;
            case '=':
                Player::player->weapon = Type::WALL;
                break;
            case 'q':
                end = true;
                return;
            default:
                break;
            }
        }
    });
    for (unsigned i=0; !end; i++) {
        for (unsigned i=0; i<Bullet::bullets.size(); i++) {
            if (i >= Bullet::bullets.size()) break;
            Bullet* bullet = Bullet::bullets[i];
            bullet->move();
        }
        for (unsigned i=0; i<EnemyBullet::enemyBullets.size(); i++) {
            if (i >= EnemyBullet::enemyBullets.size()) break;
            EnemyBullet* enemyBullet = EnemyBullet::enemyBullets[i];
            enemyBullet->move();
        }
        for (auto zombie : Zombie::zombies)
            if (Zombie::distribution(rng))
                zombie->move();
        for (auto walker : Walker::walkers)
            if (Walker::distribution(rng))
                walker->move();
        for (auto mine : Mine::mines)
            mine->checkTrigger();
        for (auto cannon : Cannon::cannons)
            if (Cannon::distribution(rng))
                cannon->fire();
        for (auto worker : Worker::workers)
            if (worker->distribution(rng))
                worker->produce();
        for (auto bomber : Bomber::bombers)
            bomber->move();

        for (auto wall : Wall::walls) {
            if (wall->strength == 0) {
                Wall::removeWall(wall);
            }
        }
        for (unsigned i=0; i<Mine::mines.size(); i++) {
            if (i >= Mine::mines.size()) break;
            if (Mine::mines[i]->triggered) {
                Mine::mines[i]->explode();
                Mine::removeMine(Mine::mines[i]);
            }
        }
        if (i % 100 == 0) {
            Walker* walker = new Walker({(unsigned short)(rand() % 20), 49});
            Walker::walkers.push_back(walker);
            field->addPrintPawn(walker);
        }
        Mother::mother->setSymbol('0' + Mother::mother->life);
        field->rePrintPawn(Mother::mother);
        if (i % 10 == 0) {
            sista::clearScreen();
            field->print(border);
        }
        // Statistics
        Mother::motherStyle.apply();
        cursor.set(8, 55);
        std::cout << "Ammonitions: " << Player::player->ammonitions << "    ";
        // cursor.set(10, 55);
        // std::cout << "Life: " << Mother::mother->life;
        std::cout << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    th.join();
    cursor.set(52, 0); // Move the cursor to the bottom of the screen, so the terminal is not left in a weird state
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
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
Bullet::Bullet() : Entity(' ', {0, 0}, bulletStyle, Type::BULLET), direction(Direction::RIGHT), speed(1) {}
Bullet::Bullet(sista::Coordinates coordinates, Direction direction) : Entity(directionSymbol[direction], coordinates, bulletStyle, Type::BULLET), direction(direction), speed(1) {}
Bullet::Bullet(sista::Coordinates coordinates, Direction direction, unsigned short speed) : Entity(directionSymbol[direction], coordinates, bulletStyle, Type::BULLET), direction(direction), speed(speed) {}
void Bullet::move() {
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction]*speed;
    if (field->isOutOfBounds(nextCoordinates)) {
        Bullet::removeBullet(this);
        return;
    } else if (field->isFree(nextCoordinates)) {
        field->movePawn(this, nextCoordinates);
        coordinates = nextCoordinates;
    } else { // Something was hitten
        Entity* hitten = (Entity*)field->getPawn(nextCoordinates);
        if (hitten->type == Type::WALL) {
            Wall* wall = (Wall*)hitten;
            wall->strength--;
            if (wall->strength == 0) {
                wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
                field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
            }
        } else if (hitten->type == Type::ZOMBIE) {
            Zombie::removeZombie((Zombie*)hitten);
        } else if (hitten->type == Type::WALKER) {
            Walker::removeWalker((Walker*)hitten);
        } else if (hitten->type == Type::BULLET) {
            Bullet::removeBullet((Bullet*)hitten);
        } else if (hitten->type == Type::ENEMYBULLET) {
            EnemyBullet::removeEnemyBullet((EnemyBullet*)hitten);
        } else if (hitten->type == Type::MINE) {
            Mine* mine = (Mine*)hitten;
            mine->triggered = true;
        } else if (hitten->type == Type::CANNON) {
            Cannon* cannon = (Cannon*)hitten;
            // Makes the cannon fire
            cannon->fire();
        } else if (hitten->type == Type::MOTHER) {
            Mother* mother = (Mother*)hitten;
            mother->life--;
            field->rePrintPawn(mother);
            if (mother->life == 0) {
                // win();
                end = true;
            }
        }
        Bullet::removeBullet(this);
    }
}
void Bullet::removeBullet(Bullet* bullet) {
    Bullet::bullets.erase(std::find(Bullet::bullets.begin(), Bullet::bullets.end(), bullet));
    field->erasePawn(bullet);
    delete bullet;
}


ANSI::Settings EnemyBullet::enemyBulletStyle = {
    ANSI::ForegroundColor::F_GREEN,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
};
EnemyBullet::EnemyBullet(sista::Coordinates coordinates, Direction direction, unsigned short speed) : Entity(directionSymbol[direction], coordinates, enemyBulletStyle, Type::BULLET), direction(direction), speed(speed) {}
EnemyBullet::EnemyBullet() : Entity(' ', {0, 0}, enemyBulletStyle, Type::ENEMYBULLET), direction(Direction::UP), speed(1) {}
void EnemyBullet::removeEnemyBullet(EnemyBullet* enemyBullet) {
    EnemyBullet::enemyBullets.erase(std::find(EnemyBullet::enemyBullets.begin(), EnemyBullet::enemyBullets.end(), enemyBullet));
    field->erasePawn(enemyBullet);
    delete enemyBullet;
}
void EnemyBullet::move() {
    sista::Coordinates nextCoordinates = coordinates + directionMap[direction]*speed;
    if (field->isOutOfBounds(nextCoordinates)) {
        EnemyBullet::removeEnemyBullet(this);
        return;
    } else if (field->isFree(nextCoordinates)) {
        field->movePawn(this, nextCoordinates);
        coordinates = nextCoordinates;
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
            Bullet::removeBullet((Bullet*)hitten);
            EnemyBullet::removeEnemyBullet(this);
        } else if (hitten->type == Type::ZOMBIE || hitten->type == Type::WALKER || hitten->type == Type::ENEMYBULLET) {
            EnemyBullet::removeEnemyBullet(this);
        } else if (hitten->type == Type::MINE) {
            Mine* mine = (Mine*)hitten;
            mine->triggered = true;
            EnemyBullet::removeEnemyBullet(this);
        } else if (hitten->type == Type::CANNON) { // The cannon is destroyed by the enemy bullet
            EnemyBullet::removeEnemyBullet(this);
            Cannon::removeCannon((Cannon*)hitten);
        } else if (hitten->type == Type::MOTHER) {
            Mother* mother = (Mother*)hitten;
            mother->life--;
            field->rePrintPawn(mother);
            if (mother->life == 0) {
                // win();
                end = true;
            }
        }
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
    // The zombie will move towards the player if it is in the same row
    if (coordinates.y == Player::player->getCoordinates().y && coordinates.x > 30) {
        nextCoordinates = coordinates + directionMap[Direction::LEFT];
    } else {
        if (coordinates.y < Player::player->getCoordinates().y) {
            nextCoordinates = coordinates + directionMap[Direction::DOWN];
        } else if (coordinates.y >= Player::player->getCoordinates().y) {
            nextCoordinates = coordinates + directionMap[Direction::UP];
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
        Zombie::removeZombie(this);
    }
}

ANSI::Settings Mother::motherStyle = {
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_RED,
    ANSI::Attribute::BRIGHT
};
Mother::Mother(sista::Coordinates coordinates) : Entity('9', coordinates, motherStyle, Type::MOTHER), life(9) {}
Mother::Mother() : Entity('9', {0, 0}, motherStyle, Type::MOTHER), life(9) {}

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
            Entity* neighbor = (Entity*)field->getPawn(coordinates.y + j, coordinates.x + i);
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
            Entity* neighbor = (Entity*)field->getPawn(coordinates.y + j, coordinates.x + i);
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
            } else if (neighbor->type == Type::MOTHER) {
                Mother* mother = (Mother*)neighbor;
                mother->life--;
                field->rePrintPawn(mother);
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

ANSI::Settings Worker::workerStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BLINK
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
        Bullet::removeBullet((Bullet*)neighbor);
    } else if (neighbor->type == Type::ENEMYBULLET) {
        EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
    } else if (neighbor->type == Type::MINE) {
        Mine* mine = (Mine*)neighbor;
        mine->triggered = true;
    } else if (neighbor->type == Type::WALKER || neighbor->type == Type::ZOMBIE) {
        explode();
    } else if (neighbor->type == Type::MOTHER) {
        Mother* mother = (Mother*)neighbor;
        mother->life--;
        field->rePrintPawn(mother);
        if (mother->life == 0) {
            // win();
            end = true;
        }
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
            } else if (neighbor->type == Type::ENEMYBULLET) {
                EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
            } else if (neighbor->type == Type::MINE) {
                Mine* mine = (Mine*)neighbor;
                mine->triggered = true;
            } else if (neighbor->type == Type::CANNON) {
                Cannon::removeCannon((Cannon*)neighbor);
            } else if (neighbor->type == Type::MOTHER) {
                Mother* mother = (Mother*)neighbor;
                mother->life--;
                field->rePrintPawn(mother);
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
    }
}