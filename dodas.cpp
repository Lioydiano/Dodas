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

    field->addPawn(Player::player = new Player({10, 18}));
    field->addPawn(Mother::mother = new Mother({10, 49}));
    for (unsigned short j=0; j<20; j++) {
        Wall* wall = new Wall({j, 30}, 3); // There is a vertical macrowall in the middle of the field
        Wall::walls.push_back(wall);
        field->addPawn(wall);
        if (j % 5 == 1) {
            Zombie* zombie = new Zombie({j, 47}); // Zombies are spawned on the right side of the field (the mother side)
            Zombie::zombies.push_back(zombie);
            field->addPawn(zombie);
        }
        if (j % 5 == 3) {
            Walker* walker = new Walker({j, 45}); // Walkers are spawned on the right side of the field (the mother side)
            Walker::walkers.push_back(walker);
            field->addPawn(walker);
        }
        if (j % 5 == 2) {
            Worker* worker = new Worker({j, 1}); // Workers are spawned on the left side of the field (the player side)
            Worker::workers.push_back(worker);
            field->addPawn(worker);
        }
    }
    field->print(border);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::thread th = std::thread([&]() {
        #if defined(_WIN32) or defined(__linux__)
            char input = getch();
        #elif __APPLE__
            char input = getchar();
        #endif
        while (input != 'q') {
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
            case 'p': case 'P':
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
            default:
                break;
            }
        }
    });
    while (true) {
        for (auto bullet : Bullet::bullets)
            bullet->move();
        for (auto enemyBullet : EnemyBullet::enemyBullets)
            enemyBullet->move();
        for (auto zombie : Zombie::zombies)
            zombie->move();
        for (auto walker : Walker::walkers)
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
        field->print(border);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
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
            Zombie::zombies.erase(std::find(Zombie::zombies.begin(), Zombie::zombies.end(), hitten));
            field->removePawn(hitten);
            delete hitten;
            Bullet::removeBullet(this);
        } else if (hitten->type == Type::WALKER) {
            Walker::walkers.erase(std::find(Walker::walkers.begin(), Walker::walkers.end(), hitten));
            field->removePawn(hitten);
            delete hitten;
            Bullet::removeBullet(this);
        } else if (hitten->type == Type::BULLET || hitten->type == Type::BOMBER || hitten->type == Type::WORKER) {
            Bullet::removeBullet(this);
        } else if (hitten->type == Type::ENEMYBULLET) {
            EnemyBullet::removeEnemyBullet((EnemyBullet*)hitten);
            Bullet::removeBullet(this);
        } else if (hitten->type == Type::MINE) {
            Mine* mine = (Mine*)hitten;
            mine->triggered = true;
            Bullet::removeBullet(this);
        } else if (hitten->type == Type::CANNON) {
            Bullet::removeBullet(this);
            Cannon* cannon = (Cannon*)hitten;
            // Makes the cannon fire
            cannon->fire();
        } else if (hitten->type == Type::MOTHER) {
            Mother* mother = (Mother*)hitten;
            mother->life--;
            if (mother->life == 0) {
                field->removePawn(mother);
                delete mother;
                // win();
            }
        }
    }
}
void Bullet::removeBullet(Bullet* bullet) {
    Bullet::bullets.erase(std::find(Bullet::bullets.begin(), Bullet::bullets.end(), bullet));
    field->removePawn(bullet);
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
    field->removePawn(enemyBullet);
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
            Cannon::cannons.erase(std::find(Cannon::cannons.begin(), Cannon::cannons.end(), hitten));
            field->removePawn(hitten);
            delete hitten;
        } else if (hitten->type == Type::MOTHER) {
            Mother* mother = (Mother*)hitten;
            mother->life--;
            if (mother->life == 0) {
                field->removePawn(mother);
                delete mother;
                // win();
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
    if (field->isOutOfBounds(nextCoordinates) || !field->isFree(nextCoordinates)) {
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
        field->addPawn(newbullet);
        break;
    }
    case Type::MINE: {
        if (Player::player->ammonitions < 3)
            return;
        Player::player->ammonitions -= 3;
        Mine* newmine = new Mine(spawn);
        Mine::mines.push_back(newmine);
        field->addPawn(newmine);
        break;
    }
    case Type::CANNON: {
        if (Player::player->ammonitions < 5)
            return;
        Player::player->ammonitions -= 5;
        Cannon* newcannon = new Cannon(spawn, CANNON_FIRE_PERIOD);
        Cannon::cannons.push_back(newcannon);
        field->addPawn(newcannon);
        break;
    }
    case Type::BOMBER: {
        if (Player::player->ammonitions < 7)
            return;
        Player::player->ammonitions -= 7;
        Bomber* newbomber = new Bomber(spawn);
        Bomber::bombers.push_back(newbomber);
        field->addPawn(newbomber);
        break;
    }
    case Type::WORKER: {
        if (Player::player->ammonitions < 5)
            return;
        Player::player->ammonitions -= 5;
        Worker* newworker = new Worker(spawn, WORKER_PRODUCTION_PERIOD);
        Worker::workers.push_back(newworker);
        field->addPawn(newworker);
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
    } else if (neighbor->type == Type::BULLET) {
        Bullet::removeBullet((Bullet*)neighbor);
        field->removePawn(neighbor);
        delete neighbor;
        Zombie::zombies.erase(std::find(Zombie::zombies.begin(), Zombie::zombies.end(), this));
        field->removePawn(this);
        delete this;
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
Wall::Wall(sista::Coordinates coordinates, short int strength) : Entity('=', coordinates, wallStyle, Type::WALL), strength(strength) {}
Wall::Wall() : Entity('=', {0, 0}, wallStyle, Type::WALL), strength(3) {}

ANSI::Settings Mine::mineStyle = {
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BLINK   
};
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
    field->addPawn(newbullet);
}

ANSI::Settings Worker::workerStyle = {
    ANSI::ForegroundColor::F_YELLOW,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BLINK
};
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
Bomber::Bomber(sista::Coordinates coordinates) : Entity('B', coordinates, bomberStyle, Type::BOMBER) {}
Bomber::Bomber() : Entity('B', {0, 0}, bomberStyle, Type::BOMBER) {}
void Bomber::move() {
    sista::Coordinates nextCoordinates = coordinates + directionMap[Direction::RIGHT];
    Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
    if (neighbor == nullptr) {
        field->movePawn((Pawn*)this, nextCoordinates);
        coordinates = nextCoordinates;
    } else if (neighbor->type == Type::WALL) {
        Wall* wall = (Wall*)neighbor;
        wall->strength = 0;
        wall->setSymbol('@'); // Change the symbol to '@' to indicate that the wall was destroyed
        field->rePrintPawn(wall); // It will be reprinted in the next frame and then removed because of (strength == 0)
        explode();
    } else if (neighbor->type == Type::PLAYER) {
        // lose();
    } else if (neighbor->type == Type::BULLET) {
        Bullet::removeBullet((Bullet*)neighbor);
        Bomber::bombers.erase(std::find(Bomber::bombers.begin(), Bomber::bombers.end(), this));
        field->removePawn(this);
        delete this;
    } else if (neighbor->type == Type::ENEMYBULLET) {
        EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
        Bomber::bombers.erase(std::find(Bomber::bombers.begin(), Bomber::bombers.end(), this));
        field->removePawn(this);
        delete this;
    }
}
void Bomber::explode() {
    for (int j=-2; j<=2; j++) {
        for (int i=-2; i<=2; i++) {
            if (i == 0 && j == 0) continue;
            Entity* neighbor = (Entity*)field->getPawn(coordinates.y + j, coordinates.x + i);
            if (neighbor == nullptr) {
                continue;
            } else if (neighbor->type == Type::ZOMBIE) {
                Zombie::zombies.erase(std::find(Zombie::zombies.begin(), Zombie::zombies.end(), neighbor));
                field->removePawn(neighbor);
                delete neighbor;
            } else if (neighbor->type == Type::WALKER) {
                Walker::walkers.erase(std::find(Walker::walkers.begin(), Walker::walkers.end(), neighbor));
                field->removePawn(neighbor);
                delete neighbor;
            } else if (neighbor->type == Type::ENEMYBULLET) {
                EnemyBullet::removeEnemyBullet((EnemyBullet*)neighbor);
            } else if (neighbor->type == Type::MINE) {
                Mine* mine = (Mine*)neighbor;
                mine->triggered = true;
            } else if (neighbor->type == Type::CANNON) {
                Cannon::cannons.erase(std::find(Cannon::cannons.begin(), Cannon::cannons.end(), neighbor));
                field->removePawn(neighbor);
                delete neighbor;
            } else if (neighbor->type == Type::MOTHER) {
                Mother* mother = (Mother*)neighbor;
                mother->life--;
                if (mother->life == 0) {
                    field->removePawn(mother);
                    delete mother;
                    // win();
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
Walker::Walker(sista::Coordinates coordinates) : Entity('Z', coordinates, walkerStyle, Type::WALKER) {}
Walker::Walker() : Entity('Z', {0, 0}, walkerStyle, Type::WALKER) {}
void Walker::move() { // Walkers mostly move horizontally because they only rarely shoot bullets and they walk slowly towards the left side
    sista::Coordinates nextCoordinates = coordinates + directionMap[Direction::LEFT];
    Entity* neighbor = (Entity*)field->getPawn(nextCoordinates);
    if (neighbor == nullptr) {
        field->movePawn((Pawn*)this, nextCoordinates);
        coordinates = nextCoordinates;
    } else if (neighbor->type == Type::PLAYER) {
        // lose();
    } else if (neighbor->type == Type::BULLET) {
        Bullet::removeBullet((Bullet*)neighbor);
        Walker::walkers.erase(std::find(Walker::walkers.begin(), Walker::walkers.end(), this));
        field->removePawn(this);
        delete this;
    }
}