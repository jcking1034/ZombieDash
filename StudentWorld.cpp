#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include "Level.h"

#include <string>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), citizens_remaining(0)   //  can i modify this?
{
}

int StudentWorld::init()
{
    level_done = false;
    citizens_remaining = 0;
    
    if (getLevel() - 1 == 99) return GWSTATUS_PLAYER_WON;   //  if level is 99 player won
    
    Level lev(assetPath());
    

    
    ostringstream level_name;
    level_name << "level";
    level_name.fill('0');
    level_name << setw(2);
    level_name << getLevel();
    level_name << ".txt";
    
    string levelFile = level_name.str();
    
    Level::LoadResult result = lev.loadLevel(levelFile);    //  check this
    if (result == Level::load_fail_file_not_found)
        return GWSTATUS_PLAYER_WON;
    else if (result == Level::load_fail_bad_format)
        return GWSTATUS_LEVEL_ERROR;
    else if (result == Level::load_success) {
        //cerr << "Successfully loaded level" << endl;
        
        for (int i = 0; i < LEVEL_WIDTH; i++) {
            for (int j = 0; j < LEVEL_HEIGHT; j++) {
                Level::MazeEntry ge = lev.getContentsOf(i,j);
                switch (ge) {
                        /* enum MazeEntry {
                         empty, player, dumb_zombie, smart_zombie, citizen, wall, exit, pit,
                         vaccine_goodie, gas_can_goodie, landmine_goodie
                         };
                         */
                    case Level::empty:
                        break;
                    case Level::player:
                        player = new Penelope(i, j, this);
                        break;
                    case Level::dumb_zombie: {
                        Actor_list.push_back(new Dumb_Zombie(i, j, this));
                        break;
                    }
                    case Level::smart_zombie: {
                        Actor_list.push_back(new Smart_Zombie(i, j, this));
                        break;
                    }
                    case Level::citizen: {
                        Actor_list.push_back(new Citizen(i, j, this));
                        citizens_remaining++;
                        break;
                    }
                    case Level::wall: {
                        Actor_list.push_back(new Wall(i, j, this));
                        break;
                    }
                    case Level::exit: {
                        Actor_list.push_back(new Exit(i, j, this));
                        break;
                    }
                    case Level::pit:
                        Actor_list.push_back(new Pit(i, j, this));
                        break;
                    case Level::vaccine_goodie: {
                        Actor_list.push_back(new Vaccine_Goodie(i, j, this));
                        break;
                    }
                    case Level::gas_can_goodie: {
                        Actor_list.push_back(new Gas_Can_Goodie(i, j, this));
                        break;
                    }
                    case Level::landmine_goodie: {
                        Actor_list.push_back(new Landmine_Goodie(i, j, this));
                        break;
                    }
                }
            }
        }
    }
    
    return GWSTATUS_CONTINUE_GAME;
}
                         
int StudentWorld::move()
{
    //  give player chance to do something
    player->doSomething();
    if (!player->is_alive()) {
//        playSound(SOUND_PLAYER_DIE);    //  check this
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    
    //  give each other actor chance to do something
    list <Actor*>::iterator i = Actor_list.begin();
    while (i != Actor_list.end()) {
        if ((*i)->is_alive()) (*i)->doSomething();
        
        if (!player->is_alive()) {
            decLives();
//            playSound(SOUND_PLAYER_DIE);
            return GWSTATUS_PLAYER_DIED;
        }
        
        if (level_done) {
            level_done = false;
            return GWSTATUS_FINISHED_LEVEL;
        }
        
        i++;
    }
    
    //  Remove newly-dead actors
    list <Actor*>::iterator j = Actor_list.begin();
    while (j != Actor_list.end()) {
        if (!(*j)->is_alive()) {
            delete (*j);
            j = Actor_list.erase(j);
        } else {
            j++;
        }
    }
    
    //  Update the Game Status Line
    ostringstream oss;
    oss << "Score: ";
    if (getScore() < 0) {
        oss << "-";
    } else {
        oss << getScore() / 100000; //  check this
    }
    oss.fill('0');
    oss << setw(5);
    if (getScore() < 0) {
        oss << getScore() * -1;
    } else {
        oss << getScore();
    }
    oss << "  ";
    oss << "Level: " << getLevel() << "  ";
    oss << "Lives: " << getLives() << "  ";
    oss << "Vaccines: " << player->get_vaccines() << "  ";
    oss << "Flames: " << player->get_flamethrower() << "  ";
    oss << "Mines: " << player->get_landmines() << "  ";
    oss << "Infected: " << player->get_infection_count();
    setGameStatText(oss.str());

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    if (player != nullptr) {
        delete player;
        player = nullptr;
    }
    list <Actor*>::iterator i = Actor_list.begin();
    
    while (i != Actor_list.end()) {
        if ((*i) != nullptr && Actor_list.size() != 0) {
            delete (*i);
            *i = nullptr;
            i = Actor_list.erase(i);
        } else {
            i++;
        }
    }
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

void StudentWorld::level_complete() {
    level_done = true;
    playSound(SOUND_LEVEL_FINISHED);
}

bool StudentWorld::collision(int x, int y, Actor * me) {
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i)
        if ((*i)->collision(x, y) && (*i)->can_block_movement() && me != (*i)) return true;
    return false;
    
}

bool StudentWorld::collision_with_player(int x, int y) {
    return (*player).collision(x, y);
//    int player_x = (*player).getX();    //  is this right?
//    int player_y = (*player).getY();
//
//    if (x + SPRITE_WIDTH - 1 >= player_x && x <= player_x + SPRITE_WIDTH - 1) return true;
//    if (y + SPRITE_HEIGHT - 1 >= player_y && y <= player_y + SPRITE_HEIGHT - 1) return true;
//    return false;
}

bool StudentWorld::overlap_wall_exit(int x, int y) {    //  wall and exit block flames
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i)
        if ((*i)->overlap(x, y) && (*i)->block_flames() && !(*i)->damaged_by_flame()) return true;
    return false;
}

bool StudentWorld::overlap_player(int x, int y) {
    if (player->overlap(x, y)) return true;
    return false;
}

bool StudentWorld::overlap_citizen_exit(int x, int y) { //  citizens can be infected and burn
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_vomit() && (*i)->damaged_by_flame() && (*i)->overlap(x, y)) {
            (*i)->set_alive(false);
            citizens_remaining--;
            return true;  //  only does 1 per tick is that OK? check
        }
    }
    return false;
}

bool StudentWorld::overlap_person_pit(int x, int y) {
    if (overlap_player(x, y)) {
        player->kill();
    }
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if (((*i)->damaged_by_vomit() || (*i)->damaged_by_flame()) && (*i)->can_block_movement() && (*i)->overlap(x, y)) {
            (*i)->kill();
        }
    }
    return false;
}

void StudentWorld::overlap_any_flame(int x, int y) {
    if (overlap_player(x, y)) {
        player->kill();
    }
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->damaged_by_flame() && (*i)->overlap(x, y)) {  //  check this for goodies
            (*i)->set_alive(false);
            if ((*i)->can_block_movement() || (!(*i)->block_flames() && !(*i)->damaged_by_vomit() && !(*i)->can_block_movement())) {
                (*i)->kill();
            }
        }
    }
}

void StudentWorld::overlap_person_vomit(int x, int y) {
    if (overlap_player(x, y)) {
        player->infect();
    }
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->damaged_by_vomit() && (*i)->overlap(x, y)) {
            (*i)->infect();
        }
    }
}

bool StudentWorld::overlap_person_landmine(int x, int y) {  //  tells landmine if it should boom
    if (overlap_player(x, y)) return true;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_flame() && (*i)->overlap(x, y)) {
            return true;
        }
    }
    return false;
}

bool StudentWorld::close_to_person(int x, int y) {
    if (distance(player_x(), player_y(), x, y) < 10) return true;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_vomit()) {
            if (distance((*i)->getX(), (*i)->getY(), x, y) < 10) return true;
        }
    }
    
    return false;
}

bool StudentWorld::overlap_person_or_wall(int x, int y, Actor * me) {
    if (overlap_player(x, y)) return true;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && ((*i)->damaged_by_flame() || (*i)->block_flames())) {
            if (overlap((*i)->getX(), (*i)->getY(), x, y) && me != (*i)) {
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::overlap_any(int x, int y) {
    if (overlap_player(x, y)) return true;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if (overlap((*i)->getX(), (*i)->getY(), x, y)) {
            return true;
        }
    }
    return false;
}

double StudentWorld::nearest_zombie(int x, int y) {    //  check this
    double shortest_distance = -1;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_flame() && !(*i)->damaged_by_vomit() && (*i)->is_alive()) {
            double temp = distance(x, y, (*i)->getX(), (*i)->getY());
            shortest_distance = (temp < shortest_distance || shortest_distance == -1) ? temp : shortest_distance;
        }
    }
    
    return shortest_distance;
}

double StudentWorld::nearest_citizen(int x, int y) {    //  check this
    double shortest_distance = -1;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_flame() && (*i)->damaged_by_vomit() && (*i)->is_alive()) {
            double temp = distance(x, y, (*i)->getX(), (*i)->getY());
            shortest_distance = (temp < shortest_distance || shortest_distance == -1) ? temp : shortest_distance;
        }
    }
    
    return shortest_distance;
}

int StudentWorld::nearest_citizen_x(int x, int y) {    //  check this
    int nearest_citizen_x = -1;
    int shortest_distance = -1;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_flame() && (*i)->damaged_by_vomit() && (*i)->is_alive()) {
            int temp = distance(x, y, (*i)->getX(), (*i)->getY());
            if (temp < shortest_distance || shortest_distance == -1) {
                nearest_citizen_x = (*i)->getX();
                shortest_distance = temp;
            }
        }
    }
    
    return nearest_citizen_x;
}

int StudentWorld::nearest_citizen_y(int x, int y) {    //  check this
    int nearest_citizen_y = -1;
    int shortest_distance = -1;
    
    list <Actor*>::iterator i;
    for (i = Actor_list.begin(); i != Actor_list.end(); ++i) {
        if ((*i)->can_block_movement() && (*i)->damaged_by_flame() && (*i)->damaged_by_vomit() && (*i)->is_alive()) {
            int temp = distance(x, y, (*i)->getX(), (*i)->getY());
            if (temp < shortest_distance || shortest_distance == -1) {
                nearest_citizen_y = (*i)->getY();
                shortest_distance = temp;
            }
        }
    }
    
    return nearest_citizen_y;
}

void StudentWorld::new_flame(int x, int y, int direction) {
    if(!overlap_wall_exit(x, y)) {
        Actor_list.push_back(new Flame(x, y, this, direction, 1, 1));
    }
}

void StudentWorld::new_zombie(int x, int y) {
    int chance = randInt(1, 100);
    if (chance <= 70) {
        Actor_list.push_back(new Dumb_Zombie(x, y, this, 1, 1));  //  NEED TO IMPLEMENT THIS
    } else {
        Actor_list.push_back(new Smart_Zombie(x, y, this, 1, 1));
    }
}

void StudentWorld::new_vomit(int x, int y, int direction) {
    Actor_list.push_back(new Vomit(x, y, this, direction, 1, 1));
}

void StudentWorld::new_pit(int x, int y) {
    Actor_list.push_back(new Pit(x, y, this, 1, 1));
}

void StudentWorld::new_landmine(int x, int y) {
    Actor_list.push_back(new Landmine(x, y, this, 1, 1));
}

void StudentWorld::new_vaccine(int x, int y) {
    Actor_list.push_back(new Vaccine_Goodie(x, y, this, 1, 1));
}

void StudentWorld::p_add_vaccines() {
    player->add_vaccines(1);
}   //  inform StudnetWorld that Penelope gets vaccine (35)

void StudentWorld::p_add_flamethrower() {
    player->add_flamethrower(5);
}  //  Inform the StudentWorld object that Penelope is to receive 5 charges for her flamethrower.

void StudentWorld::p_add_landmines() {
    player->add_landmines(2);
}
