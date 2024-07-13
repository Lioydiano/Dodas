#include "include/sista/sista.hpp"
#include <unordered_map>
#include <vector>
#include <random>


#define CANNON_FIRE_PROBABILITY 0.025
#define CANNON_FIRE_PERIOD 40
#define WORKER_PRODUCTION_PROBABILITY 1.0/150
#define WORKER_PRODUCTION_PERIOD 150
#define ZOMBIE_MOVING_PROBABILITY 0.2
#define WALKER_MOVING_PROBABILITY 0.1

#define START_AMMONITION 10


enum Type {
    PLAYER,
    WORKER,
    CANNON,
    BOMBER,
    BULLET,
    MINE,

    WALL,
    ZOMBIE,
    WALKER,
    ENEMYBULLET,
    MOTHER
};


enum Direction {UP, RIGHT, DOWN, LEFT};
extern std::unordered_map<Direction, sista::Coordinates> directionMap;
extern std::unordered_map<Direction, char> directionSymbol;
extern std::mt19937 rng;


class Entity : public sista::Pawn {
public:
    Type type;

    Entity();
    Entity(char, sista::Coordinates, ANSI::Settings&, Type);
};


class Bullet : public Entity {
public:
    static ANSI::Settings bulletStyle;
    static std::vector<Bullet*> bullets;
    Direction direction;
    unsigned short speed; // The bullet moves speed cells per frame

    Bullet();
    Bullet(sista::Coordinates, Direction);
    Bullet(sista::Coordinates, Direction, unsigned short);

    void move();

    static void removeBullet(Bullet*);
};


class EnemyBullet : public Entity {
public:
    static ANSI::Settings enemyBulletStyle;
    static std::vector<EnemyBullet*> enemyBullets;
    Direction direction;
    unsigned short speed; // The bullet moves speed cells per frame

    EnemyBullet();
    EnemyBullet(sista::Coordinates, Direction, unsigned short);

    void move();

    static void removeEnemyBullet(EnemyBullet*);
};


class Player : public Entity {
public:
    static ANSI::Settings playerStyle;
    static Player* player;
    Type weapon = Type::BULLET; // The player can have different weapons (bullets, mines, etc.)
    int ammonitions; // The player has a certain amount of ammonition (when it reaches 0, the player can't shoot anymore)
    unsigned short speed = 1; // The player shoots bullets at speed cells per frame

    Player();
    Player(sista::Coordinates);

    void move(Direction);
    void shoot(Direction);
};


class Zombie : public Entity {
public:
    static ANSI::Settings zombieStyle;
    static std::vector<Zombie*> zombies;
    static std::bernoulli_distribution distribution; // The zombie moves a cell every zombieSpeed frames, on average

    Zombie();
    Zombie(sista::Coordinates);

    void move(); // They should move mostly vertically
    // void move(Direction);
    void shoot(); // They should only shoot horizontally to the left
    void shoot(Direction);

    static void removeZombie(Zombie*);
};


class Mother : public Entity {
public:
    static ANSI::Settings motherStyle;
    static Mother* mother;
    int life; // The mother has a life score (when it reaches 0, the game is over) which regenerates over time

    Mother();
    Mother(sista::Coordinates);

    void move(); // Only moves vertically in a small range, I want it to always be near the center
    // void shoot(); // Only shoots horizontally to the left
    // void spawnZombie(); // Spawns a zombie at a random position (but I would like the zombies to be born close to the mother)
};


class Wall : public Entity {
public:
    static ANSI::Settings wallStyle;
    static std::vector<Wall*> walls;
    short int strength; // The wall has a certain strength (when it reaches 0, the wall is destroyed)

    Wall();
    Wall(sista::Coordinates, short int);

    static void removeWall(Wall*);
};


class Mine : public Entity {
public:
    static ANSI::Settings mineStyle;
    static std::vector<Mine*> mines;
    bool triggered = false;
    bool alive = true;

    Mine();
    Mine(sista::Coordinates);

    bool checkTrigger();
    void trigger();
    void explode();

    static void removeMine(Mine*);
};


class Cannon : public Entity { // Cannons shoot bullets only against the zombies, they have a certain firing rate
public:
    static ANSI::Settings cannonStyle;
    static std::vector<Cannon*> cannons;
    static std::bernoulli_distribution distribution; // The cannon shoots a bullet every firingRate frames, on average

    Cannon();
    Cannon(sista::Coordinates, unsigned short);

    void fire();

    static void removeCannon(Cannon*);
};


class Worker : public Entity { // Workers produce ammonition for the player, they have a certain production rate
public:
    static ANSI::Settings workerStyle;
    static std::vector<Worker*> workers;
    std::bernoulli_distribution distribution; // The worker produces an ammonition every productionRate frames, on average

    Worker();
    Worker(sista::Coordinates);
    Worker(sista::Coordinates, unsigned short);

    void produce();

    static void removeWorker(Worker*);
};


class Bomber : public Entity { // Bombers go towards the enemies and explode when they meet a wall
public:
    static ANSI::Settings bomberStyle;
    static std::vector<Bomber*> bombers;

    Bomber();
    Bomber(sista::Coordinates);

    void move();
    void explode();

    static void removeBomber(Bomber*);
};

class Walker : public Entity { // Walkers go towards the left side of the screen and can kill the player on touch, and they explode as bombers when they meet a worker
public:
    static ANSI::Settings walkerStyle;
    static std::vector<Walker*> walkers;
    static std::bernoulli_distribution distribution; // The walker moves a cell every walkerSpeed frames, on average

    Walker();
    Walker(sista::Coordinates);

    void move();
    void explode();

    static void removeWalker(Walker*);
};